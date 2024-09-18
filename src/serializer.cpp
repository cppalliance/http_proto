//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/message_view_base.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

#include "detail/filter.hpp"

#include <boost/buffers/algorithm.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/core/ignore_unused.hpp>

#include <stddef.h>

namespace boost {
namespace http_proto {

namespace {
class deflator_filter
    : public http_proto::detail::filter
{
    zlib::stream& deflator_;

public:
    deflator_filter(
        context& ctx,
        http_proto::detail::workspace& ws,
        bool use_gzip)
        : deflator_{ ctx.get_service<zlib::service>()
            .make_deflator(ws, -1, use_gzip ? 31 : 15, 8) }
    {
    }

    virtual filter::results
    on_process(
        buffers::mutable_buffer out,
        buffers::const_buffer in,
        bool more) override
    {
        auto flush =
            more ? zlib::flush::none : zlib::flush::finish;
        filter::results results;

        for(;;)
        {
            auto params = zlib::params{in.data(), in.size(),
                out.data(), out.size() };
            results.ec = deflator_.write(params, flush);

            results.in_bytes  += in.size() - params.avail_in;
            results.out_bytes += out.size() - params.avail_out;

            if(results.ec.failed())
                return results;

            if(results.ec == zlib::error::stream_end)
            {
                results.finished = true;
                return results;
            }

            in  = buffers::suffix(in, params.avail_in);
            out = buffers::suffix(out, params.avail_out);

            if(in.size() == 0)
            {
                // TODO: is this necessary?
                if(results.out_bytes == 0)
                {
                    flush = zlib::flush::sync;
                    continue;
                }
                return results;
            }
        }
    }
};
} // namespace

void
consume_buffers(
    buffers::const_buffer*& p,
    std::size_t& n,
    std::size_t bytes)
{
    while(n > 0)
    {
        if(bytes < p->size())
        {
            *p += bytes;
            return;
        }
        bytes -= p->size();
        ++p;
        --n;
    }

    // Precondition violation
    if(bytes > 0)
        detail::throw_invalid_argument();
}

template<class MutableBuffers>
void
write_chunk_header(
    MutableBuffers const& dest0,
    std::size_t size) noexcept
{
    static constexpr char hexdig[] =
        "0123456789ABCDEF";
    char buf[18];
    auto p = buf + 16;
    for(std::size_t i = 16; i--;)
    {
        *--p = hexdig[size & 0xf];
        size >>= 4;
    }
    buf[16] = '\r';
    buf[17] = '\n';
    auto n = buffers::buffer_copy(
        dest0,
        buffers::const_buffer(
            buf, sizeof(buf)));
    ignore_unused(n);
    BOOST_ASSERT(n == 18);
    BOOST_ASSERT(
        buffers::buffer_size(dest0) == n);
}

template<class DynamicBuffer>
void
write_chunk_close(DynamicBuffer& db)
{
    db.commit(
        buffers::buffer_copy(
            db.prepare(2),
            buffers::const_buffer("\r\n", 2)));
}

template<class DynamicBuffer>
void
write_last_chunk(DynamicBuffer& db)
{
    db.commit(
        buffers::buffer_copy(
            db.prepare(5),
            buffers::const_buffer("0\r\n\r\n", 5)));
}

//------------------------------------------------

serializer::
~serializer()
{
}

serializer::
serializer(
    serializer&&) noexcept = default;

serializer::
serializer(
    context& ctx)
    : serializer(ctx, 65536)
{
}

serializer::
serializer(
    context& ctx,
    std::size_t buffer_size)
    : ws_(buffer_size)
    , ctx_(ctx)
{
}

void
serializer::
reset() noexcept
{
    chunk_header_ = {};
    chunk_close_ = {};
    last_chunk_ = {};
    filter_ = nullptr;
    more_ = false;
    is_done_ = false;
    is_chunked_ = false;
    is_expect_continue_ = false;
    is_compressed_ = false;
    filter_done_ = false;
    in_ = nullptr;
    out_ = nullptr;
    ws_.clear();
}

//------------------------------------------------

auto
serializer::
prepare() ->
    system::result<
        const_buffers_type>
{
    // Precondition violation
    if( is_done_ )
        detail::throw_logic_error();

    // Expect: 100-continue
    if( is_expect_continue_ )
    {
        if( !is_header_done_ )
            return const_buffers_type(hp_, 1);
        is_expect_continue_ = false;
        BOOST_HTTP_PROTO_RETURN_EC(
            error::expect_100_continue);
    }

    if( st_ == style::empty )
        return const_buffers_type(
            prepped_.data(), prepped_.size());

    if( st_ == style::buffers && !filter_ )
        return const_buffers_type(
            prepped_.data(), prepped_.size());

    // callers must consume() everything before invoking
    // prepare() again
    if( !is_header_done_ &&
        buffers::buffer_size(prepped_) != prepped_[0].size() )
        detail::throw_logic_error();

    if( is_header_done_ &&
        buffers::buffer_size(prepped_) > 0 )
        detail::throw_logic_error();

    auto& input = *in_;
    auto& output = *out_;
    if( st_ == style::source && more_ )
    {
        auto results = src_->read(
            input.prepare(input.capacity()));
        more_ = !results.finished;
        input.commit(results.bytes);
    }

    if( st_ == style::stream &&
        more_ &&
        in_->size() == 0 )
        BOOST_HTTP_PROTO_RETURN_EC(error::need_data);

    bool has_avail_out =
        ((!filter_ && (more_ || input.size() > 0)) ||
        (filter_ && !filter_done_));

    auto get_input = [&]() -> buffers::const_buffer
    {
        if( st_ == style::buffers )
        {
            if( buffers::buffer_size(buf_) == 0 )
                return {};

            auto buf = *(buf_.data());
            BOOST_ASSERT(buf.size() > 0);
            return buf;
        }
        else
        {
            if( input.size() == 0 )
                return {};

            auto cbs = input.data();
            auto buf = *cbs.begin();
            if( buf.size() == 0 )
            {
                auto p = cbs.begin();
                ++p;
                buf = *p;
            }
            if( buf.size() == 0 )
                detail::throw_logic_error();
            return buf;
        }
    };

    auto get_output = [&]() -> buffers::mutable_buffer
    {
        auto mbs = output.prepare(output.capacity());
        auto buf = *mbs.begin();
        if( buf.size() == 0 )
        {
            auto p = mbs.begin();
            ++p;
            buf = *p;
        }
        return buf;
    };

    auto consume = [&](std::size_t n)
    {
        if( st_ == style::buffers )
        {
            buf_.consume(n);
            if( buffers::buffer_size(buf_) == 0 )
                more_ = false;
        }
        else
            input.consume(n);
    };

    std::size_t num_written = 0;
    if( !filter_ )
        num_written += input.size();
    else
    {
        for(;;)
        {
            auto in = get_input();
            auto out = get_output();
            if( out.size() == 0 )
            {
                if( output.size() == 0 )
                    detail::throw_logic_error();
                break;
            }

            auto rs = filter_->process(
                out, in, more_);

            if( rs.finished )
                filter_done_ = true;

            consume(rs.in_bytes);

            if( rs.out_bytes == 0 )
                break;

            num_written += rs.out_bytes;
            output.commit(rs.out_bytes);
        }
    }

    // end:
    std::size_t n = 0;
    if( !is_header_done_ )
    {
        BOOST_ASSERT(hp_ == &prepped_[0]);
        ++n;
    }
    else
        prepped_.reset(prepped_.capacity());

    if( !is_chunked_ )
    {
        for(buffers::const_buffer const& b : output.data())
            prepped_[n++] = b;
    }
    else
    {
        if( has_avail_out )
        {
            write_chunk_header(
                chunk_header_, num_written);
            prepped_[n++] = chunk_header_;

            for(buffers::const_buffer const& b : output.data())
                prepped_[n++] = b;

            prepped_[n++] = chunk_close_;
        }

        if( (filter_ && filter_done_) ||
            (!filter_ && !more_) )
            prepped_[n++] = last_chunk_;
    }

    auto cbs = const_buffers_type(
        prepped_.data(), prepped_.size());

    BOOST_ASSERT(buffers::buffer_size(cbs) > 0);
    return cbs;
}

void
serializer::
consume(
    std::size_t n)
{
    // Precondition violation
    if( is_done_ )
        detail::throw_logic_error();

    if( is_expect_continue_ )
    {
        // Cannot consume more than
        // the header on 100-continue
        if( n > hp_->size() )
            detail::throw_invalid_argument();
    }

    if( !is_header_done_ )
    {
        // consume header
        if( n < hp_->size() )
        {
            prepped_.consume(n);
            return;
        }
        n -= hp_->size();
        prepped_.consume(hp_->size());
        is_header_done_ = true;
    }

    prepped_.consume(n);
    auto is_empty = (buffers::buffer_size(prepped_) == 0);

    if( st_ == style::buffers && !filter_ && is_empty )
        more_ = false;

    if( st_ == style::empty &&
        is_empty &&
        !is_expect_continue_ )
        more_ = false;

    if( is_empty )
    {
        if( out_ && out_->size() )
        {
            BOOST_ASSERT(st_ != style::empty);
            out_->consume(out_->size());
        }
        is_done_ = filter_ ? filter_done_ : !more_;
    }
}

void
serializer::
use_deflate_encoding()
{
    // can only apply one encoding
    if(filter_)
        detail::throw_logic_error();

    is_compressed_ = true;
    filter_ = &ws_.emplace<deflator_filter>(ctx_, ws_, false);
}

void
serializer::
use_gzip_encoding()
{
    // can only apply one encoding
    if( filter_ )
        detail::throw_logic_error();

    is_compressed_ = true;
    filter_ = &ws_.emplace<deflator_filter>(ctx_, ws_, true);
}

//------------------------------------------------

void
serializer::
copy(
    buffers::const_buffer* dest,
    buffers::const_buffer const* src,
    std::size_t n) noexcept
{
    while(n--)
        *dest++ = *src++;
}

void
serializer::
start_init(
    message_view_base const& m)
{
    // VFALCO what do we do with
    // metadata error code failures?
    // m.ph_->md.maybe_throw();

    auto const& md = m.metadata();

    is_done_ = false;
    is_header_done_ = false;
    is_expect_continue_ = md.expect.is_100_continue;

    // Transfer-Encoding
    {
        auto const& te = md.transfer_encoding;
        is_chunked_ = te.is_chunked;
    }

    if( is_chunked_)
    {
        auto* p = ws_.reserve_front(chunked_overhead_);
        chunk_header_ =
            buffers::mutable_buffer(p, chunk_header_len_);
        chunk_close_ =
            buffers::mutable_buffer(
                p + chunk_header_len_, crlf_len_);
        last_chunk_ =
            buffers::mutable_buffer(
                p + chunk_header_len_ + crlf_len_,
                last_chunk_len_);

        buffers::buffer_copy(
            chunk_close_, buffers::const_buffer("\r\n", 2));
        buffers::buffer_copy(
            last_chunk_,
            buffers::const_buffer("0\r\n\r\n", 5));
    }
}

void
serializer::
start_empty(
    message_view_base const& m)
{
    start_init(m);

    st_ = style::empty;
    more_ = true;

    if(! is_chunked_)
    {
        prepped_ = make_array(
            1); // header
    }
    else
    {
        prepped_ = make_array(
            1 + // header
            1); // final chunk

        // Buffer is too small
        if(ws_.size() < 5)
            detail::throw_length_error();

        buffers::mutable_buffer dest(
            ws_.data(), 5);
        buffers::buffer_copy(
            dest,
            buffers::const_buffer(
                "0\r\n\r\n", 5));
        prepped_[1] = dest;
    }

    hp_ = &prepped_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
}

void
serializer::
start_buffers(
    message_view_base const& m)
{
    st_ = style::buffers;
    tmp1_ = {};

    if( !filter_ && !is_chunked_ )
    {
        prepped_ = make_array(
            1 +           // header
            buf_.size()); // user input

        hp_ = &prepped_[0];
        *hp_ = { m.ph_->cbuf, m.ph_->size };

        copy(&prepped_[1], buf_.data(), buf_.size());

        more_ = (buffers::buffer_size(buf_) > 0);
        return;
    }

    if( !filter_ && is_chunked_ )
    {
        if( buffers::buffer_size(buf_) == 0 )
        {
            prepped_ = make_array(
                1 +           // header
                1);           // last chunk

            hp_ = &prepped_[0];
            *hp_ = { m.ph_->cbuf, m.ph_->size };
            prepped_[1] = last_chunk_;
            more_ = false;
            return;
        }

        write_chunk_header(
            chunk_header_, buffers::buffer_size(buf_));

        prepped_ = make_array(
            1 +           // header
            1 +           // chunk header
            buf_.size() + // user input
            1 +           // chunk close
            1);           // last chunk

        hp_ = &prepped_[0];
        *hp_ = { m.ph_->cbuf, m.ph_->size };
        prepped_[1] = chunk_header_;
        copy(&prepped_[2], buf_.data(), buf_.size());

        prepped_[prepped_.size() - 2] = chunk_close_;
        prepped_[prepped_.size() - 1] = last_chunk_;
        more_ = true;
        return;
    }

    if( is_chunked_ )
    {
        prepped_ = make_array(
            1 + // header
            1 + // chunk header
            2 + // tmp
            1 + // chunk close
            1); // last chunk
    }
    else
        prepped_ = make_array(
            1 + // header
            2); // tmp

    hp_ = &prepped_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
    tmp0_ = { ws_.data(), ws_.size() };
    out_ = &tmp0_;
    in_ = out_;
    more_ = true;
}

void
serializer::
start_source(
    message_view_base const& m,
    source* src)
{
    st_ = style::source;
    src_ = src;

    if( is_chunked_ )
    {
        prepped_ = make_array(
            1 + // header
            1 + // chunk header
            2 + // tmp
            1 + // chunk close
            1); // last chunk
    }
    else
        prepped_ = make_array(
            1 + // header
            2); // tmp

    if( !filter_ )
    {
        tmp0_ = { ws_.data(), ws_.size() };
        if( tmp0_.capacity() < 1 )
            detail::throw_length_error();

        in_ = &tmp0_;
        out_ = &tmp0_;
    }
    else
    {
        auto n = ws_.size() / 2;
        auto* p = ws_.reserve_front(n);
        tmp1_ = buffers::circular_buffer(p, n);

        tmp0_ = { ws_.data(), ws_.size() };
        if( tmp0_.capacity() < 1 )
            detail::throw_length_error();

        in_ = &tmp1_;
        out_ = &tmp0_;
    }

    hp_ = &prepped_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
    more_ = true;
}

auto
serializer::
start_stream(
    message_view_base const& m) ->
        stream
{
    start_init(m);

    st_ = style::stream;
    if( is_chunked_ )
    {
        prepped_ = make_array(
            1 + // header
            1 + // chunk header
            2 + // tmp
            1 + // chunk close
            1); // last chunk
    }
    else
        prepped_ = make_array(
            1 + // header
            2); // tmp

    if( !filter_ )
    {
        tmp0_ = { ws_.data(), ws_.size() };
        if( tmp0_.capacity() < 1 )
            detail::throw_length_error();

        in_ = &tmp0_;
        out_ = &tmp0_;
    }
    else
    {
        auto n = ws_.size() / 2;
        auto* p = ws_.reserve_front(n);
        tmp1_ = buffers::circular_buffer(p, n);

        tmp0_ = { ws_.data(), ws_.size() };
        if( tmp0_.capacity() < 1 )
            detail::throw_length_error();

        in_ = &tmp1_;
        out_ = &tmp0_;
    }

    hp_ = &prepped_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
    more_ = true;
    return stream{*this};
}

//------------------------------------------------

std::size_t
serializer::
stream::
capacity() const noexcept
{
    return sr_->in_->capacity();
}

std::size_t
serializer::
stream::
size() const noexcept
{
    return sr_->in_->size();
}

bool
serializer::
stream::
is_full() const noexcept
{
    return capacity() == 0;
}

auto
serializer::
stream::
prepare() const ->
    buffers_type
{
    return sr_->in_->prepare(sr_->in_->capacity());
}

void
serializer::
stream::
commit(std::size_t n) const
{
    // the stream must make a non-zero amount of bytes
    // available to the serializer
    if( n == 0 )
        detail::throw_logic_error();

    sr_->in_->commit(n);
}

void
serializer::
stream::
close() const
{
    // Precondition violation
    if(! sr_->more_ )
        detail::throw_logic_error();
    sr_->more_ = false;
}

//------------------------------------------------

} // http_proto
} // boost

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

#include "src/detail/zlib_filter.hpp"

#include <boost/buffers/copy.hpp>
#include <boost/buffers/front.hpp>
#include <boost/buffers/prefix.hpp>
#include <boost/buffers/sans_prefix.hpp>
#include <boost/buffers/sans_suffix.hpp>
#include <boost/buffers/size.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/rts/zlib/deflate_service.hpp>
#include <boost/rts/zlib/compression_method.hpp>
#include <boost/rts/zlib/compression_strategy.hpp>

#include <stddef.h>

namespace boost {
namespace http_proto {

namespace {

constexpr
std::size_t
crlf_len = 2;

constexpr
std::size_t
chunk_header_len = 16 + crlf_len;

constexpr
std::size_t
final_chunk_len = 1 + crlf_len + crlf_len;

constexpr
std::size_t
chunked_overhead_ =
    chunk_header_len +
    crlf_len +
    final_chunk_len;

template<class MutableBufferSequence>
void
write_chunk_header(
    const MutableBufferSequence& mbs,
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
    auto n = buffers::copy(
        mbs,
        buffers::const_buffer(
            buf, sizeof(buf)));
    ignore_unused(n);
    BOOST_ASSERT(n == 18);
}

template<class MutableBufferSequence>
void
write_crlf(
    const MutableBufferSequence& mbs) noexcept
{
    auto n = buffers::copy(
        mbs,
        buffers::const_buffer(
            "\r\n", 2));
    ignore_unused(n);
    BOOST_ASSERT(n == 2);
}

template<class MutableBufferSequence>
void
write_final_chunk(
    const MutableBufferSequence& mbs) noexcept
{
    auto n = buffers::copy(
        mbs,
        buffers::const_buffer(
            "0\r\n\r\n", 5));
    ignore_unused(n);
    BOOST_ASSERT(n == 5);
}

//------------------------------------------------

class appender
{
    buffers::circular_buffer& cb_;
    buffers::mutable_buffer_pair mbp_;
    std::size_t n_ = 0;
    bool is_chunked_ = false;
    bool more_input_ = true;

public:
    appender(
        buffers::circular_buffer& cb,
        bool is_chunked)
        : cb_(cb)
        , mbp_(cb.prepare(cb.capacity()))
        , is_chunked_(is_chunked)
    {
    }

    bool
    is_full() const noexcept
    {
        auto remaining = cb_.capacity() - n_;
        if(is_chunked_)
            return remaining <= chunked_overhead_;

        return remaining == 0;
    }

    buffers::mutable_buffer_pair
    prepare() noexcept
    {
        if(is_chunked_)
        {
            return buffers::sans_suffix(
                buffers::sans_prefix(
                    mbp_,
                    chunk_header_len + n_)
                , final_chunk_len + crlf_len);
        }
        return buffers::sans_prefix(mbp_, n_);
    }

    void
    commit(std::size_t n, bool more) noexcept
    {
        BOOST_ASSERT(more_input_);
        n_ += n;
        more_input_ = more;
    }

    ~appender()
    {
        if(is_chunked_)
        {
            if(n_)
            {
                write_chunk_header(mbp_, n_);
                cb_.commit(n_ + chunk_header_len);

                write_crlf(
                    cb_.prepare(crlf_len));
                cb_.commit(crlf_len);
            }

            if(!more_input_)
            {
                write_final_chunk(
                    cb_.prepare(final_chunk_len));
                cb_.commit(final_chunk_len);
            }
        }
        else // is_chunked_ == false
        {
            cb_.commit(n_);
        }
    }
};

} // namespace

//------------------------------------------------

class serializer::filter
    : public detail::zlib_filter
{
    rts::zlib::deflate_service& svc_;

public:
    filter(
        rts::context& ctx,
        http_proto::detail::workspace& ws,
        int comp_level,
        int window_bits,
        int mem_level)
        : zlib_filter(ws)
        , svc_(ctx.get_service<rts::zlib::deflate_service>())
    {
        system::error_code ec = static_cast<rts::zlib::error>(svc_.init2(
            strm_,
            comp_level,
            rts::zlib::deflated,
            window_bits,
            mem_level,
            rts::zlib::default_strategy));
        if(ec != rts::zlib::error::ok)
            detail::throw_system_error(ec);
    }

private:
    virtual
    std::size_t
    min_out_buffer() const noexcept override
    {
        // Prevents deflate from producing
        // zero output due to small buffer
        return 8;
    }

    virtual
    rts::zlib::error
    do_process(rts::zlib::flush flush) noexcept override
    {
        return static_cast<
            rts::zlib::error>(svc_.deflate(strm_, flush));
    }
};

class serializer_service
    : public rts::service
{
public:
    serializer::config cfg;
    std::size_t space_needed = 0;

    serializer_service(
        rts::context&,
        serializer::config const& cfg_)
        : cfg(cfg_)
    {
        space_needed += cfg.payload_buffer;
        space_needed += cfg.max_type_erase;

        if(cfg.apply_deflate_encoder || cfg.apply_gzip_encoder)
        {
            // TODO: Account for the number of allocations and
            // their overhead in the workspace.

            // https://www.zlib.net/zlib_tech.html
            space_needed +=
                (1 << (cfg.zlib_window_bits + 2)) +
                (1 << (cfg.zlib_mem_level + 9)) +
                (6 * 1024) +
                #ifdef __s390x__
                5768 +
                #endif
                detail::workspace::space_needed<
                    serializer::filter>();
        }
    }
};

void
install_serializer_service(
    rts::context& ctx,
    serializer::config const& cfg)
{
    ctx.make_service<
        serializer_service>(cfg);
}

serializer::
~serializer()
{
}

serializer::
serializer(
    serializer&&) noexcept = default;

serializer::
serializer(rts::context& ctx)
    : ctx_(ctx)
    , svc_(ctx.get_service<serializer_service>())
    , ws_(svc_.space_needed)
{
}

void
serializer::
reset() noexcept
{
    ws_.clear();
    filter_ = nullptr;
    is_done_ = false;
    is_header_done_ = false;
    filter_done_ = false;
}

//------------------------------------------------

auto
serializer::
prepare() ->
    system::result<const_buffers_type>
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    // Expect: 100-continue
    if(needs_exp100_continue_)
    {
        if(!is_header_done_)
            return const_buffers_type(
                prepped_.begin(),
                1); // limit to header

        needs_exp100_continue_ = false;

        BOOST_HTTP_PROTO_RETURN_EC(
            error::expect_100_continue);
    }

    if(!filter_)
    {
        switch(st_)
        {
        case style::empty:
            return const_buffers_type(
                prepped_.begin(),
                prepped_.size());

        case style::buffers:
            // add more buffers if prepped_ is half empty.
            if(more_input_ &&
                prepped_.capacity() >= prepped_.size())
            {
                prepped_.slide_to_front();
                while(prepped_.capacity() != 0)
                {
                    auto buf = buf_gen_->next();
                    if(buf.size() != 0)
                    {
                        prepped_.append(buf);
                    }
                    else // buf_gen_ is empty
                    {
                        // append crlf and final chunk
                        if(is_chunked_)
                        {
                            prepped_.append(tmp_);
                            more_input_ = false;
                        }
                        break;
                    }
                }
                if(buf_gen_->is_empty() && !is_chunked_)
                    more_input_ = false;
            }
            return const_buffers_type(
                prepped_.begin(),
                prepped_.size());

        case style::source:
        {
            if(!more_input_)
                break;

            // handles chunked payloads automatically
            appender apndr(cb0_, is_chunked_);

            if(apndr.is_full())
                break;

            const auto rs = source_->read(
                apndr.prepare());

            if(rs.ec.failed())
            {
                is_done_ = true;
                return rs.ec;
            }

            if(rs.finished)
                more_input_ = false;

            apndr.commit(rs.bytes, more_input_);
            break;
        }

        case style::stream:
            if(is_header_done_ && cb0_.size() == 0)
                BOOST_HTTP_PROTO_RETURN_EC(
                    error::need_data);
            break;
        }
    }
    else // filter
    {
        switch(st_)
        {
        case style::empty:
            return const_buffers_type(
                prepped_.begin(),
                prepped_.size());

        case style::buffers:
        {
            appender apndr(cb0_, is_chunked_);
            while(!apndr.is_full() && !filter_done_)
            {
                if(more_input_ && tmp_.size() == 0)
                {
                    tmp_ = buf_gen_->next();
                    if(tmp_.size() == 0) // buf_gen_ is empty
                        more_input_ = false;
                }

                const auto rs = filter_->process(
                    buffers::mutable_buffer_span(apndr.prepare()),
                    { tmp_, {} },
                    more_input_);

                if(rs.ec.failed())
                {
                    is_done_ = true;
                    return rs.ec;
                }

                tmp_ = buffers::sans_prefix(tmp_, rs.in_bytes);
                apndr.commit(rs.out_bytes, !rs.finished);

                if(rs.out_short)
                    break;

                if(rs.finished)
                    filter_done_ = true;
            }
            break;
        }

        case style::source:
        {
            appender apndr(cb0_, is_chunked_);
            while(!apndr.is_full() && !filter_done_)
            {
                if(more_input_ && cb1_.capacity() != 0)
                {
                    const auto rs = source_->read(
                        cb1_.prepare(cb1_.capacity()));
                    if(rs.ec.failed())
                    {
                        is_done_ = true;
                        return rs.ec;
                    }
                    if(rs.finished)
                        more_input_ = false;
                    cb1_.commit(rs.bytes);
                }

                const auto rs = filter_->process(
                    buffers::mutable_buffer_span(apndr.prepare()),
                    cb1_.data(),
                    more_input_);

                if(rs.ec.failed())
                {
                    is_done_ = true;
                    return rs.ec;
                }

                cb1_.consume(rs.in_bytes);
                apndr.commit(rs.out_bytes, !rs.finished);

                if(rs.out_short)
                    break;

                if(rs.finished)
                    filter_done_ = true;
            }
            break;
        }

        case style::stream:
        {
            appender apndr(cb0_, is_chunked_);

            if(apndr.is_full() || filter_done_)
                break;

            // The stream object is expected to
            // have already populated cb1_
            if(more_input_ && cb1_.size() == 0)
            {
                if(!prepped_.empty())
                    break;

                BOOST_HTTP_PROTO_RETURN_EC(
                    error::need_data);
            }

            const auto rs = filter_->process(
                buffers::mutable_buffer_span(apndr.prepare()),
                cb1_.data(),
                more_input_,
                prepped_.empty()); // force_flush

            if(rs.ec.failed())
            {
                is_done_ = true;
                return rs.ec;
            }

            cb1_.consume(rs.in_bytes);
            apndr.commit(rs.out_bytes, !rs.finished);

            if(rs.finished)
                filter_done_ = true;

            break;
        }
        }
    }

    prepped_.reset(!is_header_done_);
    const auto cbp = cb0_.data();
    if(cbp[0].size() != 0)
        prepped_.append(cbp[0]);
    if(cbp[1].size() != 0)
        prepped_.append(cbp[1]);

    BOOST_ASSERT(
        buffers::size(prepped_) > 0);

    return const_buffers_type(
        prepped_.begin(),
        prepped_.size());
}

void
serializer::
consume(
    std::size_t n)
{
    // Precondition violation
    if(is_done_ && n != 0)
        detail::throw_logic_error();

    if(!is_header_done_)
    {
        const auto header_remain =
            prepped_[0].size();
        if(n < header_remain)
        {
            prepped_.consume(n);
            return;
        }
        n -= header_remain;
        prepped_.consume(header_remain);
        is_header_done_ = true;
    }

    prepped_.consume(n);

    // no-op when cb0_ is not in use
    cb0_.consume(n);

    if(!prepped_.empty())
        return;

    if(needs_exp100_continue_)
        return;

    if(more_input_)
        return;

    if(filter_ && !filter_done_)
        return;

    is_done_ = true;
}

//------------------------------------------------

detail::array_of_const_buffers
serializer::
make_array(std::size_t n)
{
    if(n > std::numeric_limits<std::uint16_t>::max())
        detail::throw_length_error();

    return {
        ws_.push_array(n,
            buffers::const_buffer{}),
        static_cast<std::uint16_t>(n) };
}

void
serializer::
start_init(
    message_view_base const& m)
{
    reset();

    // VFALCO what do we do with
    // metadata error code failures?
    // m.ph_->md.maybe_throw();

    auto const& md = m.metadata();
    needs_exp100_continue_ = md.expect.is_100_continue;

    // Transfer-Encoding
    is_chunked_ = md.transfer_encoding.is_chunked;

    // Content-Encoding
    auto const& ce = md.content_encoding;
    if(ce.encoding == encoding::deflate)
    {
        filter_ = &ws_.emplace<filter>(
            ctx_,
            ws_,
            svc_.cfg.zlib_comp_level,
            svc_.cfg.zlib_window_bits,
            svc_.cfg.zlib_mem_level);
    }
    else if(ce.encoding == encoding::gzip)
    {
        filter_ = &ws_.emplace<filter>(
            ctx_,
            ws_,
            svc_.cfg.zlib_comp_level,
            svc_.cfg.zlib_window_bits + 16,
            svc_.cfg.zlib_mem_level);
    }
}

void
serializer::
start_empty(
    message_view_base const& m)
{
    using mutable_buffer =
        buffers::mutable_buffer;

    start_init(m);
    st_ = style::empty;

    if(!is_chunked_)
    {
        prepped_ = make_array(
            1); // header
    }
    else
    {
        prepped_ = make_array(
            1 + // header
            1); // final chunk

        mutable_buffer final_chunk = {
            ws_.reserve_front(
                final_chunk_len),
            final_chunk_len };
        write_final_chunk(final_chunk);

        prepped_[1] = final_chunk;
    }

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    more_input_ = false;
}

void
serializer::
start_buffers(
    message_view_base const& m)
{
    using mutable_buffer =
        buffers::mutable_buffer;

    // start_init() already called 
    st_ = style::buffers;

    const auto buffers_max = (std::min)(
        std::size_t{ 16 },
        buf_gen_->count());

    if(!filter_)
    {
        if(!is_chunked_)
        {
            // no filter and no chunked

            prepped_ = make_array(
                1 +            // header
                buffers_max ); // buffers

            prepped_[0] = { m.ph_->cbuf, m.ph_->size };
            std::generate(
                prepped_.begin() + 1,
                prepped_.end(),
                [this](){ return buf_gen_->next(); });
            more_input_ = !buf_gen_->is_empty();
            return;
        }

        // no filter and chunked

        if(buf_gen_->is_empty())
        {
            prepped_ = make_array(
                1 + // header
                1); // final chunk

            mutable_buffer final_chunk = {
                ws_.reserve_front(
                    final_chunk_len),
                final_chunk_len };
            write_final_chunk(
                final_chunk);

            prepped_[0] = { m.ph_->cbuf, m.ph_->size };
            prepped_[1] = final_chunk;
            more_input_ = false;
            return;
        }

        // Write entire buffers as a single chunk
        // since total size is known

        mutable_buffer chunk_header = {
            ws_.reserve_front(
                chunk_header_len),
            chunk_header_len };

        write_chunk_header(
            chunk_header,
            buf_gen_->size());

        mutable_buffer crlf_and_final_chunk = {
                ws_.reserve_front(
                    crlf_len + final_chunk_len),
                crlf_len + final_chunk_len };

        write_crlf(
            buffers::prefix(
                crlf_and_final_chunk,
                crlf_len));

        write_final_chunk(
            buffers::sans_prefix(
                crlf_and_final_chunk,
                crlf_len));

        prepped_ = make_array(
            1 + // header
            1 + // chunk header
            buffers_max + // buffers
            1); // buffer or (crlf and final chunk)

        prepped_[0] = { m.ph_->cbuf, m.ph_->size };
        prepped_[1] = chunk_header;
        std::generate(
            prepped_.begin() + 2,
            prepped_.end() - 1,
            [this](){ return buf_gen_->next(); });

        more_input_ = !buf_gen_->is_empty();
        // assigning the last slot
        if(more_input_)
        {
            prepped_[prepped_.size() - 1] =
                buf_gen_->next();

            // deferred until buf_gen_ is drained
            tmp_ = crlf_and_final_chunk;
        }
        else
        {
            prepped_[prepped_.size() - 1] =
                crlf_and_final_chunk;
        }
        return;
    }

    // filter

    prepped_ = make_array(
        1 + // header
        2); // circular buffer

    const auto n = ws_.size() - 1;
    cb0_ = { ws_.reserve_front(n), n };

    if(is_chunked_)
    {
        if(cb0_.capacity() <= chunked_overhead_)
            detail::throw_length_error();
    }
    else
    {
        if(cb0_.capacity() == 0)
            detail::throw_length_error();
    }

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    tmp_ = {};
    more_input_ = !buf_gen_->is_empty();
}

void
serializer::
start_source(
    message_view_base const& m)
{
    // start_init() already called 
    st_ = style::source;

    prepped_ = make_array(
        1 + // header
        2); // circular buffer

    if(filter_)
    {
        // TODO: Optimize buffer distribution
        const auto n = (ws_.size() - 1) / 2;
        cb0_ = { ws_.reserve_front(n), n };
        cb1_ = { ws_.reserve_front(n), n };
    }
    else
    {
        const auto n = ws_.size() - 1;
        cb0_ = { ws_.reserve_front(n), n };
    }

    if(is_chunked_)
    {
        if(cb0_.capacity() <= chunked_overhead_)
            detail::throw_length_error();
    }
    else
    {
        if(cb0_.capacity() == 0)
            detail::throw_length_error();
    }

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    more_input_ = true;
}

auto
serializer::
start_stream(
    message_view_base const& m) ->
        stream
{
    start_init(m);
    st_ = style::stream;

    prepped_ = make_array(
        1 + // header
        2); // circular buffer

    if(filter_)
    {
        // TODO: Optimize buffer distribution
        const auto n = (ws_.size() - 1) / 2;
        cb0_ = { ws_.reserve_front(n), n };
        cb1_ = { ws_.reserve_front(n), n };
    }
    else
    {
        const auto n = ws_.size() - 1;
        cb0_ = { ws_.reserve_front(n), n };
    }

    if(is_chunked_)
    {
        if(cb0_.capacity() <= chunked_overhead_)
            detail::throw_length_error();
    }
    else
    {
        if(cb0_.capacity() == 0)
            detail::throw_length_error();
    }

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    more_input_ = true;
    return stream{ *this };
}

//------------------------------------------------

std::size_t
serializer::
stream::
capacity() const noexcept
{
    if(sr_->filter_)
        return sr_->cb1_.capacity();

    if(!sr_->is_chunked_)
        return sr_->cb0_.capacity();

    // chunked with no filter
    const auto cap = sr_->cb0_.capacity();
    if(cap > chunked_overhead_)
        return cap - chunked_overhead_;

    return 0;
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
    if(sr_->filter_)
        return sr_->cb1_.prepare(
            sr_->cb1_.capacity());

    if(!sr_->is_chunked_)
        return sr_->cb0_.prepare(
            sr_->cb0_.capacity());

    // chunked with no filter
    const auto cap = sr_->cb0_.capacity();
    if(cap <= chunked_overhead_)
        detail::throw_length_error();

    return buffers::sans_prefix(
        sr_->cb0_.prepare(
            cap - crlf_len - final_chunk_len),
        chunk_header_len);
}

void
serializer::
stream::
commit(std::size_t n) const
{
    if(sr_->filter_)
        return sr_->cb1_.commit(n);

    if(!sr_->is_chunked_)
        return sr_->cb0_.commit(n);

    // chunked with no filter
    if(n != 0)
    {
        write_chunk_header(
            sr_->cb0_.prepare(
                chunk_header_len),
            n);
        sr_->cb0_.commit(
            chunk_header_len);

        sr_->cb0_.prepare(n);
        sr_->cb0_.commit(n);

        write_crlf(
            sr_->cb0_.prepare(crlf_len));
        sr_->cb0_.commit(crlf_len);
    }
}

void
serializer::
stream::
close() const
{
    // Precondition violation
    if(!sr_->more_input_)
        detail::throw_logic_error();

    sr_->more_input_ = false;

    if(sr_->filter_)
        return;

    if(!sr_->is_chunked_)
        return;

    // chunked with no filter
    write_final_chunk(
        sr_->cb0_.prepare(
            final_chunk_len));
    sr_->cb0_.commit(final_chunk_len);
}

} // http_proto
} // boost

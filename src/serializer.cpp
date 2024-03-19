//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/message_view_base.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/buffers/algorithm.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/core/ignore_unused.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

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

//------------------------------------------------

serializer::
~serializer()
{
}

serializer::
serializer()
    : serializer(65536)
{
}

serializer::
serializer(
    serializer&&) noexcept = default;

serializer::
serializer(
    std::size_t buffer_size)
    : ws_(buffer_size)
{
}

void
serializer::
reset() noexcept
{
}

//------------------------------------------------

auto
serializer::
prepare() ->
    system::result<
        const_buffers_type>
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    // Expect: 100-continue
    if(is_expect_continue_)
    {
        if(out_.data() == hp_)
            return const_buffers_type(hp_, 1);
        is_expect_continue_ = false;
        BOOST_HTTP_PROTO_RETURN_EC(
            error::expect_100_continue);
    }

    if(st_ == style::empty)
    {
        return const_buffers_type(
            out_.data(),
            out_.size());
    }

    if(st_ == style::buffers)
    {
        return const_buffers_type(
            out_.data(),
            out_.size());
    }

    if(st_ == style::source)
    {
        if(more_)
        {
            if(! is_chunked_)
            {
                auto rv = src_->read(
                    tmp0_.prepare(tmp0_.capacity()));
                tmp0_.commit(rv.bytes);
                if(rv.ec.failed())
                    return rv.ec;
                more_ = ! rv.finished;
            }
            else
            {
                if(tmp0_.capacity() > chunked_overhead_)
                {
                    auto dest = tmp0_.prepare(
                        tmp0_.capacity() -
                        2 - // CRLF
                        5); // final chunk

                    auto rv = src_->read(
                        buffers::sans_prefix(dest, 18));

                    if(rv.ec.failed())
                        return rv.ec;

                    if(rv.bytes != 0)
                    {
                        write_chunk_header(
                            buffers::prefix(dest, 18), rv.bytes);
                        tmp0_.commit(rv.bytes + 18);
                        // terminate chunk
                        tmp0_.commit(
                            buffers::buffer_copy(
                                tmp0_.prepare(2),
                                buffers::const_buffer(
                                    "\r\n", 2)));
                    }

                    if(rv.finished)
                    {
                        tmp0_.commit(
                            buffers::buffer_copy(
                                tmp0_.prepare(5),
                                buffers::const_buffer(
                                    "0\r\n\r\n", 5)));
                        more_ = false;
                    }
                }
            }
        }

        std::size_t n = 0;
        if(out_.data() == hp_)
            ++n;
        for(buffers::const_buffer const& b : tmp0_.data())
            out_[n++] = b;

        return const_buffers_type(
            out_.data(),
            out_.size());
    }

    if(st_ == style::stream)
    {
        std::size_t n = 0;
        if(out_.data() == hp_)
            ++n;
        if(tmp0_.size() == 0 && more_)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                error::need_data);
        }
        for(buffers::const_buffer const& b : tmp0_.data())
            out_[n++] = b;

        return const_buffers_type(
            out_.data(),
            out_.size());
    }

    // should never get here
    detail::throw_logic_error();
}

void
serializer::
consume(
    std::size_t n)
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    if(is_expect_continue_)
    {
        // Cannot consume more than
        // the header on 100-continue
        if(n > hp_->size())
            detail::throw_invalid_argument();

        out_.consume(n);
        return;
    }
    else if(out_.data() == hp_)
    {
        // consume header
        if(n < hp_->size())
        {
            out_.consume(n);
            return;
        }
        n -= hp_->size();
        out_.consume(hp_->size());
    }

    switch(st_)
    {
    default:
    case style::empty:
        out_.consume(n);
        if(out_.empty())
            is_done_ = true;
        return;

    case style::buffers:
        out_.consume(n);
        if(out_.empty())
            is_done_ = true;
        return;

    case style::source:
    case style::stream:
        tmp0_.consume(n);
        if( tmp0_.size() == 0 &&
                ! more_)
            is_done_ = true;
        return;
    }
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
    ws_.clear();

    // VFALCO what do we do with
    // metadata error code failures?
    // m.ph_->md.maybe_throw();

    is_done_ = false;

    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;

    // Transfer-Encoding
    {
        auto const& te =
            m.ph_->md.transfer_encoding;
        is_chunked_ = te.is_chunked;
    }
}

void
serializer::
start_empty(
    message_view_base const& m)
{
    start_init(m);

    st_ = style::empty;

    if(! is_chunked_)
    {
        out_ = make_array(
            1); // header
    }
    else
    {
        out_ = make_array(
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
        out_[1] = dest;
    }

    hp_ = &out_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
}

void
serializer::
start_buffers(
    message_view_base const& m)
{
    st_ = style::buffers;

    if(! is_chunked_)
    {
        //if(! cod_)
        {
            out_ = make_array(
                1 +             // header
                buf_.size());   // body
            copy(&out_[1],
                buf_.data(), buf_.size());
        }
#if 0
        else
        {
            out_ = make_array(
                1 + // header
                2); // tmp1
        }
#endif
    }
    else
    {
        //if(! cod_)
        {
            out_ = make_array(
                1 +             // header
                1 +             // chunk size
                buf_.size() +   // body
                1);             // final chunk
            copy(&out_[2],
                buf_.data(), buf_.size());

            // Buffer is too small
            if(ws_.size() < 18 + 7)
                detail::throw_length_error();
            buffers::mutable_buffer s1(ws_.data(), 18);
            buffers::mutable_buffer s2(ws_.data(), 18 + 7);
            s2 += 18; // VFALCO HACK
            write_chunk_header(
                s1,
                buffers::buffer_size(buf_));
            buffers::buffer_copy(s2, buffers::const_buffer(
                "\r\n"
                "0\r\n"
                "\r\n", 7));
            out_[1] = s1;
            out_[out_.size() - 1] = s2;
        }
#if 0
        else
        {
            out_ = make_array(
                1 +     // header
                2);     // tmp1
        }
#endif
    }

    hp_ = &out_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };
}

void
serializer::
start_source(
    message_view_base const& m,
    source* src)
{
    st_ = style::source;
    src_ = src;
    out_ = make_array(
        1 + // header
        2); // tmp
    //if(! cod_)
    {
        buffered_base::allocator a(
            ws_.data(), ws_.size()/2, false);
        src->init(a);
        ws_.reserve_front(a.size_used());

        tmp0_ = { ws_.data(), ws_.size() };
        if(tmp0_.capacity() <
                18 +    // chunk size
                1 +     // body (1 byte)
                2 +     // CRLF
                5)      // final chunk
            detail::throw_length_error();
    }
#if 0
    else
    {
        buffers::buffered_base::allocator a(
            ws_.data(), ws_.size()/3, false);
        src->init(a);
        ws_.reserve(a.size_used());

        auto const n = ws_.size() / 2;

        tmp0_ = { ws_.data(), ws_.size() / 2 };
        ws_.reserve(n);

        // Buffer is too small
        if(ws_.size() < 1)
            detail::throw_length_error();

        tmp1_ = { ws_.data(), ws_.size() };
    }
#endif

    hp_ = &out_[0];
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
    out_ = make_array(
        1 + // header
        2); // tmp
    //if(! cod_)
    {
        tmp0_ = { ws_.data(), ws_.size() };
        if(tmp0_.capacity() <
                18 +    // chunk size
                1 +     // body (1 byte)
                2 +     // CRLF
                5)      // final chunk
            detail::throw_length_error();
    }
#if 0
    else
    {
        auto const n = ws_.size() / 2;
        tmp0_ = { ws_.data(), n };
        ws_.reserve(n);

        // Buffer is too small
        if(ws_.size() < 1)
            detail::throw_length_error();

        tmp1_ = { ws_.data(), ws_.size() };
    }
#endif

    hp_ = &out_[0];
    *hp_ = { m.ph_->cbuf, m.ph_->size };

    more_ = true;

    return stream{*this};
}

//------------------------------------------------

auto
serializer::
source::
results::
operator+=(
    results const& rv) noexcept ->
        results&
{
    BOOST_ASSERT(! ec.failed());
    BOOST_ASSERT(! finished);
    ec = rv.ec;
    bytes += rv.bytes;
    finished = rv.finished;
    return *this;
}

auto
serializer::
source::
on_read(
    buffers::mutable_buffer_span bs) ->
        results
{
    results rv;
    auto it = bs.begin();
    auto const end_ = bs.end();
    if(it == end_)
        return rv;
    do
    {
        buffers::mutable_buffer b(*it++);
        rv += on_read(b);
        if(rv.ec.failed())
            return rv;
        if(rv.finished)
            break;
    }
    while(it != end_);
    return rv;
}

//------------------------------------------------

std::size_t
serializer::
stream::
capacity() const
{
    auto const n = 
        chunked_overhead_ +
            2 + // CRLF
            5;  // final chunk
    return sr_->tmp0_.capacity() - n; // VFALCO ?
}

std::size_t
serializer::
stream::
size() const
{
    return sr_->tmp0_.size();
}

auto
serializer::
stream::
prepare(
    std::size_t n) const ->
        buffers_type
{
    return sr_->tmp0_.prepare(n);
}

void
serializer::
stream::
commit(std::size_t n) const
{
    sr_->tmp0_.commit(n);
}

void
serializer::
stream::
close() const
{
    // Precondition violation
    if(! sr_->more_)
        detail::throw_logic_error();
    sr_->more_ = false;
}

//------------------------------------------------

} // http_proto
} // boost

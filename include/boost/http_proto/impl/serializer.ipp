//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SERIALIZER_IPP
#define BOOST_HTTP_PROTO_IMPL_SERIALIZER_IPP

#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/core/ignore_unused.hpp>
#include <stddef.h>

namespace boost {
namespace http_proto {

//------------------------------------------------

void
consume_buffers(
    const_buffer*& p,
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
    auto n = buffer_copy(
        (make_buffers)(dest0),
        const_buffer(buf, sizeof(buf)));
    ignore_unused(n);
    BOOST_ASSERT(n == 18);
    BOOST_ASSERT(
        buffer_size(dest0) == n);
}

//------------------------------------------------

class serializer::
    reserve
    : public source::reserve_fn
{
    serializer& sr_;
    std::size_t limit_;

public:
    reserve(
        serializer& sr,
        std::size_t limit) noexcept
        : sr_(sr)
        , limit_(limit)
    {
    }

    void*
    operator()(
        std::size_t n) const override
    {
        // You can only call reserve() once!
        if(! sr_.is_reserving_ )
            detail::throw_logic_error();

        // Requested n exceeds the limit
        if(n > limit_)
            detail::throw_length_error();

        sr_.is_reserving_ = false;
        return sr_.ws_.reserve(n);
    }
};

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
    std::size_t buffer_size)
    : ws_(buffer_size)
{
}

//------------------------------------------------

auto
serializer::
prepare() ->
    result<output>
{
    // Precondition violation
    if(is_done_)
        detail::throw_logic_error();

    // Expect: 100-continue
    if(is_expect_continue_)
    {
        BOOST_ASSERT(hp_ != nullptr);
        if(hp_->size() > 0)
            return output(hp_, 1);
        is_expect_continue_ = false;
        hp_ = nullptr;
        BOOST_HTTP_PROTO_RETURN_EC(
            error::expect_100_continue);
    }

    if(st_ == style::empty)
    {
        return output(pp_, pn_);
    }

    if(st_ == style::buffers)
    {
        return output(pp_, pn_);
    }

    if(st_ == style::source)
    {
        if(! is_chunked_)
        {
            auto rv = src_->read(
                dat1_.prepare(
                    dat1_.capacity() -
                        dat1_.size()));
            dat1_.commit(rv.bytes);
            if(rv.ec.failed())
                return rv.ec;
            more_ = rv.more;
        }
        else
        {
            if((dat1_.capacity() -
                    dat1_.size()) >
                chunked_overhead_)
            {
                auto dest = dat1_.prepare(18);
                write_chunk_header(dest, 0);
                dat1_.commit(18);
                auto rv = src_->read(
                    dat1_.prepare(
                        dat1_.capacity() -
                            2 - // CRLF
                            5 - // final chunk
                            dat1_.size()));
                dat1_.commit(rv.bytes);
                if(rv.bytes == 0)
                    dat1_.uncommit(18); // undo
                if(rv.ec.failed())
                    return rv.ec;
                if(rv.bytes > 0)
                {
                    // rewrite with correct size
                    write_chunk_header(
                        dest, rv.bytes);
                    // terminate chunk
                    dat1_.commit(buffer_copy(
                        dat1_.prepare(2),
                        const_buffer(
                            "\r\n", 2)));
                }
                if(! rv.more)
                {
                    dat1_.commit(buffer_copy(
                        dat1_.prepare(5),
                        const_buffer(
                            "0\r\n\r\n", 5)));
                }
                more_ = rv.more;
            }
        }

        std::size_t n = 0;
        if(hp_ != nullptr)
            ++n;
        for(const_buffer b : dat1_.data())
            pp_[n++] = b;
        return output(pp_, n);
    }

    if(st_ == style::stream)
    {
        std::size_t n = 0;
        if(hp_ != nullptr)
            ++n;
        if(dat1_.empty() && more_)
        {
            BOOST_HTTP_PROTO_RETURN_EC(
                error::need_data);
        }
        for(const_buffer b : dat1_.data())
            pp_[n++] = b;
        return output(pp_, n);
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
        BOOST_ASSERT(hp_ != nullptr);

        // Precondition violation
        if(n > hp_->size())
            detail::throw_invalid_argument();

        // consume header
        if(n < hp_->size())
        {
            *hp_ += n;
            return;
        }
        *hp_ = {};
        return;
    }
    else if(hp_ != nullptr)
    {
        // consume header
        if(n < hp_->size())
        {
            *hp_ += n;
            return;
        }
        n -= hp_->size();
        hp_ = nullptr;
        ++pp_;
        --pn_;
    }

    switch(st_)
    {
    default:
    case style::empty:
    case style::buffers:
        consume_buffers(
            pp_, pn_, n);
        if(pn_ == 0)
            is_done_ = true;
        return;

    case style::source:
    case style::stream:
        dat1_.consume(n);
        if( dat1_.empty() &&
                ! more_)
            is_done_ = true;
        return;
    }
}

//------------------------------------------------

void
serializer::
apply_param(
    brotli_decoder_t const&)
{
}

void
serializer::
apply_param(
    brotli_encoder_t const&)
{
}

void
serializer::
apply_param(
    deflate_decoder_t const&)
{
}

void
serializer::
apply_param(
    deflate_encoder_t const&)
{
}

void
serializer::
apply_param(
    gzip_decoder_t const&)
{
}

void
serializer::
apply_param(
    gzip_encoder_t const&)
{
}

//------------------------------------------------

void
serializer::
do_reserve(
    source& src,
    std::size_t limit)
{
    struct cleanup
    {
        bool& is_reserving;

        ~cleanup()
        {
            is_reserving = false;
        }
    };

    BOOST_ASSERT(! is_reserving_);
    cleanup c{is_reserving_};
    is_reserving_ = true;
    reserve fn(*this, limit);
    src.maybe_reserve(limit, fn);
}

void
serializer::
reset_empty_impl(
    message_view_base const& m)
{
    // Precondition violation
    if(ws_.size() < chunked_overhead_)
        detail::throw_length_error();

    ws_.clear();

    is_done_ = false;
    // VFALCO what about the error codes?
    // m.ph_->md.maybe_throw();
    is_chunked_ =
        m.ph_->md.transfer_encoding.is_chunked;
    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;
    //---
    st_ = style::empty;
    pp_ = ws_.push_array(
        2, const_buffer{});
    pn_ = 1;
    hp_ = pp_;
    *hp_ = {
        m.ph_->cbuf,
        m.ph_->size };
    if(is_chunked_)
    {
        detail::circular_buffer tmp(
            ws_.data(), ws_.size());
        tmp.commit(buffer_copy(
            tmp.prepare(5),
            const_buffer(
                "0\r\n\r\n", 5)));
        auto data = tmp.data();
        BOOST_ASSERT(
            data[1].size() == 0);
        pp_[pn_++] = data[0];
    }
}

void
serializer::
reset_buffers_impl(
    message_view_base const& m,
    const_buffer* pp,
    std::size_t pn)
{
    // Precondition violation
    if(ws_.size() < chunked_overhead_)
        detail::throw_length_error();

    is_done_ = false;
    // VFALCO what about the error codes?
    // m.ph_->md.maybe_throw();
    is_chunked_ =
        m.ph_->md.transfer_encoding.is_chunked;
    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;
    //---
    st_ = style::buffers;
    if(! is_chunked_)
    {
        pn_ = pn - 2;
        pp_ = pp + 1;
        hp_ = pp_;
        *hp_ = {
            m.ph_->cbuf,
            m.ph_->size };
    }
    else
    {
        std::size_t n = 0;
        for(std::size_t i = 2;
                i < pn - 1; ++i)
            n += pp[i].size();

        pn_ = pn;
        pp_ = pp;
        hp_ = pp_;
        *hp_ = {
            m.ph_->cbuf,
            m.ph_->size };

        detail::circular_buffer tmp{
            ws_.data(), ws_.size() };
        {
            auto dest = tmp.prepare(18);
            BOOST_ASSERT(dest[0].size() == 18);
            BOOST_ASSERT(dest[1].size() == 0);
            write_chunk_header(dest, n);
            tmp.commit(18);
            pp_[1] = dest[0];
            const_buffer fc(
                "\r\n"
                "0\r\n"
                "\r\n", 7);
            dest = tmp.prepare(fc.size());
            buffer_copy(dest, fc);
            BOOST_ASSERT(dest[0].size() == 7);
            BOOST_ASSERT(dest[1].size() == 0);
            pp_[pn_ - 1] = dest[0];
        }
    }
}

void
serializer::
reset_source_impl(
    message_view_base const& m,
    source* src)
{
    // Precondition violation
    if(ws_.size() <
        chunked_overhead_ +
            128) // reasonable lower limit
        detail::throw_length_error();

    is_done_ = false;
    // VFALCO what about the error codes?
    // m.ph_->md.maybe_throw();
    is_chunked_ =
        m.ph_->md.transfer_encoding.is_chunked;
    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;
    //---
    st_ = style::source;
    src_ = src;
    do_reserve(
        *src,
        ws_.size() / 2); // VFALCO can this underflow?
    pn_ =
        1 + // header
        2;  // dat1
    pp_ = ws_.push_array(
        pn_, const_buffer{});
    hp_ = pp_;
    *hp_ = {
        m.ph_->cbuf,
        m.ph_->size };
    dat1_ = {
        ws_.data(),
        ws_.size() };
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
    return sr_->dat1_.capacity() -
        n - 0;
}

std::size_t
serializer::
stream::
size() const
{
    return sr_->dat1_.size();
}

auto
serializer::
stream::
prepare(
    std::size_t n) const ->
        buffers_type
{
    return sr_->dat1_.prepare(n);
}

void
serializer::
stream::
commit(std::size_t n) const
{
    sr_->dat1_.commit(n);
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

void
serializer::
reset_stream_impl(
    message_view_base const& m,
    source& src)
{
    // Precondition violation
    if(ws_.size() <
        chunked_overhead_ +
            128) // reasonable lower limit
        detail::throw_length_error();

    is_done_ = false;
    // VFALCO what about the error codes?
    // m.ph_->md.maybe_throw();
    is_chunked_ =
        m.ph_->md.transfer_encoding.is_chunked;
    is_expect_continue_ =
        m.ph_->md.expect.is_100_continue;
    //---
    st_ = style::stream;
    do_reserve(
        src,
        ws_.size() / 2); // VFALCO can this underflow?
    pn_ =
        1 + // header
        2;  // dat1
    pp_ = ws_.push_array(
        pn_, const_buffer{});
    hp_ = pp_;
    *hp_ = {
        m.ph_->cbuf,
        m.ph_->size };
    dat1_ = {
        ws_.data(),
        ws_.size() };
    more_ = true;
}

//------------------------------------------------

} // http_proto
} // boost

#endif

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
#include <stddef.h>

namespace boost {
namespace http_proto {

//------------------------------------------------
/*

write algorithm:

    get buffers from serializer
        - can fail with ec
        - can perform blocking I/O

    write buffers to socket
        - consume the written amount

serializer supply algorithm
    - supply a const buffer sequence for everything
        serializer sr;
        sr.set_body( cb );
    - supply a const buffer sequence iteratively
        serializer sr;
        sr.set_body_some( cb );
    - write into serializer-provided buffers iteratively
        sr.set_body( source( ... ) );
*/
//------------------------------------------------

serializer::
~serializer()
{
}

serializer::
serializer(
    std::size_t buffer_size)
    : ws_(buffer_size)
{
}

void
serializer::
reset(
    message_view_base const& m) noexcept
{
    ws_.clear();
    h_ = m.ph_;
    src_ = nullptr;
    st_ = state::init;

    cb_ = nullptr;
    cbn_ = 0;
    cbi_ = 0;
}

//------------------------------------------------

auto
serializer::
prepare() ->
    result<buffers>
{
    if(st_ == state::init)
        init_impl();

    //if(! expect:100 continue)

    if(src_)
    {
        // source body
        auto cb0 = cb_;
        const_buffer* p;
        if(hbuf_.size() > 0)
        {
            *cb0 = hbuf_;
            p = cb0 + 1;
        }
        else
        {
            ++cb0;
            p = cb0;
        }
        ++p;
        auto dest = buf_.prepare();
        auto rv = src_->read(
            dest.first.data(),
            dest.first.size());
        if(rv.has_error())
            return rv.error();
        buf_.commit(rv->bytes);
        if( rv->more &&
            dest.second.size() > 0)
        {
            rv = src_->read(
                dest.second.data(),
                dest.second.size());
            if(rv.has_error())
                return rv.error();
            buf_.commit(rv->bytes);
        }
        more_ = rv->more;
        auto src = buf_.data();
        *p++ = src.first;
        if(src.second.size() > 0)
            *p++ = src.second;
        return buffers(cb0, p - cb0);
    }

    // buffers body
    if(hbuf_.size() > 0)
    {
        BOOST_ASSERT(cbi_ == 0);
        cb_[0] = hbuf_;
        return buffers(cb_, 1 + cbn_);
    }
    return buffers(
        &cb_[1 + cbi_], cbn_ - cbi_);
}

void
serializer::
consume(
    std::size_t n) noexcept
{
    BOOST_ASSERT(
        st_ == state::ok);

    // header
    if(hbuf_.size() > 0)
    {
        if(n <= hbuf_.size())
        {
            hbuf_ += n;
            return;
        }

        n -= hbuf_.size();
        hbuf_ = {};
    }

    if(src_)
    {
        // source body
        buf_.consume(n);

        if( buf_.empty() &&
            ! more_)
        {
            st_ = state::done;
        }
    }
    else
    {
        // buffers body
        while(n > 0)
        {
            // n was out of range
            BOOST_ASSERT(cbi_ != cbn_);
            auto i = cbi_ + 1;
            if(n < cb_[i].size())
            {
                cb_[i] += n;
                return;
            }

            n -= cb_[i].size();
            ++cbi_;
        }
        st_ = state::done;
    }
}

//------------------------------------------------

void
serializer::
init_impl()
{
    BOOST_ASSERT(st_ == state::init);

    // header
    hbuf_ = { h_->buf, h_->size };

    if(src_)
    {
        // source body
        buf_ = { ws_.data(), ws_.size() };
    }
    else
    {
        // buffers body
    }

    st_ = state::ok;
}

} // http_proto
} // boost

#endif

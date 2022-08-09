//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BUFFERED_SOURCE_HPP
#define BOOST_HTTP_PROTO_BUFFERED_SOURCE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/circular_buffer.hpp>
#include <boost/http_proto/source.hpp>

namespace boost {
namespace http_proto {

template<class Writer>
class buffered_source
    : public source
{
    circular_buffer cb_;
    Writer w_;
    char* p_ = nullptr;
    std::size_t n_ = 0;

public:
#if 0
    template<class... Args>
    buffered_source(
        char* buf,
        std::size_t size,
        Args&&... args)
        : cb_(buf, size)
        , w_(std::forward<
            Args>(args)...)
    {
    }
#else
    buffered_source() = delete;
    buffered_source(
        buffered_source&&) = default;

    template<class... Args>
    buffered_source(
        int,
        Args&&... args)
        : cb_(
            new char[4096],
            4096)
        , w_(std::forward<
            Args>(args)...)
    {
    }

    ~buffered_source()
    {
        //if(p_)
            //delete[] p_;
    }
#endif

    bool
    more() const noexcept override
    {
        return w_.more();
    }

    void
    prepare(
        const_buffers& cb,
        error_code& ec) override
    {
        auto b = cb_.prepare(
            cb_.capacity());
        auto it = b.begin();
        auto n = w_.write(
            it->data(),
            it->size(),
            ec);
        cb_.commit(n);
        if(ec.failed())
            return;
        ++it;
        if(it == b.end())
        {
            cb = cb_.data();
            return;
        }
        n += w_.write(
            it->data(),
            it->size(),
            ec);
        cb_.commit(n);
        if(ec.failed())
            return;
        cb = cb_.data();
    }

    void
    consume(
        std::size_t n) noexcept override
    {
        cb_.consume(n);
    }
};

} // http_proto
} // boost

#endif

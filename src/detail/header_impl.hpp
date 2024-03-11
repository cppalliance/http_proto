//
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_HEADER_IMPL_HPP
#define BOOST_HTTP_PROTO_DETAIL_HEADER_IMPL_HPP

#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/core/span.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {
namespace detail {

struct prefix_op
{
    message_base& mb_;
    span<char> prefix_;
    char* buf_ = nullptr;
    std::size_t n_ = 0;

    prefix_op(
        message_base& mb,
        std::size_t n)
    : mb_{mb}
    , n_{n}
    {
        auto& h = mb_.h_;
        if( h.buf && n <= h.prefix )
        {
            prefix_ = {h.buf, n};
            return;
        }

        // allocate or grow
        if( n > h.prefix &&
            static_cast<std::size_t>(n - h.prefix) >
            static_cast<std::size_t>(max_offset - h.size) )
        {
            throw_length_error();
        }

        auto n1 = header::bytes_needed(
            n + h.size - h.prefix,
            h.count);

        auto p = new char[n1];
        if( h.buf != nullptr )
        {
            std::memcpy(
                p + n,
                h.buf + h.prefix,
                h.size - h.prefix);
            h.copy_table(p + n1);
        }
        else
        {
            std::memcpy(
                p + n,
                h.cbuf + h.prefix,
                h.size - h.prefix);
        }

        prefix_ = {p, n};
        buf_ = h.buf;

        h.buf = p;
        h.cbuf = p;
        h.size = static_cast<
            offset_type>(h.size +
                n - h.prefix);
        h.prefix = static_cast<
            offset_type>(n);
        h.cap = n1;
    }

    prefix_op(prefix_op&&) = delete;
    prefix_op(prefix_op const&) = delete;

    ~prefix_op()
    {
        auto& h = mb_.h_;
        if( n_ < h.prefix )
        {
            std::memmove(
                h.buf + n_,
                h.buf + h.prefix,
                h.size - h.prefix);
            h.size = static_cast<
                offset_type>(h.size -
                    h.prefix + n_);
            h.prefix = static_cast<
                offset_type>(n_);
        }
        delete[] buf_;
    }
};

} // detail
} // http_proto
} // boost

#endif // BOOST_HTTP_PROTO_DETAIL_HEADER_IMPL_HPP

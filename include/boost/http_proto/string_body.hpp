//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STRING_BODY_HPP
#define BOOST_HTTP_PROTO_STRING_BODY_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/source.hpp>
#include <boost/http_proto/string_view.hpp>
#include <string>
#include <utility>

namespace boost {
namespace http_proto {

class string_body
    // : public source
{
    std::string s_;

public:
    explicit
    string_body(
        string_view s)
        : s_(s)
    {
    }

    explicit
    string_body(
        std::string&& s) noexcept
        : s_(std::move(s))
    {
    }

    string_body(string_body&&) = default;
    string_body(string_body const&) = delete;

    string_view
    next();
};

/*

class BufferedBody
{
public:
    void
    
};

class StreamingBody
{
public:
    result<std::size_t>
    write(
        void* dest,
        std::size_t size);
};

*/

} // http_proto
} // boost

#endif

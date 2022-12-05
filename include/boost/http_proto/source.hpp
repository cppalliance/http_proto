//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SOURCE_HPP
#define BOOST_HTTP_PROTO_SOURCE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** A source of body data
*/
struct source
{
    struct amount
    {
        std::size_t bytes = 0;
        bool more = false;
    };

    virtual
    ~source() = 0;

    virtual
    result<amount>
    read(
        void* dest,
        std::size_t size) = 0;
};

} // http_proto
} // boost

#endif

//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP
#define BOOST_HTTP_PROTO_RESPONSE_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

class response_view
{
    friend class response;
    friend class response_parser;

public:
    BOOST_HTTP_PROTO_DECL
    response_view() noexcept;
};

} // http_proto
} // boost

#endif

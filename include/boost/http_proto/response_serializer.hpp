//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_RESPONSE_SERIALIZER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/version.hpp>

namespace boost {
namespace http_proto {

class BOOST_SYMBOL_VISIBLE
    response_serializer
    : public serializer
{
public:
#if 0
    BOOST_HTTP_PROTO_DECL
    void
    reset(response_view const& prv);
#endif
};

} // http_proto
} // boost

#endif

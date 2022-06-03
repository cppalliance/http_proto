//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_REQUEST_SERIALIZER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/serializer.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class request;
class request_view;
#endif

class BOOST_SYMBOL_VISIBLE
    request_serializer
    : public serializer
{
public:
    BOOST_HTTP_PROTO_DECL
    void
    set_header(
        request_view const& res);

    BOOST_HTTP_PROTO_DECL
    void
    set_header(request const& res);
};

} // http_proto
} // boost

#endif

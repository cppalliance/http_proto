//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_MIME_TYPES_SERVICE_HPP
#define BOOST_HTTP_PROTO_SERVICE_MIME_TYPES_SERVICE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

struct mime_type
{
    string_view value;
    string_view type;
    string_view subtype;
};

/** Service for mime types
*/
class BOOST_SYMBOL_VISIBLE
    mime_types_service
{
public:
    /** Return the mime-type for a given extension

        @param s The file extension
    */
    BOOST_HTTP_PROTO_DECL
    virtual
    mime_type
    find(
        string_view ext) const noexcept = 0;
};

/** Install the static mime-types service
*/
mime_types_service&
install_mime_types_service(
    context& ctx);

} // http_proto
} // boost

#endif

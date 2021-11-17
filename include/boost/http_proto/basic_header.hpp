//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BASIC_HEADER_HPP
#define BOOST_HTTP_PROTO_BASIC_HEADER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** Base class for all header types

    @see
        @ref headers, @ref headers_view,
        @ref request, @ref request_view,
        @ref response, @ref response_view
*/
class basic_header
{
public:
    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    virtual
    ~basic_header();

    /** Returns a string holding the serialized protocol element

        This function returns a string representing
        the serialized form of the protocol element.

        @par Lifetime

        Ownership of the string is not transferred;
        the string returned is a reference to memory
        owned by the object and remains valid until:

        @li The object is destroyed, or

        @li Any non-const member function of the object
        is invoked.
    */
    BOOST_HTTP_PROTO_DECL
    virtual
    string_view
    get_const_buffer() const noexcept = 0;
};

} // http_proto
} // boost

#endif

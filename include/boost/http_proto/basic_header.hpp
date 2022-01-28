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
#include <cstdint>

namespace boost {
namespace http_proto {

/** Base class for all header types

    @see
        @ref fields, @ref fields_view,
        @ref request, @ref request_view,
        @ref response, @ref response_view
*/
class BOOST_SYMBOL_VISIBLE
    basic_header
{
#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif
    std::uint64_t content_length_;
    bool has_chunked_ : 1;
    bool has_content_length_ : 1;

    struct ctor_params
    {
        std::uint64_t content_length = 0;
        bool has_chunked = false;
        bool has_content_length = false;
    };

    basic_header() noexcept;

    basic_header(
        ctor_params const& init) noexcept;

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
    buffer() const noexcept = 0;

    bool
    has_content_length() const noexcept
    {
        return has_content_length_;
    }
};

} // http_proto
} // boost

#endif

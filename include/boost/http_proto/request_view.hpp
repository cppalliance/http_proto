//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_VIEW_HPP
#define BOOST_HTTP_PROTO_REQUEST_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
enum class method : char;
enum class version : char;
#endif

/** A read-only reference to an HTTP request
*/
class BOOST_SYMBOL_VISIBLE
    request_view
    : public fields_view_base
{
    friend class request;
    friend class request_parser;

#ifndef BOOST_HTTP_PROTO_DOCS
protected:
#endif

    explicit
    request_view(
        detail::header const& h) noexcept
        : fields_view_base(h)
    {
        BOOST_ASSERT(h.kind ==
            detail::kind::request);
    }

public:
    /** Constructor
    */
    request_view() noexcept
        : fields_view_base(
            detail::kind::request)
    {
    }

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    request_view(
        request_view const&) noexcept;

    /** Assignment
    */
    BOOST_HTTP_PROTO_DECL
    request_view&
    operator=(request_view const&) noexcept;

    /** Return the known method constant
    */
    http_proto::method
    method() const noexcept
    {
        return h_.req.method;
    };

    /** Return the exact method string
    */
    string_view
    method_str() const noexcept
    {
        return string_view(
            h_.cbuf, h_.req.method_len);
    }

    /** Return the request-target string
    */
    string_view
    target() const noexcept
    {
        return string_view(
            h_.cbuf +
                h_.req.method_len + 1,
            h_.req.target_len);
    }

    /** Return the HTTP-version
    */
    http_proto::version
    version() const noexcept
    {
        return h_.req.version;
    }
};

} // http_proto
} // boost

#endif

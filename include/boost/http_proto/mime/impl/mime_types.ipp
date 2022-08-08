//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_MIME_IMPL_MIME_TYPES_IPP
#define BOOST_HTTP_PROTO_MIME_IMPL_MIME_TYPES_IPP

#include <boost/http_proto/mime/mime_types.hpp>
#include <boost/http_proto/context.hpp>

namespace boost {
namespace http_proto {

mime_types::
~mime_types() noexcept = default;

namespace detail {

class mime_types_impl
    : public mime_types
    , public context::service
{
public:
    using key_type = mime_types;

    explicit
    mime_types_impl(
        context&) noexcept
    {
    }

    string_view
    content_type(
        string_view s) const noexcept override
    {
        return s;
    }
};

mime_types&
install_mime_types_service(
    context& ctx)
{
    return make_service<mime_types_impl>(ctx);
}

} // detail

} // http_proto
} // boost

#endif

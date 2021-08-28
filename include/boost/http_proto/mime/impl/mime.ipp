//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_MIME_IMPL_MIME_IPP
#define BOOST_HTTP_PROTO_MIME_IMPL_MIME_IPP

#include <boost/http_proto/mime/mime.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/bnf/ctype.hpp>

namespace boost {
namespace http_proto {

mime::
~mime() noexcept = default;

namespace detail {

class mime_impl
    : public mime
    , public context::service
{
public:
    using key_type = mime;

    explicit
    mime_impl(
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

mime&
install_mime_service(
    context& ctx)
{
    return make_service<mime_impl>(ctx);
}

} // detail

} // http_proto
} // boost

#endif

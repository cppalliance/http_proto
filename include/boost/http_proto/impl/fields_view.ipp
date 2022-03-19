//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_IPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_VIEW_IPP

#include <boost/http_proto/fields_view.hpp>

namespace boost {
namespace http_proto {

fields_view::
fields_view(
    fields_view const&) noexcept = default;

fields_view&
fields_view::
operator=(
    fields_view const&) noexcept = default;

//------------------------------------------------

void
fields_view::
swap(fields_view& other) noexcept
{
    this->fields_view_base::swap(other);
}

} // http_proto
} // boost

#endif

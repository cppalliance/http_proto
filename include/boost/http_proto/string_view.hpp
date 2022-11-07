//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_STRING_VIEW_HPP
#define BOOST_HTTP_PROTO_STRING_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/core/detail/string_view.hpp>

namespace boost {
namespace http_proto {

/** The type of string_view used by the library

    String views are used to pass character
    buffers into or out of functions. Ownership
    of the underlying character buffer is not
    transferred; the caller is responsible for
    ensuring that the lifetime of character
    buffer extends until it is no longer
    referenced.
*/
typedef boost::core::string_view string_view;

} // http_proto
} // boost

#endif

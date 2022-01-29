//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_CONFIG_HPP
#define BOOST_HTTP_PROTO_DETAIL_CONFIG_HPP

#include <boost/config.hpp>
#include <stdint.h>

namespace boost {

namespace http_proto {

#if defined(BOOST_HTTP_PROTO_DOCS)
# define BOOST_HTTP_PROTO_DECL
# define BOOST_HTTP_PROTO_PROTECTED private
#else
# define BOOST_HTTP_PROTO_PROTECTED protected
# if (defined(BOOST_HTTP_PROTO_DYN_LINK) || defined(BOOST_ALL_DYN_LINK)) && !defined(BOOST_HTTP_PROTO_STATIC_LINK)
#  if defined(BOOST_HTTP_PROTO_SOURCE)
#   define BOOST_HTTP_PROTO_DECL        BOOST_SYMBOL_EXPORT
#   define BOOST_HTTP_PROTO_CLASS_DECL  BOOST_SYMBOL_EXPORT
#   define BOOST_HTTP_PROTO_BUILD_DLL
#  else
#   define BOOST_HTTP_PROTO_DECL        BOOST_SYMBOL_IMPORT
#   define BOOST_HTTP_PROTO_CLASS_DECL  BOOST_SYMBOL_IMPORT
#  endif
# endif // shared lib
# ifndef  BOOST_HTTP_PROTO_DECL
#  define BOOST_HTTP_PROTO_DECL
# endif
# if !defined(BOOST_HTTP_PROTO_SOURCE) && !defined(BOOST_ALL_NO_LIB) && !defined(BOOST_HTTP_PROTO_NO_LIB)
#  define BOOST_LIB_NAME boost_json
#  if defined(BOOST_ALL_DYN_LINK) || defined(BOOST_HTTP_PROTO_DYN_LINK)
#   define BOOST_DYN_LINK
#  endif
#  include <boost/config/auto_link.hpp>
# endif
#endif

using off_t = ::uint16_t; // private

// maximum size of http header,
// chunk header, or chunk extensions
#ifndef BOOST_HTTP_PROTO_MAX_HEADER
#define BOOST_HTTP_PROTO_MAX_HEADER 65535U
#endif
static constexpr auto max_off_t =
    BOOST_HTTP_PROTO_MAX_HEADER;

} // http_proto

// lift grammar into our namespace
namespace urls {
namespace grammar {}
}
namespace http_proto {
namespace grammar = ::boost::urls::grammar;
} // http_proto

} // boost

#endif

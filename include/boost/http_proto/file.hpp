//
// Copyright (c) 2022 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILE_HPP
#define BOOST_HTTP_PROTO_FILE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/file_base.hpp>
#include <boost/http_proto/file_posix.hpp>
#include <boost/http_proto/file_stdio.hpp>
#include <boost/http_proto/file_win32.hpp>

namespace boost {
namespace http_proto {

/** An implementation of File.

    This alias is set to the best available implementation
    of <em>File</em> given the platform and build settings.
*/
#if BOOST_HTTP_PROTO_DOCS
struct file : file_stdio
{
};
#else
#if BOOST_HTTP_PROTO_USE_WIN32_FILE
using file = file_win32;
#elif BOOST_BEAST_USE_POSIX_FILE
using file = file_posix;
#else
using file = file_stdio;
#endif
#endif

} // http_proto
} // boost

#endif

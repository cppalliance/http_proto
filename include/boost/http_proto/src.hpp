//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_SRC_HPP
#define BOOST_HTTP_PROTO_SRC_HPP

/*

This file is meant to be included once,
in a translation unit of the program.

*/

#ifndef BOOST_HTTP_PROTO_SOURCE
#define BOOST_HTTP_PROTO_SOURCE
#endif

// We include this in case someone is
// using src.hpp as their main header file
#include <boost/http_proto.hpp>

#include <boost/http_proto/detail/impl/except.ipp>
#include <boost/http_proto/detail/impl/rfc7230.ipp>

#include <boost/http_proto/impl/basic_parser.ipp>
#include <boost/http_proto/impl/connection.ipp>
#include <boost/http_proto/impl/context.ipp>
#include <boost/http_proto/impl/ctype.ipp>
#include <boost/http_proto/impl/decoder.ipp>
#include <boost/http_proto/impl/deflate_service.ipp>
#include <boost/http_proto/impl/encoder.ipp>
#include <boost/http_proto/impl/error.ipp>
#include <boost/http_proto/impl/field.ipp>
#include <boost/http_proto/impl/fields_view.ipp>
#include <boost/http_proto/impl/method.ipp>
#include <boost/http_proto/impl/request_parser.ipp>
#include <boost/http_proto/impl/request_view.ipp>
#include <boost/http_proto/impl/response_parser.ipp>
#include <boost/http_proto/impl/response_view.ipp>
#include <boost/http_proto/impl/rfc.ipp>
#include <boost/http_proto/impl/status.ipp>
#include <boost/http_proto/impl/token_list.ipp>
#include <boost/http_proto/impl/transfer_param_list.ipp>

// VFALCO These are tucked away here temporarily
#include <boost/beast/zlib/detail/deflate_stream.ipp>
#include <boost/beast/zlib/detail/inflate_stream.ipp>
#include <boost/beast/zlib/impl/error.ipp>

#endif

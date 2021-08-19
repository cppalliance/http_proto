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

#include <boost/http_proto/impl/context.ipp>
#include <boost/http_proto/impl/decoder.ipp>
#include <boost/http_proto/impl/deflate_service.ipp>
#include <boost/http_proto/impl/encoder.ipp>
#include <boost/http_proto/impl/error.ipp>
#include <boost/http_proto/impl/field.ipp>
#include <boost/http_proto/impl/fields.ipp>
#include <boost/http_proto/impl/fields_view.ipp>
#include <boost/http_proto/impl/headers.ipp>
#include <boost/http_proto/impl/headers_view.ipp>
#include <boost/http_proto/impl/method.ipp>
#include <boost/http_proto/impl/parser.ipp>
#include <boost/http_proto/impl/request.ipp>
#include <boost/http_proto/impl/request_parser.ipp>
#include <boost/http_proto/impl/request_view.ipp>
#include <boost/http_proto/impl/response.ipp>
#include <boost/http_proto/impl/response_parser.ipp>
#include <boost/http_proto/impl/response_view.ipp>
#include <boost/http_proto/impl/status.ipp>
#include <boost/http_proto/impl/version.ipp>

#include <boost/http_proto/bnf/impl/chunk_ext.ipp>
#include <boost/http_proto/bnf/impl/chunk_part.ipp>
#include <boost/http_proto/bnf/impl/ctype.ipp>
#include <boost/http_proto/bnf/impl/header_fields.ipp>
#include <boost/http_proto/bnf/impl/number.ipp>
#include <boost/http_proto/bnf/impl/quoted_string.ipp>
#include <boost/http_proto/bnf/impl/request_line.ipp>
#include <boost/http_proto/bnf/impl/token.ipp>
#include <boost/http_proto/bnf/impl/transfer_encoding.ipp>
#include <boost/http_proto/bnf/impl/transfer_param_list.ipp>

#include <boost/http_proto/bnf/detail/impl/rfc7230.ipp>

// VFALCO These are tucked away here temporarily
#include <boost/beast/zlib/detail/deflate_stream.ipp>
#include <boost/beast/zlib/detail/inflate_stream.ipp>
#include <boost/beast/zlib/impl/error.ipp>

#endif

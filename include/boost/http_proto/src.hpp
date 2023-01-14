//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
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
//#include <boost/http_proto.hpp>

#include <boost/http_proto/detail/impl/circular_buffer.ipp>
#include <boost/http_proto/detail/impl/consuming_buffers.ipp>
#include <boost/http_proto/detail/impl/except.ipp>
#include <boost/http_proto/detail/impl/header.ipp>
#include <boost/http_proto/detail/impl/workspace.ipp>

#include <boost/http_proto/impl/context.ipp>
#include <boost/http_proto/impl/error.ipp>
#include <boost/http_proto/impl/field.ipp>
#include <boost/http_proto/impl/fields.ipp>
#include <boost/http_proto/impl/fields_base.ipp>
#include <boost/http_proto/impl/fields_view_base.ipp>

#include <boost/http_proto/impl/file_body.ipp>
#include <boost/http_proto/impl/file_posix.ipp>
#include <boost/http_proto/impl/file_stdio.ipp>
#include <boost/http_proto/impl/file_win32.ipp>

#include <boost/http_proto/impl/message_base.ipp>
#include <boost/http_proto/impl/message_view_base.ipp>
#include <boost/http_proto/impl/method.ipp>
#include <boost/http_proto/impl/parser.ipp>
#include <boost/http_proto/impl/request.ipp>
#include <boost/http_proto/impl/request_parser.ipp>
#include <boost/http_proto/impl/response.ipp>
#include <boost/http_proto/impl/response_parser.ipp>
#include <boost/http_proto/impl/serializer.ipp>
#include <boost/http_proto/impl/source.ipp>
#include <boost/http_proto/impl/status.ipp>
#include <boost/http_proto/impl/version.ipp>

#include <boost/http_proto/rfc/impl/combine_field_values.ipp>
#include <boost/http_proto/rfc/impl/parameter.ipp>
#include <boost/http_proto/rfc/impl/quoted_token_rule.ipp>
#include <boost/http_proto/rfc/impl/transfer_encoding_rule.ipp>
#include <boost/http_proto/rfc/impl/upgrade_rule.ipp>

#include <boost/http_proto/rfc/detail/impl/rules.ipp>

#include <boost/http_proto/service/impl/mime_types_service.ipp>
#include <boost/http_proto/service/impl/service.ipp>

// VFALCO These are tucked away here temporarily
#if 0
#include <boost/beast/zlib/detail/deflate_stream.ipp>
#include <boost/beast/zlib/detail/inflate_stream.ipp>
#include <boost/beast/zlib/impl/error.ipp>
#endif

#endif

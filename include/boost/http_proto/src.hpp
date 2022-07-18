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
#include <boost/http_proto.hpp>

#include <boost/http_proto/detail/impl/except.ipp>
#include <boost/http_proto/detail/impl/header.ipp>

#include <boost/http_proto/impl/context.ipp>
#include <boost/http_proto/impl/error.ipp>
#include <boost/http_proto/impl/field.ipp>
#include <boost/http_proto/impl/fields.ipp>
#include <boost/http_proto/impl/fields_base.ipp>
#include <boost/http_proto/impl/fields_view_base.ipp>

#include <boost/http_proto/impl/file_posix.ipp>
#include <boost/http_proto/impl/file_win32.ipp>
#include <boost/http_proto/impl/file_source.ipp>
#include <boost/http_proto/impl/file_stdio.ipp>

#include <boost/http_proto/impl/filter.ipp>
#include <boost/http_proto/impl/method.ipp>
#include <boost/http_proto/impl/parser.ipp>
#include <boost/http_proto/impl/request.ipp>
#include <boost/http_proto/impl/request_parser.ipp>
#include <boost/http_proto/impl/response.ipp>
#include <boost/http_proto/impl/response_parser.ipp>
#include <boost/http_proto/impl/serializer.ipp>
#include <boost/http_proto/impl/status.ipp>
#include <boost/http_proto/impl/version.ipp>

#include <boost/http_proto/bnf/impl/chunk_part.ipp>
#include <boost/http_proto/bnf/impl/ctype.ipp>
#include <boost/http_proto/bnf/impl/number.ipp>
#include <boost/http_proto/bnf/impl/token.ipp>
#include <boost/http_proto/bnf/impl/transfer_encoding.ipp>
#include <boost/http_proto/bnf/impl/transfer_param_list.ipp>

#include <boost/http_proto/codec/impl/codecs.ipp>
#include <boost/http_proto/codec/impl/decoder.ipp>
#include <boost/http_proto/codec/impl/deflate_service.ipp>
#include <boost/http_proto/codec/impl/encoder.ipp>

#include <boost/http_proto/mime/impl/mime_types.ipp>

#include <boost/http_proto/rfc/impl/chunk_ext_rule.ipp>
#include <boost/http_proto/rfc/impl/digits_rule.ipp>
#include <boost/http_proto/rfc/impl/field_rule.ipp>
#include <boost/http_proto/rfc/impl/method_rule.ipp>
#include <boost/http_proto/rfc/impl/quoted_string_rule.ipp>
#include <boost/http_proto/rfc/impl/reason_phrase_rule.ipp>
#include <boost/http_proto/rfc/impl/request_line_rule.ipp>
#include <boost/http_proto/rfc/impl/request_target_rule.ipp>
#include <boost/http_proto/rfc/impl/status_code_rule.ipp>
#include <boost/http_proto/rfc/impl/status_line_rule.ipp>
#include <boost/http_proto/rfc/impl/version_rule.ipp>

// VFALCO These are tucked away here temporarily
#if 0
#include <boost/beast/zlib/detail/deflate_stream.ipp>
#include <boost/beast/zlib/detail/inflate_stream.ipp>
#include <boost/beast/zlib/impl/error.ipp>
#endif

#endif

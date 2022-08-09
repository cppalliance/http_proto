//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_HPP
#define BOOST_HTTP_PROTO_HPP

#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/buffered_source.hpp>
#include <boost/http_proto/circular_buffer.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/http_proto/file.hpp>
#include <boost/http_proto/file_base.hpp>
#include <boost/http_proto/file_posix.hpp>
#include <boost/http_proto/file_win32.hpp>
#include <boost/http_proto/file_source.hpp>
#include <boost/http_proto/file_stdio.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/source.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/version.hpp>

#include <boost/http_proto/codec/codecs.hpp>
#include <boost/http_proto/codec/decoder.hpp>
#include <boost/http_proto/codec/deflate_service.hpp>
#include <boost/http_proto/codec/encoder.hpp>

#include <boost/http_proto/mime/mime_types.hpp>

#include <boost/http_proto/rfc/token_rule.hpp>

#endif

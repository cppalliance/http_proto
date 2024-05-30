//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_HPP
#define BOOST_HTTP_PROTO_HPP

#include <boost/http_proto/buffered_base.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/deflate.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/fields_view_base.hpp>
#include <boost/http_proto/file.hpp>
#include <boost/http_proto/file_base.hpp>
#include <boost/http_proto/file_body.hpp>
#include <boost/http_proto/file_posix.hpp>
#include <boost/http_proto/file_win32.hpp>
#include <boost/http_proto/file_stdio.hpp>
#include <boost/http_proto/filter.hpp>
#include <boost/http_proto/header_limits.hpp>
#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/message_view_base.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/serializer.hpp>
#include <boost/http_proto/sink.hpp>
#include <boost/http_proto/source.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_body.hpp>
#include <boost/http_proto/version.hpp>

#include <boost/http_proto/rfc/combine_field_values.hpp>
#include <boost/http_proto/rfc/list_rule.hpp>
#include <boost/http_proto/rfc/parameter.hpp>
#include <boost/http_proto/rfc/quoted_token_rule.hpp>
#include <boost/http_proto/rfc/quoted_token_view.hpp>
#include <boost/http_proto/rfc/token_rule.hpp>
#include <boost/http_proto/rfc/upgrade_rule.hpp>

#include <boost/http_proto/service/service.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

#endif

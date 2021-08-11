//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_HPP
#define BOOST_HTTP_PROTO_HPP

#include <boost/http_proto/basic_parser.hpp>
#include <boost/http_proto/connection.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/decoder.hpp>
#include <boost/http_proto/deflate_service.hpp>
#include <boost/http_proto/encoder.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/rfc.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/trivial_optional.hpp>

#include <boost/http_proto/bnf/any_list.hpp>
#include <boost/http_proto/bnf/char_set.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/bnf/header_fields.hpp>
#include <boost/http_proto/bnf/number.hpp>
#include <boost/http_proto/bnf/optional_list.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/bnf/request_line.hpp>
#include <boost/http_proto/bnf/required_list.hpp>
#include <boost/http_proto/bnf/token_list.hpp>
#include <boost/http_proto/bnf/transfer_encoding_list.hpp>
#include <boost/http_proto/bnf/transfer_param_list.hpp>

#endif

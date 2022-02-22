//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_TEST_HELPERS_HPP
#define BOOST_HTTP_PROTO_TEST_HELPERS_HPP

#include <boost/http_proto/string_view.hpp>
#include "test_suite.hpp"
#include <string>

namespace boost {
namespace http_proto {

class fields;
class fields_view;
class fields_view_base;

class request;
class request_view;

fields_view
make_fields(
    string_view s);

// with table
fields_view
make_fields(
    string_view s,
    std::string& buf);

request_view
make_request(
    string_view s);

// with table
/*
request_view
make_request(
    string_view s,
    std::string& buf);
*/

void
check(
    fields_view_base const& f,
    std::size_t n,
    string_view m);

void
check(
    request_view const& req,
    std::size_t n,
    string_view m);

void
check(
    request const& req,
    std::size_t n,
    string_view m);

} // http_proto
} // boost

#endif

//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/parser.hpp>

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/request_parser.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

struct socket {};
template<class Stream>
void read_header(Stream&, parser&) {}
template<class Stream>
void read_body(Stream&, parser&) {}
struct string_body { string_body(std::string&) {} };
struct value {};
struct json_body { json_body(value&) {} };

class parser_test
{
public:
    void
    run()
    {
        {
            // read body into string
            socket sock;
            context ctx;
            request_parser p(ctx);
            read_header(sock, p);
            std::string s;
            p.attach_body(string_body(s));
            read_body(sock, p);
        }
        {
            // read body into json
            socket sock;
            context ctx;
            request_parser p(ctx);
            read_header(sock, p);
            value v;
            p.attach_body(json_body(v));
            read_body(sock, p);
        }
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost

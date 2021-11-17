//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
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
/*
    parser::parse_header()
        Postcondition: header is fully parsed

        * When the postcondition is met, the error
          indicates success

        * If the postcondition is not met when the
          call returns, the error indicates failure

          - failure can be terminal (i.e. syntax)
          - or failure is recoverable (need_more)

        Effects:
            Buffered octets are parsed as message
            header until a complete header is
            parsed or a terminal error occurs.

            If the semantics of the header indicate
            that there is no payload, has_body()
            will return false, and subsequent
            parsing calls will return immediately
            with error::end_of_message

    parser::parse_body()
        Postcondition: body is fully parsed

        * When the postcondition is met, the error
          indicates success

        * If the postcondition is not met when the
          call returns, the error indicates failure

          - failure can be terminal (i.e. syntax)
          - or failure is recoverable (need_more)

        Effects:
            Buffered octets are parsed as message
            body until the entire body is parsed
            (which may require seeing the end of
            stream), or a terminal error occurs.

            Any chunked transfer-encoding is removed
            first. Chunk extensions or trailers are
            discarded.

            If a decoder is selected, body octets
            are passed through the decoder, after
            any de-chunking, and stored in the
            buffer.

            If a body sink is selected, dechunked and
            decoded body octets are passed to the
            body sink.

    parser::parse_body_part()
        Postcondition: additional body octets have been parsed

        * When the postcondition is met, the error
          indicates success

        * If the postcondition is not met when the
          call returns, the error indicates failure

          - failure can be terminal, for example a
            syntax error or end_of_message,
          - or failure is recoverable (need_more)

        Effects:
            Buffered octets are parsed as message
            body until either the entire buffered
            input is consumed, the entire body is
            parsed (which may require seeing the
            end of stream), or a terminal error
            occurs.

            Any chunked transfer-encoding is removed
            first. Chunk extensions or trailers are
            discarded.

            If a decoder is selected, body octets
            are passed through the decoder, after
            any de-chunking, and stored in the
            buffer.

            If a body sink is selected, dechunked and
            decoded body octets are passed to the
            body sink.
*/
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

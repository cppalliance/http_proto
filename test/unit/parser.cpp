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

#include <boost/http_proto/request_parser.hpp>
#include <boost/buffers/buffer_copy.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

/*

Body Styles
-----------

1 A Sink
    parser calls the sink zero or more
    times with a buffer containing body data

2 A DynaBuffer
    parser writes body data directly into
    the dynamic buffer

3 Read fully into parser
    parser reads the entire body into its
    internal buffer. if the body doesn't fit
    an error is returned.

4 Read a buffer at a time
    parser reads the next body chunk into its
    internal buffer, clearing the previous
    body chunk first.

parser pr;
pr.reset();                     // new connection, no preconditions

response_parser pr;
pr.start( head_response );      // response parser only

parser pr;                      // request or response
pr.start( headers_first );      // error::headers_first on headers
pr.start();                     

pr.body_into( Sink&& );         // receive body into Sink
pr.body_into( DynaBuffer&& );   // receive body into DynBuffer
pr.body_into( &part );          // return string_view to body

// A lambda to defer body choice
pr.reset(
    []( request_parser&,
        request_view const& req,
        set_body_fn const& set_body )
    {
        if( req.has_payload() )
            set_body( string_sink() );
    });

// reading a HEAD response
pr.reset( no_payload );
async_read( sock, pr );

// reading headers first
pr.reset( headers_first );
auto ec = co_await async_read( sock, pr );
assert( ec == error::got_headers );
if(! pr.is_done())
    ec = co_await async_read( sock, pr );

need to get trailers
*/
struct parser_test
{
    static
    void
    feed(
        parser& pr,
        string_view& s,
        std::size_t n,
        error_code& ec)
    {
        auto dest = pr.prepare();
        if( n > s.size())
            n = s.size();
        auto const n1 = buffers::buffer_copy(
            dest, buffers::const_buffer(
                s.data(), n));
        BOOST_TEST_EQ(n1, n);
        pr.commit(n1);
        s = s.substr(n);
        pr.parse(ec);
    }

    void
    testParse()
    {
        auto const check =
        [](string_view const s0)
        {
            request_parser pr;
            for(std::size_t i = 1;
                i < s0.size() - 1; ++i)
            {
                auto s = s0;
                pr.start();
                for(;;)
                {
                    error_code ec;
                    feed(pr, s, i, ec);
                    if(ec == grammar::error::need_more)
                        continue;
                    if(! BOOST_TEST(! ec.failed()))
                    {
                        pr.reset();
                        break;
                    }
                    BOOST_TEST(pr.got_header());
                    BOOST_TEST_EQ(
                        pr.get().buffer(), s0);
                    break;
                }
            }
        };

        check(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "\r\n");
    }

    void
    run()
    {
        testParse();
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost

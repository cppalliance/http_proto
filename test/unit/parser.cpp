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
class parser_test
{
public:
    void
    run()
    {
/*
    parser p;

    read_some( s, p );
    switch( p.where() )
    {
    case parser::got_nothing:
        // parser has 0 bytes of input.
        // if the connection is closed now,
        // then it is a graceful closure.

    case parser::got_something:
        // parser has 1 or more bytes of
        // input, but no complete header.
        // if the connection is closed now,
        // then the stream was truncated.

    case parser::got_header:
        // parser got a complete header,
        // and 1 or more body bytes are
        // expected (or the end of file).
        // if no body bytes are expected,
        // this will be got_message instead.

    case parser::got_body_part:
        // parser got 1 or more bytes of
        // body data

    case parser::got_chunk:
        // parser got a chunked header.
        // the chunk size and extensions
        // are available now.

    case parser::got_chunk_final:
        // parser got a final chunk.
        // the trailer is availabe now.

    case parser::got_message:
        // the message is complete.
    }

*/
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost

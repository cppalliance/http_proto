//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/request_parser.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

/*
    Three body styles for `serializer`
        * Specify a ConstBuffers
        * Specify a Source
        * Write into a serializer::stream

    Three body styles for `parser`
        * Specify a DynamicBuffer
        * Specify a Sink
        * Read from a parser::stream

--------------------------------------------------

    Reading directly into a buffered body

    auto n = read_size(h_, body);
    dest = body.prepare(n); // VFALCO Need a type-erasing here
    auto nr = read( sock, dest );
    body.commit(nr);



    Transferring ownership to serializer/parser:
        std::string s = ...;
        sr.reset( req, make_buffers( s ) ); // reference to `s`
    versus
        std::string s = ...;
        sr.reset( req, make_buffers( sr.store(std::move(s)) ) ); // ownership transferred
*/

struct sandbox_test
{
#if 0
    template<class T> static void read_headers(T, parser&) { }
    template<class T> static void read_body(T, parser&) { }
    static int const sock = 0;

    void
    sandbox()
    {
        request_parser pr;
        pr.start();
        read_headers( sock, pr );
        request_view req = pr.get();
        if(req.method() == method::post)
        {
            string_sink& ss = pr.set_body( string_sink{} );
            read_body( sock, pr );
            std::string const& s = ss.get();
            std::count << s;
        }
    }
#endif
    void
    run()
    {
    }
};

TEST_SUITE(
    sandbox_test,
    "boost.http_proto.sandbox");

} // http_proto
} // boost

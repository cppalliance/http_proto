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

#if 0

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/bnf/chunk_ext.hpp>
#include <boost/http_proto/bnf/header_fields.hpp>
#include <boost/http_proto/bnf/range.hpp>
#include <iostream>

namespace boost {
namespace http_proto {

struct socket {};

void read_more(socket&, basic_parser&) {}

void f1()
{
    error_code ec;
    context ctx;
    request_parser p(ctx);
    socket s; {

    // Read complete body
    for(;;)
    {
        p.parse_body( ec );
        if( ec == error::need_more )
        {
            read_more( s, p );
            continue;
        }
        if( ec == error::end_of_message )
            break;
        if( ec )
            throw ec;
    }
    std::cout << p.payload();

    // Ready body incrementally
    for(;;)
    {
        p.parse_body( ec );
        if( ec == error::need_more )
        {
            read_more( s, p );
            continue;
        }
        if( ec == error::end_of_message )
            break;
        if( ec )
            throw ec;
        std::cout << p.payload();
        p.discard_payload();
    }

    // Read chunked body incrementally
    for(;;)
    {
        auto const ci = p.parse_chunk(ec);
        if( ec == error::need_more )
        {
            read_more(s, p);
            continue;
        }
        if( ec == error::end_of_message )
        {
            for(auto it : p.trailer())
                std::cout << it->name << ": " << it->value << "\n";
            return;
        }
        if( ec == error::end_of_chunk )
        {
            for( auto it : p.chunk_ext())
                std::cout << it->name << "=" << it->value << "\n";
            ec = {};
        }
        if( ec )
            throw ec;
        std::cout << p.payload();
        p.discard_payload();
    }

}}

} // http_proto
} // boost
#endif

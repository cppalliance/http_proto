//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/beast2/http/request_parser.hpp>

#include "test_suite.hpp"

namespace boost {
namespace beast2 {
namespace http {

class request_parser_test
{
public:
    static
    void
    do_server()
    {
    }

    void
    run()
    {
/*
        error_code ec;
        request_parser rp;

        while( ! rp.is_header_done() )
        {
            if( rp.need_more() )
            {
                auto const bytes_transferred =
                    sock.read_some( rp.buffer(), ec );
                if( ec == net::error::eof )
                    rp.commit_eof( ec ); // clears ec
                if( ec )
                    return;
                rp.commit( bytes_transferred );
            }
            rp.consume( ec );
            if( ec )
                return;
        }

        // inspect header
        f( rp.header() );

        while( ! rp.is_body_done() )
        {
            if( rp.need_more() )
            {
                auto const bytes_transferred =
                    sock.read_some( rp.buffer(), ec );
                rp.commit( bytes_transferred );
                if( ec == net::error::eof )
                    rp.commit_eof( ec ); // clears ec
                if( ec )
                    return;
            }
            rp.consume( ec );
            if( ec )
                return;
        }
*/
    }
};

TEST_SUITE(request_parser_test, "boost.beast2.http.request_parser");

} // http
} // beast2
} // boost


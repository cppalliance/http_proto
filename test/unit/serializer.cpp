//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/serializer.hpp>

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/request.hpp>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

/*
    chunked-body   = *chunk
                    last-chunk
                    trailer-part
                    CRLF

    chunk          = chunk-size [ chunk-ext ] CRLF
                    chunk-data CRLF
    chunk-size     = 1*HEXDIG
    last-chunk     = 1*("0") [ chunk-ext ] CRLF

    chunk-data     = 1*OCTET ; a sequence of chunk-size octets

    trailer-part   = *( header-field CRLF )

    ----------------------------------------------

    Design Notes

    serializing a file
        read into a buffer

    zlib inflate on serialize
        process input and produce output

    chunk prefix
        000 [ ; <ext> ] CRLF
        <data> CRLF
*/

//------------------------------------------------

class serializer_test
{
public:
    void
    run()
    {
    }
};

TEST_SUITE(
    serializer_test,
    "boost.http_proto.serializer");

} // http_proto
} // boost

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

    Attach body to the serializer:

        1. As a ConstBufferSequence
        2. As a ConstBufferSequence with ownership transfer
            This is the same as 1 if we take ownership of
            the sequence (which can, e.g. hold shared_ptr)
        3. As a incremental Source
            needs to be given a buffer
        4. As an incremental buffered Source
            comes with its own buffer

    sr.set_body( string_body( s ) );
    sr.set_body( string_body( std::move(s) ) );
    sr.set_body( file_body( fi ) );
    sr.set_body( json_body( jv ) );
*/

//------------------------------------------------

struct source_
{
    struct params
    {
        /** The number of bytes fetched
        */
        std::size_t amount = 0;

        /** True if there is more data
        */
        bool more = false;
    };

    virtual ~source_() = 0;

    /** Return the next buffer of data.
    */
    virtual
    result<params>
    fetch(
        void* dest,
        std::size_t size) = 0;
};

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

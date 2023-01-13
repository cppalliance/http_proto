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

#include <boost/http_proto/response.hpp>

#include "test_suite.hpp"

#include <string>

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

struct serializer_test
{
    static
    std::string
    read_some(serializer& sr)
    {
        std::string s;
        auto cbs = sr.prepare().value();
        for(auto const& cb : cbs)
            s.append(
                reinterpret_cast<
                    char const*>(cb.data()),
                cb.size());
        sr.consume(s.size());
        return s;
    }

    static
    std::string
    read(serializer& sr)
    {
        std::string s;
        while(! sr.is_done())
            s += read_some(sr);
        return s;
    }

    template<class T>
    static
    T const&
    make_const(T&& t) noexcept
    {
        return t;
    }

    void
    testSyntax()
    {
        struct test_body : source
        {
            results
            read(
                mutable_buffers_pair) override
            {
                return {};
            }
        };

        serializer sr(1024);
        response res;

        sr.reset(res);
        sr.reset(res, const_buffer{});
        sr.reset(res, mutable_buffer{});
        sr.reset(res, const_buffers_1{});
        sr.reset(res, mutable_buffers_1{});
        sr.reset(res, test_body{});
        sr.reset(res, make_const(const_buffer{}));
        sr.reset(res, make_const(mutable_buffer{}));
        sr.reset(res, make_const(const_buffers_1{}));
        sr.reset(res, make_const(mutable_buffers_1{}));
        sr.reset(res, make_const(test_body{}));


    }

    void
    testOutput()
    {
        response res;

        {
            serializer sr(1024);
            sr.reset(res);
            auto s = read(sr);
        }
    }

    void
    run()
    {
        testSyntax();
        testOutput();
    }
};

TEST_SUITE(
    serializer_test,
    "boost.http_proto.serializer");

} // http_proto
} // boost

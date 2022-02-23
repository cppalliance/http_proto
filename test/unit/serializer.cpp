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

class serializer_test
{
public:
    template<class Buffer>
    std::size_t
    write(Buffer const&, error_code&)
    {
        return {};
    }

    void
    testPrototypes()
    {
        // send GET request, empty body
        {
            error_code ec;
            request req;
            serializer_ sr;
            sr.reset(req);
            while(! sr.is_complete())
            {
                auto b = sr.prepare(ec);
                if(ec.failed())
                    return;
                auto bytes_transferred =
                    write(b, ec);
                if(ec.failed())
                    return;
                sr.consume(bytes_transferred);
            }
        }

        // send POST request, string body
        {
            error_code ec;
            request req;
            req.set_method( method::post );
            std::string body = "These seeds.";
            req.emplace_back(
                field::content_length,
                "12");
            serializer_ sr;
            sr.reset(req);
            sr.attach_body(body);
            while(! sr.is_complete())
            {
                auto b = sr.prepare(ec);
                if(ec.failed())
                    return;
                auto bytes_transferred =
                    write(b, ec);
                if(ec.failed())
                    return;
                sr.consume(bytes_transferred);
            }
        }
    }

    void
    run()
    {
        //testPrototypes();

        context ctx;
        serializer sr(ctx);
    }
};

TEST_SUITE(
    serializer_test,
    "boost.http_proto.serializer");

} // http_proto
} // boost

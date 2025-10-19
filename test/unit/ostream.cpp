//
// Copyright (c) 2025 GitHub Copilot
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_base.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/response_base.hpp>

#include "test_suite.hpp"

#include <sstream>

namespace boost {
namespace http_proto {

struct ostream_test
{
    void
    testFields()
    {
        // Test fields
        {
            fields f;
            f.set(field::host, "example.com");
            f.set(field::content_type, "text/html");
            
            std::stringstream ss;
            ss << f;
            
            std::string result = ss.str();
            
            // Should not contain CRLF
            BOOST_TEST(result.find("\r\n") == std::string::npos);
            
            // Should contain LF
            BOOST_TEST(result.find("\n") != std::string::npos);
            
            // Should not end with newline (no trailing CRLF)
            BOOST_TEST(!result.empty() && result.back() != '\n');
            
            // Should contain the field names and values
            BOOST_TEST(result.find("Host: example.com") != std::string::npos);
            BOOST_TEST(result.find("Content-Type: text/html") != std::string::npos);
        }
        
        // Test fields_base
        {
            fields f;
            f.set(field::server, "test-server");
            
            std::stringstream ss;
            ss << static_cast<const fields_base&>(f);
            
            std::string result = ss.str();
            
            // Should not contain CRLF
            BOOST_TEST(result.find("\r\n") == std::string::npos);
            
            // Should contain LF
            BOOST_TEST(result.find("\n") != std::string::npos);
            
            // Should not end with newline
            BOOST_TEST(!result.empty() && result.back() != '\n');
        }
    }
    
    void
    testRequest()
    {
        // Test request
        {
            request req(method::get, "/");
            req.set(field::host, "example.com");
            req.set(field::user_agent, "test-agent");
            
            std::stringstream ss;
            ss << req;
            
            std::string result = ss.str();
            
            // Should not contain CRLF
            BOOST_TEST(result.find("\r\n") == std::string::npos);
            
            // Should contain LF
            BOOST_TEST(result.find("\n") != std::string::npos);
            
            // Should not end with newline
            BOOST_TEST(!result.empty() && result.back() != '\n');
            
            // Should start with request line
            BOOST_TEST(result.find("GET / HTTP/1.1") == 0);
            
            // Should contain headers
            BOOST_TEST(result.find("Host: example.com") != std::string::npos);
            BOOST_TEST(result.find("User-Agent: test-agent") != std::string::npos);
        }
        
        // Test request_base
        {
            request req(method::post, "/data");
            
            std::stringstream ss;
            ss << static_cast<const request_base&>(req);
            
            std::string result = ss.str();
            
            // Should not contain CRLF
            BOOST_TEST(result.find("\r\n") == std::string::npos);
            
            // Should start with request line
            BOOST_TEST(result.find("POST /data HTTP/1.1") == 0);
        }
    }
    
    void
    testResponse()
    {
        // Test response
        {
            response res(status::ok);
            res.set(field::server, "test-server");
            res.set(field::content_type, "text/plain");
            
            std::stringstream ss;
            ss << res;
            
            std::string result = ss.str();
            
            // Should not contain CRLF
            BOOST_TEST(result.find("\r\n") == std::string::npos);
            
            // Should contain LF
            BOOST_TEST(result.find("\n") != std::string::npos);
            
            // Should not end with newline
            BOOST_TEST(!result.empty() && result.back() != '\n');
            
            // Should start with status line
            BOOST_TEST(result.find("HTTP/1.1 200 OK") == 0);
            
            // Should contain headers
            BOOST_TEST(result.find("Server: test-server") != std::string::npos);
            BOOST_TEST(result.find("Content-Type: text/plain") != std::string::npos);
        }
        
        // Test response_base
        {
            response res(status::not_found);
            
            std::stringstream ss;
            ss << static_cast<const response_base&>(res);
            
            std::string result = ss.str();
            
            // Should not contain CRLF
            BOOST_TEST(result.find("\r\n") == std::string::npos);
            
            // Should start with status line
            BOOST_TEST(result.find("HTTP/1.1 404 Not Found") == 0);
        }
    }
    
    void
    testEmptyMessages()
    {
        // Test empty fields
        {
            fields f;
            
            std::stringstream ss;
            ss << f;
            
            std::string result = ss.str();
            
            // Empty fields should produce empty output (no trailing CRLF)
            BOOST_TEST(result.empty());
        }
    }
    
    void
    run()
    {
        testFields();
        testRequest();
        testResponse();
        testEmptyMessages();
    }
};

TEST_SUITE(
    ostream_test,
    "boost.http_proto.ostream"
);

} // http_proto
} // boost

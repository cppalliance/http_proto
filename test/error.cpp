//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/beast2/http/error.hpp>

#include <memory>

#include "test_suite.hpp"

namespace boost {
namespace beast2 {
namespace http {

class error_test
{
public:
    void
    check(char const* name, error ev)
    {
        auto const ec = make_error_code(ev);
        auto const& cat = make_error_code(
            static_cast<http::error>(0)).category();
        BOOST_TEST(std::string(ec.category().name()) == name);
        BOOST_TEST(! ec.message().empty());
        BOOST_TEST(
            std::addressof(ec.category()) == std::addressof(cat));
        BOOST_TEST(cat.equivalent(
            static_cast<std::underlying_type<error>::type>(ev),
                ec.category().default_error_condition(
                    static_cast<std::underlying_type<error>::type>(ev))));
        BOOST_TEST(cat.equivalent(ec,
            static_cast<std::underlying_type<error>::type>(ev)));
    }

    void
    run()
    {
        check("beast2.http", error::end_of_stream);
        check("beast2.http", error::partial_message);
        check("beast2.http", error::need_more);
        check("beast2.http", error::unexpected_body);
        check("beast2.http", error::need_buffer);
        check("beast2.http", error::end_of_chunk);
        check("beast2.http", error::buffer_overflow);
        check("beast2.http", error::header_limit);
        check("beast2.http", error::body_limit);
        check("beast2.http", error::bad_alloc);

        check("beast2.http", error::bad_line_ending);
        check("beast2.http", error::bad_method);
        check("beast2.http", error::bad_target);
        check("beast2.http", error::bad_version);
        check("beast2.http", error::bad_status);
        check("beast2.http", error::bad_reason);
        check("beast2.http", error::bad_field);
        check("beast2.http", error::bad_value);
        check("beast2.http", error::bad_content_length);
        check("beast2.http", error::bad_transfer_encoding);
        check("beast2.http", error::bad_chunk);
        check("beast2.http", error::bad_chunk_extension);
        check("beast2.http", error::bad_obs_fold);

        check("beast2.http", error::stale_parser);
        check("beast2.http", error::short_read);
    }
};

TEST_SUITE(error_test, "boost.beast2.http.error");

} // http
} // beast2
} // boost

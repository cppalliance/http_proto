//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/error.hpp>
#include <boost/url/grammar/error.hpp>

#include <memory>

#include "test_suite.hpp"

namespace boost {
namespace http_proto {

class error_test
{
public:
    void
    check(
        char const* name,
        error ev)
    {
        auto const ec = make_error_code(ev);
        auto const& cat = make_error_code(
            static_cast<http_proto::error>(0)).category();
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
        char const* const n = "boost.http.proto";

        check(n, error::expect_100_continue);
        check(n, error::end_of_message);
        check(n, error::end_of_stream);

        check(n, error::bad_connection);
        check(n, error::bad_content_length);
        check(n, error::bad_expect);
        check(n, error::bad_field_name);
        check(n, error::bad_field_value);
        check(n, error::bad_line_ending);
        check(n, error::bad_list);
        check(n, error::bad_method);
        check(n, error::bad_number);
        check(n, error::bad_version);
        check(n, error::bad_reason);
        check(n, error::bad_request_target);
        check(n, error::bad_status_code);
        check(n, error::bad_status_line);
        check(n, error::bad_transfer_encoding);
        check(n, error::bad_upgrade);
        check(n, error::syntax);

        check(n, error::body_too_large);
        check(n, error::field_too_large);
        check(n, error::header_too_large);
        check(n, error::too_many_fields);
        check(n, error::numeric_overflow);

        check(n, error::multiple_content_length);
    }
};

TEST_SUITE(error_test, "boost.http_proto.error");

} // http_proto
} // boost

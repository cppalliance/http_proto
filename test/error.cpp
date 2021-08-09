//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/error.hpp>

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
        error ev,
        condition c)
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
        BOOST_TEST(ec == c);
    }

    void
    run()
    {
        char const* const n = "boost.http_proto";

        condition c;

        c = condition::partial_success;
        check(n, error::need_more, c);

        c = condition::syntax_error;
        check(n, error::bad_method, c);
        check(n, error::bad_field_name, c);
        check(n, error::bad_field_value, c);
        check(n, error::bad_line_ending, c);
        check(n, error::bad_list, c);
        check(n, error::bad_version, c);
        check(n, error::bad_request_target, c);
        check(n, error::bad_transfer_encoding, c);
    }
};

TEST_SUITE(error_test, "boost.http_proto.error");

} // http_proto
} // boost

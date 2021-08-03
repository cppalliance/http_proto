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
    check(char const* name, error ev)
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
        check("http_proto", error::end_of_stream);
        check("http_proto", error::partial_message);
        check("http_proto", error::need_more);
        check("http_proto", error::unexpected_body);
        check("http_proto", error::need_buffer);
        check("http_proto", error::end_of_chunk);
        check("http_proto", error::buffer_overflow);
        check("http_proto", error::header_limit);
        check("http_proto", error::body_limit);

        check("http_proto", error::bad_line_ending);
        check("http_proto", error::bad_method);
        check("http_proto", error::bad_target);
        check("http_proto", error::bad_version);
        check("http_proto", error::bad_status);
        check("http_proto", error::bad_reason);
        check("http_proto", error::bad_field);
        check("http_proto", error::bad_value);
        check("http_proto", error::bad_content_length);
        check("http_proto", error::bad_transfer_encoding);
        check("http_proto", error::bad_chunk);
        check("http_proto", error::bad_chunk_extension);
        check("http_proto", error::bad_obs_fold);

        check("http_proto", error::short_read);
    }
};

TEST_SUITE(error_test, "boost.http_proto.error");

} // http_proto
} // boost

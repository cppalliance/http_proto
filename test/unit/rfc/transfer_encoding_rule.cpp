//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include "../../src/rfc/transfer_encoding_rule.hpp"

#include <boost/static_assert.hpp>
#include <type_traits>

#include "test_helpers.hpp"

namespace boost {
namespace http_proto {

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_assignable<
        system::result<detail::transfer_encoding>>::value);

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_assignable<
        grammar::range<detail::transfer_encoding::param>>::value);

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_assignable<
        detail::transfer_encoding::param>::value);

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_assignable<
        quoted_token_view>::value);

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_constructible<
        system::result<detail::transfer_encoding>>::value);

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_constructible<
        grammar::range<detail::transfer_encoding::param>>::value);

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_constructible<
        detail::transfer_encoding::param>::value);

BOOST_STATIC_ASSERT(
    std::is_nothrow_copy_constructible<
        quoted_token_view>::value);

struct transfer_encoding_rule_test
{
    void
    run()
    {
        auto const& t =
            detail::transfer_encoding_rule;

        bad(t, "");
        bad(t, " ");
        bad(t, " x");
        bad(t, "x ");
        bad(t, " x ");
        ok(t,  "chunked");
        ok(t,  "compress");
        ok(t,  "deflate");
        ok(t,  "gzip");
        ok(t,  "wakanda;status=good");
        ok(t,  "wakanda;x=1;y=2");
        ok(t,  "main;dir=\"Program\\ Files\"");
        ok(t,  "main;dir1=\"Program\\ Files\";dir2=master");
        ok(t,  "gzip,chunked");
        ok(t,  "gzip, chunked");
        ok(t,  "br;level=5,chunked");
        ok(t,  "br;level=5, chunked");
        ok(t,  "br;level=5, ,chunked");
        ok(t,  "br;level=5, ,chunked,");
        ok(t,  "br;level=5, ,chunked, ,");
        bad(t, "br;level=5, ,chunked, ");

        ok(t, "chunked");
        ok(t, "compress");
        ok(t, "deflate");
        ok(t, "gzip");

        ok(t, "Chunked");
        ok(t, "Compress");
        ok(t, "Deflate");
        ok(t, "Gzip");

        bad(t, "chunked;p=2");
        bad(t, "compress;p=2");
        bad(t, "deflate;p=2");
        bad(t, "gzip;p=2");
    }
};

TEST_SUITE(
    transfer_encoding_rule_test,
    "boost.http_proto.transfer_encoding_rule");

} // http_proto
} // boost

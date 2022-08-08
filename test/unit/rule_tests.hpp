//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RULE_TESTS_HPP
#define BOOST_HTTP_PROTO_RULE_TESTS_HPP

#include "test_suite.hpp"
#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error_code.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/type_traits.hpp>
#include <type_traits>

namespace boost {
namespace http_proto {

// rule must match the string
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
ok( R const& r,
    string_view s)
{
    BOOST_TEST(grammar::parse(s, r).has_value());
}

// rule must match the string and value
template<class R, class V>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
ok( R const& r,
    string_view s,
    V const& v)
{
    auto rv = grammar::parse(s, r);
    if(BOOST_TEST(rv.has_value()))
        BOOST_TEST_EQ(rv.value(), v);
}

// rule must fail the string
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
bad(
    R const& r,
    string_view s)
{
    BOOST_TEST(grammar::parse(s, r).has_error());
}

// rule must fail the string with error
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
bad(
    R const& r,
    string_view s,
    error_code const& e)
{
    auto rv = grammar::parse(s, r);
    if(BOOST_TEST(rv.has_error()))
        BOOST_TEST_EQ(rv.error(), e);
}

} // http_proto
} // boost

#endif

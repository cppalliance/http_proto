//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_TEST_HELPERS_HPP
#define BOOST_HTTP_PROTO_TEST_HELPERS_HPP

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/core/detail/string_view.hpp>

#include "test_suite.hpp"

#include <iterator>
#include <string>

namespace boost {
namespace http_proto {

inline
std::string const&
test_pattern()
{
    static std::string const pat =
        "012" "34567" "89abcde";
    return pat;
}

template<class Buffers>
std::string
test_to_string(Buffers const& bs)
{
    std::string s(
        buffers::buffer_size(bs), 0);
    s.resize(buffers::buffer_copy(
        buffers::make_buffer(&s[0], s.size()),
        bs));
    return s;
}

//------------------------------------------------

// Test that fields equals HTTP string
void
test_fields(
    fields_view_base const& f,
    core::string_view match);

//------------------------------------------------

// rule must match the string
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
ok( R const& r,
    core::string_view s)
{
    BOOST_TEST(grammar::parse(s, r).has_value());
}

// rule must match the string and value
template<class R, class V>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
ok( R const& r,
    core::string_view s,
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
    core::string_view s)
{
    BOOST_TEST(grammar::parse(s, r).has_error());
}

// rule must fail the string with error
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
bad(
    R const& r,
    core::string_view s,
    system::error_code const& e)
{
    auto rv = grammar::parse(s, r);
    if(BOOST_TEST(rv.has_error()))
        BOOST_TEST_EQ(rv.error(), e);
}

} // http_proto
} // boost

#endif

//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_TEST_BNF_HPP
#define BOOST_HTTP_PROTO_TEST_BNF_HPP

#include <boost/http_proto/bnf/range.hpp>
#include <boost/http_proto/string_view.hpp>

#include "test_suite.hpp"

#include <sstream>

namespace boost {
namespace http_proto {
namespace test {

template<class T>
void
bad(string_view s)
{
    T r(s);
    BOOST_TEST(! r.is_valid());
    BOOST_TEST_THROWS(r.validate(),
        system_error);
}

template<class T>
void
suffix(
    string_view s,
    std::size_t n)
{
    auto s1 = valid_prefix<
        typename T::type>(s);
    BOOST_TEST(s.size() - s1.size() == n);
}

} // test
} // http_proto
} // boost

#endif

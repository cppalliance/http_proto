//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_NUMBER_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_NUMBER_IPP

#include <boost/http_proto/bnf/number.hpp>
#include <boost/http_proto/ctype.hpp>
#include <boost/http_proto/error.hpp>

namespace boost {
namespace http_proto {

char const*
number_bnf::
parse_element(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    if(start == end)
    {
        // no digits
        ec = error::bad_number;
        return start;
    }
    digit_set ds;
    std::uint64_t v = 0;
    auto const max = (static_cast<
        std::uint64_t>(-1));
    auto const max10 = max / 10;
    auto it = start;
    do
    {
        if(! ds.contains(*it))
        {
            if(it == start)
            {
                // no digits
                ec = error::bad_number;
                return start;
            }
            break;
        }
        if(v > max10)
        {
            ec = error::numeric_overflow;
            return start;
        }
        v *= 10;
        std::uint64_t const d =
            *it - '0';
        if(max - v < d)
        {
            ec = error::numeric_overflow;
            return start;
        }
        v += d;
    }
    while(++it != end);
    value = v;
    return it;
}

} // http_proto
} // boost

#endif

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
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/ctype.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
dec_number::
parse(
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
    auto const max = (static_cast<
        std::uint64_t>(-1));
    auto const max10 = max / 10;
    auto it = start;
    v_ = 0;
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
        if(v_ > max10)
        {
            ec = error::numeric_overflow;
            return start;
        }
        v_ *= 10;
        std::uint64_t const d =
            *it - '0';
        if(max - v_ < d)
        {
            ec = error::numeric_overflow;
            return start;
        }
        v_ += d;
    }
    while(++it != end);
    return it;
}

//------------------------------------------------

char const*
hex_number::
parse(
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
    hexdig_set hs;
    auto const max = (static_cast<
        std::uint64_t>(-1));
    auto const max16 = max / 16;
    auto it = start;
    v_ = 0;
    do
    {
        if(! hs.contains(*it))
        {
            if(it == start)
            {
                // no digits
                ec = error::bad_number;
                return start;
            }
            break;
        }
        if(v_ > max16)
        {
            ec = error::numeric_overflow;
            return start;
        }
        v_ *= 16;
        std::uint64_t const d =
            hs.value(*it);
        if(max - v_ < d)
        {
            ec = error::numeric_overflow;
            return start;
        }
        v_ += d;
    }
    while(++it != end);
    return it;
}

} // bnf
} // http_proto
} // boost

#endif

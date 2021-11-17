//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_NUMBER_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_NUMBER_IPP

#include <boost/http_proto/bnf/number.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/url/bnf/charset.hpp>

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
        ec = error::need_more;
        return start;
    }
    auto ds = urls::bnf::digit_chars;
    auto const max = (static_cast<
        std::uint64_t>(-1));
    auto const max10 = max / 10;
    auto it = start;
    v_ = 0;
    do
    {
        if(! ds(*it))
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
        ec = error::need_more;
        return start;
    }
    auto const hs = urls::bnf::hexdig_chars;
    auto const max = (static_cast<
        std::uint64_t>(-1));
    auto const max16 = max / 16;
    auto it = start;
    v_ = 0;
    do
    {
        if(! hs(*it))
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
            urls::bnf::hexdig_value(*it);
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

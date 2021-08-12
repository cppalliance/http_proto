//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_REPEAT_HPP
#define BOOST_HTTP_PROTO_BNF_IMPL_REPEAT_HPP

#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

template<
    class Element,
    std::size_t N,
    std::size_t M>
char const*
repeat<Element, N, M>::
begin(
    char const* start,
    char const* end,
    error_code& ec)
{
    n_ = 0;
    return increment(
        start, end, ec);
}

template<
    class Element,
    std::size_t N,
    std::size_t M>
char const*
repeat<Element, N, M>::
increment(
    char const* start,
    char const* end,
    error_code& ec)
{
    auto it = element_.parse(
        start, end, ec);
    if(! ec)
    {
        ++n_;
        if(n_ <= M)
            return it;
        // treat this as end
        ec = error::end;
        return start;
    }
    BOOST_ASSERT(
        ec != error::end);
    if(n_ >= N)
    {
        // treat as end
        ec = error::end;
        return it;
    }
    // too few elements
    ec = error::syntax;
    return it;
}

} // bnf
} // http_proto
} // boost

#endif

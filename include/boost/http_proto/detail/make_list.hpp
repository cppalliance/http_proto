//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_MAKE_LIST_HPP
#define BOOST_HTTP_PROTO_DETAIL_MAKE_LIST_HPP

#include <boost/http_proto/detail/header.hpp>
#include <boost/url/grammar/type_traits.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<class MutableString>
void
make_list(
    MutableString& dest,
    field id,
    std::size_t n, // # of matching fields
    header const& h)
{
    BOOST_ASSERT(n > 0);
    {
        // clear
        string_view s;
        dest.assign(
            s.begin(), s.end());
    }
    auto ft = h.tab();
    std::size_t i = 0;
    for(;;)
    {
        if(ft[i].id == id)
            break;
        ++i;
        BOOST_ASSERT(i < h.count);
    }
    {
        string_view s(
            h.cbuf + h.prefix +
                ft[i].vp,
            ft[i].vn);
        dest.assign(
            s.begin(), s.end());
        if(--n == 0)
            return;
    }
    for(;;)
    {
        ++i;
        BOOST_ASSERT(i < h.count);
        if(ft[i].id != id)
            continue;
        string_view s = ",";
        dest.append(
            s.begin(), s.end());
        s = {h.cbuf + h.prefix +
                ft[i].vp, ft[i].vn};
        dest.append(
            s.begin(), s.end());
        if(--n == 0)
            return;
    }
}

} // detail
} // http_proto
} // boost

#endif

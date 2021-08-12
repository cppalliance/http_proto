//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_DETAIL_IMPL_REPEAT_BASE_IPP
#define BOOST_HTTP_PROTO_BNF_DETAIL_IMPL_REPEAT_BASE_IPP

#include <boost/http_proto/bnf/detail/repeat_base.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace bnf {
namespace detail {

char const*
repeat_base::
begin(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    // [...]
    if(start == end)
    {
        ec = error::end;
        return end;
    }
    auto it = start;
    // ( "," / element )
    if(*it != ',')
    {
        it = parse(it, end, ec);
        if(ec)
        {
            // empty list
            ec = error::end;
        }
        return it;
    }
    ++it;
    return increment(it, end, ec);
}

char const*
optional_list_base::
increment(
    char const* start,
    char const* end,
    error_code& ec)
{
    ws_set ws;
    auto it = start;
    // *( OWS "," [ OWS element ] )
    for(;;)
    {
        it = ws.skip(it, end);
        if(it == end)
        {
            // empty list
            ec = error::end;
            return it;
        }
        if(*it != ',')
        {
            it = parse(it, end, ec);
            if(ec)
                ec = error::end;
            return it;
        }
        ++it;
    }
}

} // detail
} // bnf
} // http_proto
} // boost

#endif

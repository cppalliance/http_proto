//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_IMPL_QUOTED_STRING_IPP
#define BOOST_HTTP_PROTO_BNF_IMPL_QUOTED_STRING_IPP

#include <boost/http_proto/bnf/quoted_string.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <memory>

namespace boost {
namespace http_proto {
namespace bnf {

char const*
quoted_string::
parse(
    char const* const start,
    char const* const end,
    error_code& ec)
{
    if(start == end)
    {
        ec = error::syntax;
        return start;
    }
    qdtext_set qts;
    auto it = start;
    // DQUOTE
    if(*it != '\"')
    {
        ec = error::syntax;
        return start;
    }
    ++it;
    qpchar_set qps;
    for(;;)
    {
        if(it == end)
        {
            // missing DQUOTE
            ec = error::syntax;
            return start;
        }
        switch(*it)
        {
        case '"':
        {
            // DQUOTE
            ++it;
            v_ = string_view(
                start, it - start);
            return it;
        }
        case '\\':
        {
            // quoted-pair
            ++it;
            if(it == end)
            {
                ec = error::syntax;
                return start;
            }
            if(! qps.contains(*it))
            {
                // bad qpchar
                ec = error::syntax;
                return start;
            }
            break;
        }
        }
        ++it;
        it = qts.skip(it, end);
    }
}

std::string
unquote_text(
    char* const start,
    char const* end)
{
    std::string s;
    s.reserve(end - start);
    // return the same text
    // if it is not quoted
    if( s.size() < 2 ||
        s.front() != '\"' ||
        s.back() != '\"')
    {
        s.append(start,
            end - start);
        return s;
    }
    auto it = start + 1;
    end--;
    while(it != end)
    {
        if(*it != '\\')
        {
            s.push_back(*it);
        }
        else
        {
            ++it;
            if(it == end)
                break;
            s.push_back(*it);
        }
        ++it;
    }
    return s;
}

} // bnf
} // http_proto
} // boost

#endif

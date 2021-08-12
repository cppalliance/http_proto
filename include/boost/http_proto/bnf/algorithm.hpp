//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_ALGORITHM_HPP
#define BOOST_HTTP_PROTO_BNF_ALGORITHM_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/bnf/type_traits.hpp>
#include <type_traits>

namespace boost {
namespace http_proto {
namespace bnf {

/** Consume a matching element BNF.
*/
template<class Element>
#ifdef BOOST_HTTP_PROTO_DOCS
char const*
#else
typename std::enable_if<
    is_element<Element>::value,
    char const*>::type
#endif
consume(
    char const* start,
    char const* end,
    error_code& ec)
{
    Element e;
    auto it = e.parse(
        start, end, ec);
    if(ec.failed())
    {
        // end makes no sense
        BOOST_ASSERT(
            ec != error::end);
        return it;
    }
    return it;
}

//------------------------------------------------

/** Consume a matching list BNF
*/
template<class List>
#ifdef BOOST_HTTP_PROTO_DOCS
char const*
#else
typename std::enable_if<
    is_list<List>::value,
    char const*>::type
#endif
consume(
    char const* start,
    char const* end,
    error_code& ec)
{
    List e;
    auto it = e.begin(
        start, end, ec);
    for(;;)
    {
        if(ec == error::end)
        {
            ec = {};
            break;
        }
        if(ec.failed())
        {
            // treat as non-match
            return start;
        }
        it = e.increment(
            it, end, ec);
    }
    return it;
}

//------------------------------------------------

/** Return true if s matches the BNF exactly
*/
template<class BNF>
bool
is_valid(string_view s)
{
    error_code ec;
    auto const end =
        s.data() + s.size();
    auto it = consume<BNF>(
        s.data(), end, ec);
    if(ec.failed())
        return false;
    return it == end;
}

/** Throw an exception if s does not match the BNF exactly
*/
template<class BNF>
void
validate(string_view s)
{
    if(! is_valid<BNF>(s))
        http_proto::detail::throw_invalid_argument(
            "bad syntax", BOOST_CURRENT_LOCATION);
}

} // bnf
} // http_proto
} // boost

#endif

//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_LIST_HPP
#define BOOST_HTTP_PROTO_BNF_LIST_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/type_traits.hpp>
#include <boost/static_assert.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {
namespace bnf {

/** BNF for a comma-delimited list of elements

    This rule defines a list containing
    at least n and at most m of Element,
    each separated by a single comma and
    optional whitespace.

    @par BNF
    @code
    list        =  <n>#<m>element => element <n-1>*<m-1>( OWS "," OWS element )

    #element    => [ ( ("," OWS element) / element ) *( OWS "," [ OWS element ] ) ]

    1#element   => *( "," OWS ) element *( OWS "," [ OWS element ] )
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#section-7
        https://www.rfc-editor.org/errata/eid5257

    @tparam Element The element type to use in the list
    @tparam N The minimum number of list items, which may be zero
    @tparam M The maximum number of list items.
*/
template<
    class Element,
    std::size_t N = 0,
    std::size_t M = std::size_t(-1)>
class list
#ifdef BOOST_HTTP_PROTO_DOCS
    ;
#else
{
    BOOST_STATIC_ASSERT(M > 0);
    BOOST_STATIC_ASSERT(M >= N);
    BOOST_STATIC_ASSERT(
        is_element<Element>::value);

    Element element_;
    std::size_t n_;

public:
    using value_type =
        typename Element::value_type;

    value_type
    value() const noexcept
    {
        return element_.value();
    }

    char const*
    begin(
        char const* start,
        char const* end,
        error_code& ec);

    char const*
    increment(
        char const* start,
        char const* end,
        error_code& ec);
};
#endif

//------------------------------------------------

/** BNF for optional whitespace followed by a comma

    @par BNF
    @code
    ows-comma   = OWS ","
    @endcode
*/
#ifdef BOOST_HTTP_PROTO_DOCS
using ows_comma = __see_below__;
#else
struct ows_comma
{
    using value_type = void;

    value_type
    value() const noexcept
    {
    }

    inline
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec);
};
#endif

/** BNF for a comma followed by optional whitespace

    @par BNF
    @code
    comma-ows   = "," OWS
    @endcode
*/
#ifdef BOOST_HTTP_PROTO_DOCS
using comma_ows = __see_below__;
#else
struct comma_ows
{
    using value_type = void;

    value_type
    value() const noexcept
    {
    }

    inline
    char const*
    parse(
        char const* start,
        char const* end,
        error_code& ec);
};
#endif

//------------------------------------------------

/** A BNF rule for a comma-separated list of zero or more Element
*/
template<class Element>
using list_of_zero_or_more = list<Element>;

/** A BNF rule for a comma-separated list of one or more Element
*/
template<class Element>
using list_of_one_or_more = list<Element, 1>;

} // bnf
} // http_proto
} // boost

#include <boost/http_proto/bnf/impl/list.hpp>

#endif

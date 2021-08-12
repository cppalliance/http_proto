//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_REPEAT_HPP
#define BOOST_HTTP_PROTO_BNF_REPEAT_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/detail/repeat_base.hpp>
#include <boost/static_assert.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

#if 0
/** BNF for a list of N to M comma-separated elements

    This rule defines a list of elements
    separated by commas and optional whitespace.

    @par BNF
    @code
    #element        => [ ( "," / element ) *( OWS "," [ OWS element ] ) ]
    1#element       => *( "," OWS ) element *( OWS "," [ OWS element ] )
    <n>#<m>element  => element <n-1>*<m-1>( OWS "," OWS element )

    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-7
*/
#endif

/** BNF for a repetition of elements

    This rule defines a sequence containing
    at least n and at most m elements.

    @par BNF
    @code
    <n>*<m>element
    <n>element      => <n>*<n>element
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc5234
        https://datatracker.ietf.org/doc/html/rfc7230#section-7

    @tparam Element The element type to repeat
    @tparam N The minimum number of repetitions, which may be zero
    @tparam M The maximum number of repetitions.
*/
template<
    class Element,
    std::size_t N = 0,
    std::size_t M = std::size_t(-1)>
class repeat
{
    BOOST_STATIC_ASSERT(M >= N);

    Element element_;
    std::size_t n_;

public:
    using value_type =
        typename Element::value_type;

    value_type const&
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

//------------------------------------------------

template<
    class Element,
    std::size_t N,
    std::size_t M>
char const*
repeat::
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
repeat::
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
        if(n_ > M)
        {
            // too many, treat this
            // as unparsed input.
            it = start;
        }
    }
    else if(ec == error::end)
    {
        if(n_ >= N)
        {
            ec = {};
        }
        else
        {
            // too few elements
            ec = error::syntax;
        }
    }
    return it;
}

//------------------------------------------------

/** A BNF rule for zero or more repetitions of element
*/
template<class Element>
using zero_or_more = repeat<Element>;

/** A BNF rule for one or more repetitions of element
*/
template<class Element>
using one_or_more = repeat<Element, 1>;

} // bnf
} // http_proto
} // boost

#endif

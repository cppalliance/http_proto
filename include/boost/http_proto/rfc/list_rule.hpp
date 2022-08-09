//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_LIST_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_LIST_RULE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/rfc/detail/rules.hpp>
#include <boost/url/grammar/range_rule.hpp>
#include <boost/core/empty_value.hpp>

namespace boost {
namespace http_proto {

/** Rule for a comma-delimited list of elements

    This rule defines a list containing
    at least n and at most m of Element,
    each separated by at least one comma
    and optional whitespace.

    @par BNF
    @code
    #element       => [ 1#element ]
    1#element      => element *( OWS "," OWS element )
    <n>#<m>element => element <n-1>*<m-1>( OWS "," OWS element )
    @endcode

    Senders must emit compliant values, but
    receivers should accept values generated
    with the legacy production rules:

    @par Legacy BNF
    @code
    #element => [ element ] *( OWS "," OWS [ element ] )
    @endcode

    @tparam R The rule to use for elements
    @tparam N The minimum number of elements, which may be zero
    @tparam M The maximum number of elements.

    @par Specification
    @li <a href="https://datatracker.ietf.org/doc/html/rfc7230#section-7"
        >5.6.1. Lists (#rule ABNF Extension) (rfc7230)</a>
*/
#ifdef BOOST_HTTP_PROTO_DOCS
template<class Rule>
constexpr
__implementation_defined__
list_rule(
    Rule element,
    std::size_t N = 0,
    std::size_t M =
        std::size_t(-1)) noexcept;
#else
template<class Rule>
struct list_rule_t
    : private empty_value<Rule>
{
    using value_type = grammar::range<
        typename Rule::value_type>;

    constexpr
    list_rule_t(
        Rule const& r,
        std::size_t n,
        std::size_t m) noexcept
        : empty_value<Rule>(
            empty_init, r)
        , n_(n)
        , m_(m)
    {
    }

    auto
    parse(
        char const*& it,
        char const* end) const ->
            result<value_type>;

private:
    struct element_rule;

    std::size_t n_;
    std::size_t m_;
};

template<class Rule>
constexpr
auto
list_rule(
    Rule const& r,
    std::size_t N = 0,
    std::size_t M =
        std::size_t(-1)) noexcept ->
    list_rule_t<Rule>
{
    return list_rule_t<Rule>(r, N, M);
}
#endif

} // http_proto
} // boost

#include <boost/http_proto/rfc/impl/list_rule.hpp>

#endif

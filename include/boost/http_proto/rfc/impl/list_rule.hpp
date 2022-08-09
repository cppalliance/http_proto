//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_IMPL_LIST_RULE_HPP
#define BOOST_HTTP_PROTO_RFC_IMPL_LIST_RULE_HPP

#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/range_rule.hpp>

namespace boost {
namespace http_proto {

template<class Rule>
auto
list_rule_t<Rule>::
parse(
    char const*& it,
    char const* end) const ->
        result<value_type>
{
    return grammar::parse(it, end,
        grammar::range_rule(element_rule{
            this->get()}, n_, m_));
}

template<class Rule>
struct list_rule_t<Rule>::
    element_rule
{
    using value_type = string_view;

    Rule r;

    auto
    parse(
        char const*& it,
        char const* end) const ->
            result<typename Rule::value_type>
    {
        // *( OWS "," OWS )
        for(;;)
        {
            auto it0 = it;
            auto rv = grammar::parse(it, end,
                detail::ows_comma_ows_rule);
            if(! rv)
            {
                it = it0;
                break;
            }
        }
        // element
        auto it0 = it;
        auto rv = r.parse(it, end);
        if(! rv)
        {
            it = it0;
            return grammar::error::range_end;
        }
        // *( OWS "," OWS )
        for(;;)
            if(! grammar::parse(it, end,
                detail::ows_comma_ows_rule))
                break;
        return *rv;
    }
};

} // http_proto
} // boost

#endif

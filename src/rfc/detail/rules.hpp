//
// Copyright (c) 2024 Christian Mazakas
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SRC_RFC_DETAIL_RULES_HPP
#define BOOST_HTTP_PROTO_SRC_RFC_DETAIL_RULES_HPP

#include <boost/core/detail/string_view.hpp>
#include <boost/system/result.hpp>

namespace boost
{
namespace http_proto
{
namespace detail
{

// header-field   = field-name ":" OWS field-value OWS
struct field_name_rule_t
{
    using value_type = core::string_view;

    system::result<value_type>
    parse(
        char const*& it,
        char const* end) const noexcept;
};

constexpr field_name_rule_t field_name_rule{};

struct field_value_rule_t
{
    struct value_type
    {
        core::string_view value;
        // detected occurrence of `\r\n `, `\r\n\t`
        bool has_obs_fold = false;
        // detected `\r\nX`, attempt at field termination
        bool has_crlf = false;
    };

    system::result<value_type>
    parse(
        char const*& it,
        char const* end) const noexcept;
};

constexpr field_value_rule_t field_value_rule{};

} // namespace detail
} // namespace http_proto
} // namespace boost



#endif // BOOST_HTTP_PROTO_SRC_RFC_DETAIL_RULES_HPP

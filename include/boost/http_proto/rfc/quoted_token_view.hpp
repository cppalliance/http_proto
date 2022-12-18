//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_QUOTED_TOKEN_VIEW_HPP
#define BOOST_HTTP_PROTO_RFC_QUOTED_TOKEN_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/url/grammar/string_view_base.hpp>

namespace boost {
namespace http_proto {

class quoted_token_view
    : public grammar::string_view_base
{
    std::size_t n_ = 0;

    friend struct quoted_token_rule_t;

    // unquoted
    explicit
    quoted_token_view(
        string_view s) noexcept
        : string_view_base(s)
        , n_(s.size())
    {
    }

    // maybe quoted
    quoted_token_view(
        string_view s,
        std::size_t n) noexcept
        : string_view_base(s)
        , n_(n)
    {
        BOOST_ASSERT(s.size() >= 2);
        BOOST_ASSERT(s.front() == '\"');
        BOOST_ASSERT(s.back() == '\"');
        BOOST_ASSERT(n_ <= s_.size() - 2);
    }

public:
    quoted_token_view() = default;

    quoted_token_view(
        quoted_token_view const&) noexcept = default;

    quoted_token_view& operator=(
        quoted_token_view const&) noexcept = default;

    bool
    has_escapes() const noexcept
    {
        return n_ != s_.size();
    }

    std::size_t
    unescaped_size() const noexcept
    {
        return n_;
    }
};

} // http_proto
} // boost

#endif

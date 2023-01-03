//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_TEST_HELPERS_HPP
#define BOOST_HTTP_PROTO_TEST_HELPERS_HPP

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/url/grammar/parse.hpp>

#include "test_suite.hpp"

#include <iterator>
#include <string>

namespace boost {
namespace http_proto {

//------------------------------------------------

/** ForwardRange of fields used for testing
*/
class fields_range
{
    string_view s_;

public:
    struct value_type
    {
        string_view name;
        string_view value;

        value_type const*
        operator->() const noexcept
        {
            return this;
        }
    };

    class iterator;
    using const_iterator = iterator;

    fields_range(
        string_view s) noexcept
        : s_(s)
    {
    }

    iterator begin() const noexcept;
    iterator end() const noexcept;
};

//------------------------------------------------

class fields_range::iterator
{
public:
    using value_type =
        fields_range::value_type;
    using reference = value_type;
    using difference_type = std::ptrdiff_t;
    using iterator_category =
        std::forward_iterator_tag;
    using pointer = void;

    iterator() = default;
    iterator(iterator const&) = default;
    iterator& operator=(iterator const&) = default;

    bool
    operator==(
        iterator const& it) const noexcept
    {
        return
            s_.data() ==
                it.s_.data() &&
            s_.size() ==
                it.s_.size();
    }

    bool
    operator!=(
        iterator const& it) const noexcept
    {
        return !(*this == it);
    }

    iterator& operator++() noexcept;

    iterator
    operator++(int) noexcept
    {
        auto temp = *this;
        ++(*this);
        return temp;
    }

    value_type const
    operator->() const noexcept
    {
        return v_;
    }

    reference const
    operator*() const noexcept
    {
        return v_;
    }

private:
    string_view s_;
    value_type v_;

    friend class fields_range;

    explicit
    iterator(
        string_view s) noexcept
        : s_(s)
    {
        read();
    }

    void read() noexcept;
};

//------------------------------------------------

inline
auto
fields_range::
begin() const noexcept ->
    iterator
{
    return iterator(s_);
}

inline
auto
fields_range::
end() const noexcept ->
    iterator
{
    return iterator(s_.substr(
        s_.size() - 2));
}

//------------------------------------------------

// Test that fields equals HTTP string
void
test_fields(
    fields_view_base const& f,
    string_view match);

fields make_fields(string_view s);
request make_request(string_view s);
response make_response(string_view s);

request make_request_(string_view s);
response make_response(string_view s);

//------------------------------------------------

// rule must match the string
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
ok( R const& r,
    string_view s)
{
    BOOST_TEST(grammar::parse(s, r).has_value());
}

// rule must match the string and value
template<class R, class V>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
ok( R const& r,
    string_view s,
    V const& v)
{
    auto rv = grammar::parse(s, r);
    if(BOOST_TEST(rv.has_value()))
        BOOST_TEST_EQ(rv.value(), v);
}

// rule must fail the string
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
bad(
    R const& r,
    string_view s)
{
    BOOST_TEST(grammar::parse(s, r).has_error());
}

// rule must fail the string with error
template<class R>
typename std::enable_if<
    grammar::is_rule<R>::value>::type
bad(
    R const& r,
    string_view s,
    error_code const& e)
{
    auto rv = grammar::parse(s, r);
    if(BOOST_TEST(rv.has_error()))
        BOOST_TEST_EQ(rv.error(), e);
}

} // http_proto
} // boost

#endif

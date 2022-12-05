//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#include "test_helpers.hpp"

#include <boost/http_proto/fields.hpp>
#include <algorithm>

namespace boost {
namespace http_proto {

//------------------------------------------------

auto
fields_range::
iterator::
operator++() noexcept ->
    iterator&
{
    auto pos =
        s_.find_first_of('\r');
    s_.remove_prefix(pos + 2);
    read();
    return *this;
}

void
fields_range::
iterator::
read() noexcept
{
    auto p0 =
        s_.find_first_of(':');
    if(p0 != string_view::npos)
    {
        v_.name = s_.substr(
            0, p0);
        auto p1 = s_.find_first_of('\r');
        v_.value = s_.substr(
            p0 + 2, p1 - (p0 + 2));
    }
    else
    {
        v_ = {};
    }
}

//------------------------------------------------

void
test_fields(
    fields_view_base const& f,
    string_view match)
{
    fields_range r(match);
    std::size_t n = std::distance(
        f.begin(), f.end());
    BOOST_TEST_EQ(f.size(), n);
    auto it0 = r.begin();
    auto it1 = f.begin();
    auto const end = r.end();
    while(it0 != end)
    {
        if(! BOOST_TEST_NE(
                it1, f.end()))
            break;
        BOOST_TEST_EQ(
            it0->name, it1->name);
        BOOST_TEST_EQ(
            it0->value, it1->value);
        ++it0;
        ++it1;
    }
}

fields
make_fields(
    string_view s)
{
    fields f;
    for(auto v : fields_range(s))
        f.append(v.name, v.value);
    test_fields(f, s);
    return f;
}

request
make_request(
    string_view s)
{
    request req;
    for(auto v : fields_range(s))
        req.append(v.name, v.value);
    test_fields(req, s);
    return req;
}

request
make_request_(
    string_view s)
{
    request req;
    detail::header h(
        detail::kind::request);
    h.size = 0;
    h.prefix = 0;
    using detail::parse_start_line;
    s.remove_prefix(
        parse_start_line(h, s).value());
    req.set_start_line(
        string_view(
            h.cbuf, h.req.method_len),
        string_view(
            h.cbuf + h.req.method_len + 1,
            h.req.target_len),
        h.version);
    for(auto v : fields_range(s))
        req.append(v.name, v.value);
    test_fields(req, s);
    return req;
}

response
make_response_(
    string_view s)
{
    response res;
    detail::header h(
        detail::kind::response);
    h.size = 0;
    h.prefix = 0;
    using detail::parse_start_line;
    s.remove_prefix(
        parse_start_line(h, s).value());
    res.set_start_line(
        h.res.status_int,
        string_view(
            h.cbuf + 13,
            h.prefix - 15),
        h.version);
    for(auto v : fields_range(s))
        res.append(v.name, v.value);
    test_fields(res, s);
    return res;
}

response
make_response(
    string_view s)
{
    response res;
    for(auto v : fields_range(s))
        res.append(v.name, v.value);
    test_fields(res, s);
    return res;
}

} // http_proto
} // boost


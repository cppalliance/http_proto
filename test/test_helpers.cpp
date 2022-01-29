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
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/rfc/field_rule.hpp>
#include <boost/http_proto/rfc/request_line_rule.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/url/grammar/parse.hpp>

namespace boost {
namespace http_proto {

// construct without table
fields_view
make_fields(
    string_view s)
{
    struct helper : fields_view
    {
        explicit
        helper(string_view s)
            : fields_view(
            [&s]
            {
                ctor_params init;
                init.cbuf = s.data();
                init.buf_len = 0;
                init.start_len = 0;
                init.end_pos = s.size();
                init.count = 0;
                char const* it = s.data();
                char const* const end =
                    s.data() + s.size();
                error_code ec;
                field_rule r;
                for(;;)
                {
                    if(grammar::parse(
                        it, end, ec, r))
                    {
                        ++init.count;
                        continue;
                    }
                    if(ec == grammar::error::end)
                        break;
                    detail::throw_system_error(ec,
                        BOOST_CURRENT_LOCATION);
                }
                return init;
            }())
        {
        }
    };
    return helper(s);
}

//------------------------------------------------

// construct with table
fields_view
make_fields(
    string_view s,
    std::string& buf)
{
    fields_view f = make_fields(s);
    buf.resize(detail::buffer_needed(
        s.size(), f.size()));
    std::memcpy(&buf[0],
        s.data(), s.size());
    std::size_t i = 0;
    detail::fields_table ft(
        &buf[0] + buf.size());
    for(auto const& v : f)
    {
        auto& e = ft[i++];
        e.np = static_cast<off_t>(
            v.name.data() - s.data());
        e.nn = static_cast<off_t>(
            v.name.size());
        e.vp = static_cast<off_t>(
            v.value.data() - s.data());
        e.vn = static_cast<off_t>(
            v.value.size());
        e.id = v.id;
    }
    fields_view::ctor_params init;
    init.cbuf = buf.data();
    init.buf_len = buf.size();
    init.start_len = 0;
    init.end_pos = s.size();
    init.count = f.size();
    return fields_view(init);        
}

//------------------------------------------------

request_view
make_request(
    string_view s)
{
    struct helper : request_view
    {
        explicit
        helper(string_view s)
            : request_view(
            [&s]
            {
                error_code ec;
                auto it = s.data();
                auto const end =
                    it + s.size();
                request_line_rule t0;
                if(! grammar::parse(
                    it, end, ec, t0))
                    detail::throw_system_error(
                        ec, BOOST_CURRENT_LOCATION);

                ctor_params init;
                init.cbuf = s.data();
                init.buf_len = 0;
                init.start_len = static_cast<
                    off_t>(it - s.data());
                init.end_pos = s.size();
                init.count = 0;
                init.method_len = t0.ms.size();
                init.target_len = t0.t.size();
                init.method = t0.m;
                init.version = t0.v;
                field_rule t1;
                for(;;)
                {
                    if(grammar::parse(
                        it, end, ec, t1))
                    {
                        ++init.count;
                        continue;
                    }
                    if(ec == grammar::error::end)
                        break;
                    detail::throw_system_error(ec,
                        BOOST_CURRENT_LOCATION);
                }
                return init;
            }())
        {
        }
    };
    return helper(s);
}

//------------------------------------------------

void
check(
    fields_view const& f,
    std::size_t n,
    string_view m)
{
    std::string s;
    s.reserve(m.size());
    for(auto const& v : f)
    {
        s.append(v.name);
        s.append(": ", 2);
        s.append(v.value);
        s.append("\r\n", 2);
    }
    s.append("\r\n");
    BOOST_TEST(s == m);
    BOOST_TEST(f.size() == n);
    BOOST_TEST(f.buffer() == m);
}

//------------------------------------------------

void
check(
    request_view const& req,
    std::size_t n,
    string_view m)
{
    std::string s;
    s.reserve(m.size());
    s.append(req.method_str());
    s.push_back(' ');
    s.append(req.target());
    s.push_back(' ');
    s.append(to_string(req.version()));
    s.append("\r\n");
    for(auto const& v : req)
    {
        s.append(v.name);
        s.append(": ");
        s.append(v.value);
        s.append("\r\n");
    }
    s.append("\r\n");
    BOOST_TEST(s == m);
    BOOST_TEST(req.size() == n);
    BOOST_TEST(req.buffer() == m);
}

//------------------------------------------------

void
check(
    request const& req,
    std::size_t n,
    string_view m)
{
    check(static_cast<
        request_view const&>(
            req), n, m);
}

} // http_proto
} // boost


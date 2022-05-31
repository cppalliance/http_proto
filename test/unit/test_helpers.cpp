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
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/rfc/field_rule.hpp>
#include <boost/http_proto/rfc/request_line_rule.hpp>
#include <boost/http_proto/rfc/status_line_rule.hpp>
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
                detail::header h;
                h.kind = detail::kind::fields;
                h.cbuf = s.data();
                h.cap = 0;
                h.prefix = 0;
                h.size = static_cast<
                    off_t>(s.size());
                h.count = 0;
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
                        ++h.count;
                        continue;
                    }
                    if(ec == grammar::error::end)
                        break;
                    detail::throw_system_error(ec,
                        BOOST_CURRENT_LOCATION);
                }
                return h;
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
    // build buffer with table
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

    struct helper : fields_view
    {
        helper(
            string_view s,
            std::string& buf,
            fields_view const& f)
            : fields_view([](
                string_view s,
                std::string& buf,
                fields_view const& f)
                {
                    detail::header h;
                    h.kind = detail::kind::fields;
                    h.cbuf = buf.data();
                    h.cap = buf.size();
                    h.prefix = 0;
                    h.size = static_cast<
                        off_t>(s.size());
                    h.count = static_cast<
                        off_t>(f.size());
                    return h;
                }(s, buf, f))
        {
        }
    };

    return helper(s, buf, f);
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

                detail::header h;
                h.kind = detail::kind::request;
                h.cbuf = s.data();
                h.cap = 0;
                h.prefix = static_cast<
                    off_t>(it - s.data());
                h.size = static_cast<
                    off_t>(s.size());
                h.count = 0;
                h.req.method_len = static_cast<
                    off_t>(t0.ms.size());
                h.req.target_len = static_cast<
                    off_t>(t0.t.size());
                h.req.method = t0.m;
                h.req.version = t0.v;
                field_rule t1;
                for(;;)
                {
                    if(grammar::parse(
                        it, end, ec, t1))
                    {
                        ++h.count;
                        continue;
                    }
                    if(ec == grammar::error::end)
                        break;
                    detail::throw_system_error(ec,
                        BOOST_CURRENT_LOCATION);
                }
                return h;
            }())
        {
        }
    };
    return helper(s);
}

//------------------------------------------------

response_view
make_response(
    string_view s)
{
    struct helper : response_view
    {
        explicit
        helper(string_view s)
            : response_view(
            [&s]
            {
                error_code ec;
                auto it = s.data();
                auto const end =
                    it + s.size();
                status_line_rule t0;
                if(! grammar::parse(
                    it, end, ec, t0))
                    detail::throw_system_error(
                        ec, BOOST_CURRENT_LOCATION);

                detail::header h;
                h.kind = detail::kind::response;
                h.cbuf = s.data();
                h.cap = 0;
                h.prefix = static_cast<
                    off_t>(it - s.data());
                h.size = static_cast<
                    off_t>(s.size());
                h.count = 0;
                h.res.version = t0.v;
                h.res.status = int_to_status(
                    t0.status_int);
                h.res.status_int = t0.status_int;
                field_rule t1;
                for(;;)
                {
                    if(grammar::parse(
                        it, end, ec, t1))
                    {
                        ++h.count;
                        continue;
                    }
                    if(ec == grammar::error::end)
                        break;
                    detail::throw_system_error(ec,
                        BOOST_CURRENT_LOCATION);
                }
                return h;
            }())
        {
        }
    };
    return helper(s);
}

//------------------------------------------------

void
check(
    fields_view_base const& f,
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
    BOOST_TEST(f.string() == m);
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
    BOOST_TEST(req.string() == m);
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


//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_REQUEST_PARSER_IPP

#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/ctype.hpp>

namespace boost {
namespace http_proto {

//------------------------------------------------

request_parser::
request_parser(
    context& ctx) noexcept
    : basic_parser(ctx)
    , method_(http_proto::method::unknown)
    , n_method_(0)
    , n_target_(0)
{
}

request_view
request_parser::
header() const noexcept
{
    return request_view(
        buffer_,
        parsed_,
        n_method_,
        n_target_,
        method_,
        version_);
}

//------------------------------------------------

char*
request_parser::
parse_start_line(
    char* first,
    char const* const last,
    error_code& ec)
{
    auto it = first;

/*
    request-line   = method SP request-target SP HTTP-version CRLF
*/
    // method
    parse_method(it, last, ec);
    if(ec)
        return first;

    // request-target
    parse_target(it, last, ec);
    if(ec)
        return first;

    // HTTP-version
    parse_version(it, last, ec);
    if(ec)
        return first;

    // CRLF
    if(last - it < 2)
    {
        ec = error::need_more;
        return first;
    }
    if(it[0] != '\r' || it[1] != '\n')
    {
        ec = error::bad_version;
        return first;
    }
    it += 2;
    return it;
}

void
request_parser::
parse_method(
    char*& first,
    char const* last,
    error_code& ec)
{
    tchar_set ts;
    auto const need_more =
        [&ec]{ ec = error::need_more; };
    // token SP
    auto it = ts.skip(first, last);
    if(it == last)
        return need_more();
    if(*it != ' ')
    {
        // bad method char
        ec = error::bad_method;
        return;
    }
    if(it == first)
    {
        // empty method
        ec = error::bad_method;
        return;
    }

    string_view s(
        first, it++ - first);
    method_ = string_to_method(s);
    n_method_ = static_cast<
        off_t>(s.size());
    first = it;
}

void
request_parser::
parse_target(
    char*& first,
    char const* last,
    error_code& ec)
{
    auto const need_more =
        [&ec]{ ec = error::need_more; };
    // target SP
    auto it = first;
    for(;; ++it)
    {
        if(it == last)
            return need_more();
        if(! detail::is_pathchar(*it))
            break;
    }
    if(it == last)
        return need_more();
    if(*it != ' ')
    {
        // bad path char
        ec = error::bad_target;
        return;
    }
    if(it == first)
    {
        // empty target
        ec = error::bad_target;
        return;
    }
    string_view s(
        first, it++ - first);
    n_target_ = static_cast<
        off_t>(s.size());
    first = it;
}

void
request_parser::
finish_header(
    error_code& ec)
{
    (void)ec;
}

} // http_proto
} // boost

#endif

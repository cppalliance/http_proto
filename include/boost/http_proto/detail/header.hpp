//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_HEADER_HPP
#define BOOST_HTTP_PROTO_DETAIL_HEADER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

enum class version : char;
enum class method : char;
enum class status : unsigned short;
enum class field : unsigned short;

namespace detail {

struct fields_table;
struct const_fields_table;

enum class kind : unsigned char
{
    fields = 0,
    request,
    response, 
};

struct header
{
    detail::kind kind;

    struct req_t
    {
        off_t method_len;
        off_t target_len;
        http_proto::method method;
    };

    struct res_t
    {
        unsigned short status_int;
        http_proto::status status;
    };

    char const* cbuf = nullptr;
    char* buf = nullptr;
    std::size_t cap = 0;

    off_t size = 0;
    off_t count = 0;
    off_t prefix = 0;
    http_proto::version version;

    union
    {
        req_t req;
        res_t res;
    };

    content_length cl;

    BOOST_HTTP_PROTO_DECL
    explicit
    header(detail::kind k) noexcept;

    fields_table
    tab() noexcept;

    const_fields_table
    ctab() const noexcept;

    std::size_t
    find(field id) const noexcept;

    std::size_t
    find(string_view name) const noexcept;

    string_view
    name(std::size_t i) const noexcept;

    void reset() noexcept;

    void on_insert(field id, string_view v) noexcept;
    void on_insert_content_length(
        field id, string_view v) noexcept;

    // VFALCO swap() is in fields_view_base
};

BOOST_HTTP_PROTO_DECL
void
parse_start_line(
    header& h,
    std::size_t,
    error_code&) noexcept;

BOOST_HTTP_PROTO_DECL
bool
parse_field(
    header& h,
    std::size_t,
    field& id,
    string_view& v,
    error_code&) noexcept;

} // detail
} // http_proto
} // boost

#endif

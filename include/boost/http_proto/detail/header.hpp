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
#include <boost/assert.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {

enum class version : char;
enum class method : char;
enum class status : unsigned short;

namespace detail {

//------------------------------------------------

// field array stored at the
// end of allocated message data

struct fields_table_entry
{
    off_t np;   // name pos
    off_t nn;   // name size
    off_t vp;   // value pos
    off_t vn;   // value size
    field id;

    fields_table_entry
    operator+(std::size_t dv) const noexcept
    {
        return {
            static_cast<
                off_t>(np + dv),
            nn,
            static_cast<
                off_t>(vp + dv),
            vn,
            id };
    }

    fields_table_entry
    operator-(std::size_t dv) const noexcept
    {
        return {
            static_cast<
                off_t>(np - dv),
            nn,
            static_cast<
                off_t>(vp - dv),
            vn,
            id };
    }
};

//------------------------------------------------

struct fields_table
{
    explicit
    fields_table(
        void* end) noexcept
        : p_(reinterpret_cast<
            fields_table_entry*>(
                end))
    {
    }

    fields_table_entry&
    operator[](
        std::size_t i) const noexcept
    {
        return p_[-1 * (
            static_cast<
                long>(i) + 1)];
    }

private:
    fields_table_entry* p_;
};

//------------------------------------------------

// return total bytes needed
// to store message of `size`
// bytes and `count` fields.
inline
std::size_t
buffer_needed(
    std::size_t size,
    std::size_t count) noexcept
{
    // make sure `size` is big enough
    // to hold the largest default buffer:
    // "HTTP/1.1 200 OK\r\n\r\n"
    if( size < 19)
        size = 19;
    static constexpr auto A =
        alignof(fields_table_entry);
    return A * (
        (size + A - 1) / A) +
            (count * sizeof(
                fields_table_entry));
}

//------------------------------------------------

enum class kind : unsigned char
{
    fields = 0,
    request,
    response, 
};

struct header
{
    detail::kind kind;
    char const* cbuf = nullptr;
    char* buf = nullptr;
    std::size_t cap = 0;

    off_t size = 0;
    off_t count = 0;
    off_t prefix = 0;
    http_proto::version version;
    content_length cl;

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

    union
    {
        req_t req;
        res_t res;
    };

    BOOST_HTTP_PROTO_DECL
    explicit
    header(detail::kind k) noexcept;

    fields_table
    tab() const noexcept;

    std::size_t
    find(field id) const noexcept;

    std::size_t
    find(string_view name) const noexcept;

    void copy_table(void* dest,
        std::size_t n) const noexcept;

    void copy_table(
        void* dest) const noexcept
    {
        copy_table(dest, count);
    }

    void reset() noexcept;

    void on_insert(field id, string_view v) noexcept;
    void on_insert_content_length(
        field id, string_view v) noexcept;

    // VFALCO swap() is in fields_view_base
};

//------------------------------------------------

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

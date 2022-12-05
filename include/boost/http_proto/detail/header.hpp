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
#include <boost/http_proto/metadata.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/version.hpp>
#include <boost/assert.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {

class fields_base;

namespace detail {

enum kind : unsigned char
{
    fields = 0,
    request,
    response, 
};

struct empty
{
    kind param;
};

struct header
{
    // this field lookup table is
    // stored at the end of the
    // allocated buffer, in
    // reverse order.
    struct entry
    {
        off_t np;   // name pos
        off_t nn;   // name size
        off_t vp;   // value pos
        off_t vn;   // value size
        field id;

        entry operator+(
            std::size_t dv) const noexcept;
        entry operator-(
            std::size_t dv) const noexcept;
    };

    struct table
    {
        explicit
        table(
            void* end) noexcept
            : p_(reinterpret_cast<
                entry*>(end))
        {
        }

        entry&
        operator[](
            std::size_t i) const noexcept
        {
            return p_[-1 * (
                static_cast<
                    long>(i) + 1)];
        }

    private:
        entry* p_;
    };

    detail::kind kind;
    char const* cbuf = nullptr;
    char* buf = nullptr;
    std::size_t cap = 0;

    off_t size = 0;
    off_t count = 0;
    off_t prefix = 0;

    http_proto::version version =
        http_proto::version::http_1_1;
    metadata md;

    struct fld_t
    {
    };

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
        fld_t fld;
        req_t req;
        res_t res;
    };

    //--------------------------------------------

private:
    struct fields_tag {};
    struct request_tag {};
    struct response_tag {};

    constexpr header(fields_tag) noexcept;
    constexpr header(request_tag) noexcept;
    constexpr header(response_tag) noexcept;

public:
    // in fields_base.hpp
    static header& get(
        fields_base& f) noexcept;

    explicit
    header(empty) noexcept;

    BOOST_HTTP_PROTO_DECL
    static
    header const*
    get_default(detail::kind k) noexcept;

    BOOST_HTTP_PROTO_DECL
    header(detail::kind) noexcept;

    BOOST_HTTP_PROTO_DECL
    void swap(header&) noexcept;

    table tab() const noexcept;
    bool is_default() const noexcept;
    std::size_t find(field) const noexcept;
    std::size_t find(string_view) const noexcept;
    void copy_table(void*, std::size_t) const noexcept;
    void copy_table(void*) const noexcept;
    void assign_to(header&) const noexcept;

    // called when things change
    void on_start_line();
    void on_insert(field, string_view);
    void on_insert_clen(string_view);
    void on_insert_con(string_view);
    void on_insert_te();
    void on_insert_up(string_view);
    void on_erase(field);
    void on_erase_clen();
    void on_erase_con();
    void on_erase_te();
    void on_erase_up();
    void on_erase_all(field);
    void update_payload() noexcept;

    BOOST_HTTP_PROTO_DECL
    bool keep_alive() const noexcept;
};

//------------------------------------------------

BOOST_HTTP_PROTO_DECL
result<std::size_t>
parse_start_line(
    header& h,
    string_view s) noexcept;

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
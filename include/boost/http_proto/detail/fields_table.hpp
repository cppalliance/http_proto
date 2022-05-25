//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_FIELDS_TABLE_HPP
#define BOOST_HTTP_PROTO_DETAIL_FIELDS_TABLE_HPP

#include <boost/http_proto/string_view.hpp>
#include <boost/assert.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {

enum class field : unsigned short;

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

struct const_fields_table
{
    explicit
    const_fields_table(
        void const* end) noexcept
        : p_(reinterpret_cast<
            fields_table_entry const*>(
                end))
    {
    }

    fields_table_entry const&
    operator[](
        std::size_t i) const noexcept
    {
        return p_[-1 * (
            static_cast<
                long>(i) + 1)];
    }

    void
    copy(
        void* dest,
        std::size_t n) const noexcept
    {
        std::memcpy(
            reinterpret_cast<
                fields_table_entry*>(
                    dest) - n,
            p_ - n,
            n * sizeof(
                fields_table_entry));
    }

private:
    fields_table_entry const* p_;
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

    void
    copy(
        void* dest,
        std::size_t n) const noexcept
    {
        std::memcpy(
            reinterpret_cast<
                fields_table_entry*>(
                    dest) - n,
            p_ - n,
            n * sizeof(
                fields_table_entry));
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

} // detail
} // http_proto
} // boost

#endif

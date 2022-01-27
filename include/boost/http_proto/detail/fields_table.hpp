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
#include <boost/static_assert.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {

enum class field : unsigned short;

namespace detail {

struct fields_table_entry
{
    off_t np;   // name pos
    off_t nn;   // name size
    off_t vp;   // value pos
    off_t vn;   // value size
    field id;

    void
    adjust(long dv) noexcept
    {
        np = static_cast<
            off_t>(np + dv);
        vp = static_cast<
            off_t>(vp + dv);
    }
};

// field array stored at the
// end of allocated message data
template<bool IsConst>
class basic_fields_table
{
public:
    using value_type =
        fields_table_entry;

    using pointer = typename
        std::conditional<IsConst,
            value_type const*,
            value_type*>::type;

    using reference = typename
        std::conditional<IsConst,
            value_type const&,
            value_type&>::type;

    basic_fields_table() = default;

    basic_fields_table(
        basic_fields_table const&) = default;

    basic_fields_table&
        operator=(basic_fields_table const&) = default;

    explicit
    basic_fields_table(
        typename std::conditional<
            IsConst, void const*,
                void*>::type p) noexcept
        : p_(reinterpret_cast<
            pointer>(p))
    {
    }

    bool
    empty() const noexcept
    {
        return p_ == nullptr;
    }

    reference
    operator[](
        std::size_t i) const noexcept
    {
        return p_[-1 * (static_cast<
            long>(i) + 1)];
    }

    struct result
    {
        field id;
        string_view name;
        string_view value;
    };

    result
    operator()(
        char const* base,
        std::size_t i) const noexcept
    {
        result r;
        auto const& v = (*this)[i];
        r.name = { base + v.np, v.nn };
        r.value = { base + v.vp, v.vn };
        r.id = v.id;
        return r;
    }

private:
    pointer p_ = nullptr;
};

using const_fields_table =
    basic_fields_table<true>;

using fields_table =
    basic_fields_table<false>;

inline
std::size_t
fields_table_size(
    std::size_t n) noexcept
{
    return n * sizeof(
        fields_table_entry);
}

inline
void
write(
    fields_table& t,
    char const* base,
    std::size_t i,
    string_view name,
    string_view value,
    field id) noexcept
{
    auto& e = t[i];
    e.np = static_cast<off_t>(
        name.data() - base);
    e.vp = static_cast<off_t>(
        value.data() - base);
    e.nn = static_cast<
        off_t>(name.size());
    e.vn = static_cast<
        off_t>(value.size());
    e.id = id;
}

} // detail
} // http_proto
} // boost

#endif

//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_FTAB_HPP
#define BOOST_HTTP_PROTO_DETAIL_FTAB_HPP

#include <boost/http_proto/field.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {
namespace detail {

struct fitem
{
    off_t pos;          // start of field
    off_t name_pos;     // name offset
    off_t name_len;     // name length
    off_t value_pos;    // value offset
    off_t value_len;    // value length
    field id;

    void
    add(std::size_t amount) noexcept
    {
        pos += static_cast<
            off_t>(amount);
        name_pos += static_cast<
            off_t>(amount);
        value_pos += static_cast<
            off_t>(amount);
    }
};

// field array stored at the
// end of allocated message data
template<class Char>
struct ftab
{
#if 0
    BOOST_STATIC_ASSERT(
        std::is_same<Char,
        typename std::remove_const<
            Char>::type>::value);
#endif

    using value_type = 
        typename std::conditional<
            std::is_const<Char>::value,
            fitem const, fitem>::type;
    value_type* base;

    explicit
    ftab(Char* end)
        : base(reinterpret_cast<
            value_type*>(end))
    {
    }

    value_type&
    operator[](
        std::size_t i) const noexcept
    {
        return base[-1 * (
            static_cast<long>(i) + 1)];
    }
};

template<class Char>
ftab<Char>
get_ftab(Char* end)
{
    return ftab<Char>(end);
}

} // detail
} // http_proto
} // boost

#endif

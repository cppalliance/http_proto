//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_FIELDS_TABLE_HPP
#define BOOST_HTTP_PROTO_DETAIL_FIELDS_TABLE_HPP

#include <boost/http_proto/field.hpp>
#include <boost/static_assert.hpp>
#include <cstdint>
#include <type_traits>

namespace boost {
namespace http_proto {
namespace detail {

// headers have a maximum size of 65536 chars
using off_t = std::uint16_t;

struct fitem
{
    off_t name_pos;     // name start
    off_t name_len;     // name length
    off_t value_pos;    // value start
    off_t value_len;    // value length
    field id;
};

// field array stored at the
// end of allocated message data
template<class Char>
struct ftab
{
    BOOST_STATIC_ASSERT(
        std::is_same<Char,
        typename std::remove_const<
            Char>::type>::value);

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

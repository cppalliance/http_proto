//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/buffers
//

#ifndef BOOST_HTTP_PROTO_DETAIL_ZLIB_FILTER_HPP
#define BOOST_HTTP_PROTO_DETAIL_ZLIB_FILTER_HPP

#include <boost/http_proto/detail/workspace.hpp>

namespace boost {
namespace http_proto {
namespace detail {

/** utilities for zlib filter
*/
class zlib_filter
{
protected:
    static
    void* zalloc(
        void* opaque,
        unsigned items,
        unsigned size) noexcept
    {
        return reinterpret_cast<
            detail::workspace*>(opaque)->try_reserve_front(items * size);
    }

    static
    void
    zfree(void* /* opaque */, void* /* addr */) noexcept
    {
        // no-op
    }

    static
    unsigned int
    saturate_cast(std::size_t n) noexcept
    {
        if(n >= std::numeric_limits<unsigned int>::max())
            return std::numeric_limits<unsigned int>::max();
        return static_cast<unsigned int>(n);
    }
};

} // detail
} // http_proto
} // boost

#endif

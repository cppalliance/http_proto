//
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_MANAGED_HPP
#define BOOST_HTTP_PROTO_DETAIL_MANAGED_HPP

namespace boost {
namespace http_proto {
namespace detail {

template<typename T, T Default>
class managed
{
    T value_ = Default;

public:
    managed() = default;

    managed(T s) noexcept
        : value_(s)
    {
    }

    managed(managed&& other) noexcept
        : value_(other.value_)
    {
        other.value_ = Default;
    }

    managed&
    operator=(managed&& other) noexcept
    {
        value_ = other.value_;
        other.value_ = Default;
        return *this;
    }

    operator T() const noexcept
    {
        return value_;
    }
};

} // detail
} // http_proto
} // boost

#endif

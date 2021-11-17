//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_NUMBER_STRING_HPP
#define BOOST_HTTP_PROTO_DETAIL_NUMBER_STRING_HPP

#include <boost/http_proto/detail/config.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {
namespace detail {

// Convert integer to decimal
// string using in-place storage
class number_string
{
    static constexpr unsigned buf_size = 18;
    char buf_[buf_size + 1];
    std::size_t size_ = 0;

public:
    number_string() = default;
    number_string(
        number_string const&) = default;
    number_string& operator=
        (number_string const&) = default;

    explicit
    number_string(
        std::uint64_t n) noexcept
    {
        buf_[buf_size] = '\0';
        auto const end =
            &buf_[buf_size];
        auto p = end;
        if(n == 0)
        {
            *--p = '0';
        }
        else
        {
            while(n > 0)
            {
                *--p = '0' + (n%10);
                n /= 10;
            }
        }
        size_ = end - p;
    }

    char const*
    data() const noexcept
    {
        return &buf_[
            buf_size - 1] - size_;
    }

    std::size_t
    size() const noexcept
    {
        return size_;
    }

    operator string_view() const noexcept
    {
        return string_view(
            data(), size());
    }
};

} // detail
} // http_proto
} // boost

#endif

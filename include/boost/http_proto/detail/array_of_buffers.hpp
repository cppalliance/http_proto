//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_ARRAY_OF_BUFFERS_HPP
#define BOOST_HTTP_PROTO_DETAIL_ARRAY_OF_BUFFERS_HPP

#include <boost/buffers/const_buffer.hpp>
#include <boost/buffers/mutable_buffer.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<bool isConst>
class array_of_buffers
{
public:
    using value_type = typename
        std::conditional<isConst,
            buffers::const_buffer,
            buffers::mutable_buffer>::type;
    using iterator = value_type*;
    using const_iterator = iterator;

    array_of_buffers() = default;
    array_of_buffers(
        array_of_buffers const&) = default;
    array_of_buffers& operator=(
        array_of_buffers const&) = default;

    array_of_buffers(
        value_type* p,
        std::size_t n) noexcept;

    bool empty() const noexcept;
    value_type* data() const noexcept;
    std::size_t size() const noexcept;
    std::size_t capacity() const noexcept;
    value_type& operator[](std::size_t) const noexcept;
    void consume(std::size_t n);
    void reset(std::size_t n);

    iterator begin() const noexcept;
    iterator end() const noexcept;

private:
    value_type* o_ = nullptr;
    value_type* p_ = nullptr;
    std::size_t n_ = 0;
    std::size_t c_ = 0;
};

using array_of_const_buffers =
    array_of_buffers<true>;

using array_of_mutable_buffers =
    array_of_buffers<true>;

} // detail
} // http_proto
} // boost

#include <boost/http_proto/detail/impl/array_of_buffers.hpp>

#endif

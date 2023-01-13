//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_CONSUMING_BUFFERS_HPP
#define BOOST_HTTP_PROTO_DETAIL_CONSUMING_BUFFERS_HPP

#include <boost/http_proto/buffer.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<bool isConst>
class consuming_buffers
{
public:
    using value_type = typename
        std::conditional<isConst,
            const_buffer,
            mutable_buffer>::type;

    consuming_buffers() = default;
    consuming_buffers(
        consuming_buffers const&) = default;
    consuming_buffers& operator=(
        consuming_buffers const&) = default;

    consuming_buffers(value_type* p,
        std::size_t n) noexcept;

    bool empty() const noexcept;
    value_type* data() const noexcept;
    std::size_t count() const noexcept;
    value_type const* begin() const noexcept;
    value_type const* end() const noexcept;
    void consume(std::size_t n);

private:
    value_type* p_ = nullptr;
    std::size_t n_ = 0;
    std::size_t size_ = 0;
};

} // detail
} // http_proto
} // boost

#endif

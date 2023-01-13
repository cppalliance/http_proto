//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_CONSUMING_BUFFERS_IPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_CONSUMING_BUFFERS_IPP

#include <boost/http_proto/detail/consuming_buffers.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<bool isConst>
consuming_buffers<isConst>::
consuming_buffers(
    value_type* p,
    std::size_t n) noexcept
    : p_(p)
    , n_(n)
    , size_(buffer_size(*this))
{
}

template<bool isConst>
bool
consuming_buffers<isConst>::
empty() const noexcept
{
    return size_ == 0;
}

template<bool isConst>
auto
consuming_buffers<isConst>::
data() const noexcept ->
    value_type*
{
    return p_;
}

template<bool isConst>
std::size_t
consuming_buffers<isConst>::
count() const noexcept
{
    return n_;
}

template<bool isConst>
auto
consuming_buffers<isConst>::
begin() const noexcept ->
    value_type const* 
{
    return p_;
}

template<bool isConst>
auto
consuming_buffers<isConst>::
end() const noexcept ->
    value_type const*
{
    return p_ + n_;
}

template<bool isConst>
void
consuming_buffers<isConst>::
consume(std::size_t n)
{
    // Precondition violation
    if(n > size_)
        detail::throw_invalid_argument();

    while(n_ > 0)
    {
        if(n < p_->size())
        {
            *p_ += n;
            size_ -= n;
            return;
        }
        n -= p_->size();
        size_ -= p_->size();
        ++p_;
        --n_;
        if(n == 0)
            return;
    }
    if(n > 0)
        detail::throw_logic_error();
}

} // detail
} // http_proto
} // boost

#endif

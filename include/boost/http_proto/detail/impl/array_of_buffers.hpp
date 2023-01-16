//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_IMPL_ARRAY_OF_BUFFERS_HPP
#define BOOST_HTTP_PROTO_DETAIL_IMPL_ARRAY_OF_BUFFERS_HPP

#include <boost/http_proto/detail/except.hpp>
#include <boost/assert.hpp>

namespace boost {
namespace http_proto {
namespace detail {

template<bool isConst>
array_of_buffers<isConst>::
array_of_buffers(
    value_type* p,
    std::size_t n) noexcept
    : p_(p)
    , n_(n)
{
}

template<bool isConst>
bool
array_of_buffers<isConst>::
empty() const noexcept
{
    return n_ == 0;
}

template<bool isConst>
auto
array_of_buffers<isConst>::
data() const noexcept ->
    value_type*
{
    return p_;
}

template<bool isConst>
std::size_t
array_of_buffers<isConst>::
size() const noexcept
{
    return n_;
}

template<bool isConst>
auto
array_of_buffers<isConst>::
begin() const noexcept ->
    iterator
{
    return p_;
}

template<bool isConst>
auto
array_of_buffers<isConst>::
end() const noexcept ->
    iterator
{
    return p_ + n_;
}

template<bool isConst>
auto
array_of_buffers<isConst>::
operator[](
    std::size_t i) const noexcept ->
        value_type& 
{
    BOOST_ASSERT(i < n_);
    return p_[i];
}

template<bool isConst>
void
array_of_buffers<isConst>::
consume(std::size_t n)
{
    while(n_ > 0)
    {
        if(n < p_->size())
        {
            *p_ += n;
            return;
        }
        n -= p_->size();
        ++p_;
        --n_;
        if(n == 0)
            return;
    }

    // n exceeded available size
    if(n > 0)
        detail::throw_logic_error();
}

} // detail
} // http_proto
} // boost

#endif

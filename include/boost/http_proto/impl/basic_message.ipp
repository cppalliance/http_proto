//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_BASIC_MESSAGE_IPP
#define BOOST_HTTP_PROTO_IMPL_BASIC_MESSAGE_IPP

#include <boost/http_proto/basic_message.hpp>
#include <boost/http_proto/field.hpp>
#include <utility>

namespace boost {
namespace http_proto {

#if 0
std::size_t
basic_message::
next_pow2(
    std::size_t n) noexcept
{

}
#endif

class basic_message::resizer
{
    basic_message* self_;
    char* buf_;
    std::size_t cap_;
    std::size_t size_;
    std::size_t n_field_;

public:
    resizer(
        basic_message* self,
        std::size_t new_size,
        std::size_t new_fields)
        : self_(self)
        , buf_(self->buf_)
        , cap_(self->cap_)
        , size_(self->size_)
        , n_field_(self->n_field_)
    {
        auto const n = new_size +
            new_fields * sizeof(entry);
        auto cap = cap_;
        if( cap < 32)
            cap = 32;
        for(;;)
        {
            if(cap >= n)
                goto alloc;
            cap *= 2;
        }
        cap = std::size_t(-1);
    alloc:
        self_->buf_ =
            new char[cap];
        self_->cap_ = cap;
    }

    ~resizer()
    {
        if(buf_)
            delete[] buf_;
    }
};

//------------------------------------------------

basic_message::
basic_message() = default;

//------------------------------------------------

basic_message::
basic_message(
    string_view start_line)
{
    (void)start_line;
}

string_view
basic_message::
data() const noexcept
{
    if(buf_)
        return string_view(
            buf_, size_);
    return empty_string();
}

void
basic_message::
append(
    field f,
    string_view name,
    string_view value)
{
    auto const needed =
        size_ - 2 +
        name.size() + 2 +
        value.size() + 2 + 2;

    if(! buf_)
    {
        
    }    
}

char*
basic_message::
resize_start_line(
    std::size_t n)
{
    buf_ = new char[n] + 2;
    cap_ = n + 2;
    size_ = n + 2;
    return buf_;
}

} // http_proto
} // boost

#endif

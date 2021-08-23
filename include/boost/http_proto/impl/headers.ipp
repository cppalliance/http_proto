//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/boostorg/beast
//

#ifndef BOOST_HTTP_PROTO_IMPL_HEADERS_IPP
#define BOOST_HTTP_PROTO_IMPL_HEADERS_IPP

#include <boost/http_proto/headers.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/headers_view.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/detail/copied_strings.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/ftab.hpp>
#include <cstring>
#include <utility>

namespace boost {
namespace http_proto {

class headers::growth
{
    char* buf_ = nullptr;
    char* old_ = nullptr;

public:
    ~growth()
    {
        if(old_)
            delete[] old_;
    }

    growth(
        std::size_t size_extra,
        std::size_t count_extra)
    {
    }
};

//------------------------------------------------

constexpr
std::size_t
headers::
align_up(std::size_t n) noexcept
{
    auto const a = sizeof(
        detail::fitem);
    return a * ((n + a - 1) / a);
}

// returns minimum capacity to hold
// size characters and count table
// entries, including alignment.
constexpr
std::size_t
headers::
bytes_needed(
    std::size_t size,
    std::size_t count) noexcept
{
    return align_up(size +
        count * sizeof(
            detail::fitem));
}

headers::
headers(
    string_view empty_start) noexcept
    : empty_start_(empty_start)
{
}

headers::
~headers()
{
    if(buf_)
        delete[] buf_;
}

headers::
operator headers_view() const noexcept
{
    return headers_view(
        buf_,
        count_,
        start_bytes_,
        fields_bytes_,
        capacity_);
}

//------------------------------------------------
//
// Observers
//
//------------------------------------------------

auto
headers::
operator[](
    std::size_t i) const noexcept ->
        value_type const
{
    auto const ft =
        detail::get_ftab(
            buf_ + capacity_)[i];
    return value_type {
        ft.id,
        string_view(
            buf_ + ft.name_pos,
            ft.name_len),
        string_view(
            buf_ + ft.value_pos,
            ft.value_len) };
}

bool
headers::
exists(
    field id) const noexcept
{
    return find(id) != end();
}

bool
headers::
exists(
    string_view name) const noexcept
{
    return find(name) != end();
}

std::size_t
headers::
count(field id) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(v.id == id)
            ++n;
    return n;
}

std::size_t
headers::
count(string_view name) const noexcept
{
    std::size_t n = 0;
    for(auto v : *this)
        if(bnf::iequals(
            v.name, name))
            ++n;
    return n;
}

auto
headers::
at(std::size_t i) const ->
    value_type const
{
    if(i < count_)
    {
        auto const ft =
            detail::get_ftab(
                buf_ + capacity_)[i];
        return value_type {
            ft.id,
            string_view(
                buf_ + ft.name_pos,
                ft.name_len),
            string_view(
                buf_ + ft.value_pos,
                ft.value_len) };
    }
    detail::throw_invalid_argument(
        "bad index", BOOST_CURRENT_LOCATION);
}

string_view
headers::
at(field id) const
{
    auto it = find(id);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found", BOOST_CURRENT_LOCATION);
}

string_view
headers::
at(string_view name) const
{
    auto it = find(name);
    if(it != end())
        return it->value;
    detail::throw_invalid_argument(
        "not found", BOOST_CURRENT_LOCATION);
}

string_view
headers::
value_or(
    field id,
    string_view v) const noexcept
{
    auto it = find(id);
    if(it != end())
        return it->value;
    return v;
}

string_view
headers::
value_or(
    string_view name,
    string_view v) const noexcept
{
    auto it = find(name);
    if(it != end())
        return it->value;
    return v;
}

auto
headers::
find(field id) const noexcept ->
    iterator
{
    auto it = begin();
    auto const last = end();
    while(it != last)
    {
        if(it->id == id)
            break;
        ++it;
    }
    return it;
}

auto
headers::
find(string_view name) const noexcept ->
    iterator
{
    auto it = begin();
    auto const last = end();
    while(it != last)
    {
        if(bnf::iequals(
            it->name, name))
            break;
        ++it;
    }
    return it;
}

auto
headers::
matching(field id) const noexcept ->
    subrange
{
    return subrange(
        this, find(id).i_);
}

auto
headers::
matching(
    string_view name) const noexcept ->
        subrange
{
    return subrange(
        this, find(name).i_);
}

//------------------------------------------------
//
// Modifiers
//
//------------------------------------------------

void
headers::
clear() noexcept
{
}

void
headers::
reserve(std::size_t n)
{
    (void)n;
}

void
headers::
shrink_to_fit() noexcept
{
}

void
headers::
append(
    field f,
    string_view value)
{
    append(f, to_string(f), value);
}

void
headers::
append(
    string_view name,
    string_view value)
{
    append(string_to_field(name),
        name, value);
}

//------------------------------------------------
//
// private observers
//
//------------------------------------------------

string_view
headers::
str_impl() const noexcept
{
    if(buf_)
        return string_view(buf_,
            start_bytes_ +
                fields_bytes_);
    return empty_start_;
}

std::size_t
headers::
find(
    std::size_t after,
    field id) const noexcept
{
    std::size_t i = after;
    while(++i < count_)
        if((*this)[i].id == id)
            break;
    return i;
}

std::size_t
headers::
find(
    std::size_t after,
    string_view name) const noexcept
{
    std::size_t i = after;
    while(++i < count_)
        if(bnf::iequals(
            (*this)[i].name, name))
            break;
    return i;
}

//------------------------------------------------
//
// private modifiers
//
//------------------------------------------------

auto
headers::
alloc(
    std::size_t size,
    std::size_t count) ->
        alloc_t
{
    if(size > max_header_size_)
        detail::throw_length_error(
            "header too big",
            BOOST_CURRENT_LOCATION);
    if(! buf_)
    {
        // apply minimums to the
        // first allocation to prevent
        // many small reallocations.
        if( size < 256)
            size = 256; // bytes of header
        if( count < 8)
            count = 8;  // number of fields
        auto n = bytes_needed(
            size, count);
        return { new char[n], n };
    }

    // reallocate
    auto n = bytes_needed(size, count);
    BOOST_ASSERT(capacity_ < n);
    auto const growth = align_up(
        (capacity_ + capacity_ / 2));
    if(growth < capacity_)
    {
        // unsigned overflow
        detail::throw_length_error(
            "header too big",
            BOOST_CURRENT_LOCATION);
    }
    if( n < growth)
        n = growth;
    return { new char[n], n };
}

char*
headers::
resize_start_line(
    std::size_t n)
{
    if(! buf_)
    {
        // new buffer
        auto al =
            alloc(n + 2, 0);
        buf_ = al.buf;
        capacity_ =
            al.capacity;
        count_ = 0;
        start_bytes_ = n;
        fields_bytes_ = 2;
        buf_[n] = '\r';
        buf_[n+1] = '\n';
        return buf_;
    }

    if(n == fields_bytes_)
    {
        // no change in size
        return buf_;
    }

    auto const need =
        bytes_needed(
            n + fields_bytes_,
            count_);
    if(need <= capacity_)
    {
        // existing buffer
        std::memmove(
            buf_ + n,
            buf_ + start_bytes_,
            fields_bytes_);
        start_bytes_ = n;
        buf_[n] = '\r';
        buf_[n+1] = '\n';
        return buf_;
    }

    // grow
    auto al = alloc(
        n + fields_bytes_,
        count_);
    std::memmove(
        al.buf + n,
        buf_ + start_bytes_,
        fields_bytes_);
    delete[] buf_;
    buf_ = al.buf;
    start_bytes_ = n;
    capacity_ = al.capacity;
    buf_[n] = '\r';
    buf_[n+1] = '\n';
    return buf_;
}

void
headers::
append(
    field id,
    string_view name,
    string_view value)
{
    detail::copied_strings cs(
        str_impl());
    name = cs.maybe_copy(name);
    value = cs.maybe_copy(value);

    auto const need =
        bytes_needed(
            name.size() + 2 +
                value.size() + 2,
            count_ + 1);

    if(buf_ != nullptr)
    {
        if(capacity_ < need)
        {
            BOOST_ASSERT(
                start_bytes_ +
                fields_bytes_ >= 2);
            char* buf = new char[need];
            std::memcpy(buf, buf_,
                start_bytes_ +
                fields_bytes_ - 2);
            auto const tabsize = sizeof(
                detail::fitem) * count_;
            std::memcpy(
                buf + need - tabsize,
                buf_ + capacity_ - tabsize,
                tabsize);
            std::swap(buf_, buf);
            delete[] buf;
            capacity_ = need;
        }
    }
    else
    {
        //auto al = alloc(

        //buf_ = new char[need];
        count_ = 0;
        start_bytes_ = 0;
        fields_bytes_ = 2;
        capacity_ = need;
        buf_[0] = '\r';
        buf_[1] = '\n';
    }

    auto dest = buf_ +
        start_bytes_ +
        fields_bytes_;
    auto& ft = detail::get_ftab(
        buf_ + capacity_)[count_];
    ft.id = id;
    ft.name_len = static_cast<
        off_t>(name.size());
    ft.value_len = static_cast<
        off_t>(value.size());
    ft.name_pos = static_cast<
        off_t>(dest - buf_);
    std::memcpy(
        dest,
        name.data(),
        name.size());
    dest += name.size();
    *dest++ = ':';
    *dest++ = ' ';
    ft.value_pos = static_cast<
        off_t>(dest - buf_);
    std::memcpy(
        dest,
        value.data(),
        value.size());
    dest += value.size();
    *dest++ = '\r';
    *dest++ = '\n';
    *dest++ = '\r';
    *dest++ = '\n';
    fields_bytes_ =
        dest - buf_ -
            start_bytes_;
    ++count_;
}

//------------------------------------------------

std::string
headers::
subrange::
make_list() const
{
    auto it = begin();
    auto const end_ = end();
    std::string s;
    if(it == end_)
        return s;
    s.append(
        it->value.data(),
        it->value.size());
    while(++it != end_)
    {
        s.push_back(',');
        s.append(it->value.data(),
            it->value.size());
    }
    return s;
}

auto
headers::
subrange::
iterator::
operator++() noexcept ->
    iterator&
{
    BOOST_ASSERT(
        i_ < h_->size());
    if((*h_)[i_].id !=
        field::unknown)
    {
        i_ = h_->find(i_,
            (*h_)[i_].id);
    }
    else
    {
        i_ = h_->find(i_,
            (*h_)[i_].name);
    }
    return *this;
}

} // http_proto
} // boost

#endif

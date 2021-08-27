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
#include <boost/http_proto/headers_view.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/detail/copied_strings.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/ftab.hpp>
#include <cstring>
#include <new>
#include <utility>

namespace boost {
namespace http_proto {

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
    string_view empty) noexcept
    : buf_(nullptr)
    , empty_(empty)
    , count_(0)
    , start_len_(
        empty.size() - 2)
    , fields_len_(0) // excludes CRLF
    , cap_(empty.size())
{
}

//------------------------------------------------

headers::
~headers()
{
    if(buf_)
        delete[] buf_;
}

headers::
headers() noexcept
    : headers(
        string_view("\r\n"))
{
}

headers::
headers(headers const& other)
{
    // VFALCO TODO this needs
    // to respect the minimum capacity
    auto const tab_size =
        other.count_ *
            sizeof(detail::fitem);
    auto new_cap = align_up(
        other.start_len_ +
            other.fields_len_ +
            tab_size);
    buf_ = new char[new_cap];
    empty_ = other.empty_;
    count_ = other.count_;
    start_len_ = other.start_len_;
    fields_len_ = other.fields_len_;
    cap_ = new_cap;
    std::memcpy(
        buf_, other.buf_,
        start_len_ +
            fields_len_ + 2);
    std::memcpy(
        buf_ + cap_ - tab_size,
        other.buf_ + other.cap_ - tab_size,
        tab_size);
}

headers::
headers(headers&& other) noexcept
    : headers(other.empty_)
{
    swap(other);
}

headers&
headers::
operator=(
    headers&& other) noexcept
{
    headers temp(
        std::move(other));
    swap(temp);
    return *this;
}

headers&
headers::
operator=(
    headers const& other)
{
    headers temp(other);
    swap(temp);
    return *this;
}

//------------------------------------------------
//
// Observers
//
//------------------------------------------------

headers::
operator headers_view() const noexcept
{
    return headers_view(
        str_impl().data(),
        count_,
        start_len_,
        fields_len_,
        cap_);
}

auto
headers::
operator[](
    std::size_t i) const noexcept ->
        value_type const
{
    auto const& ft =
        detail::get_ftab(
            buf_ + cap_)[i];
    return value_type {
        ft.id,
        string_view(
            buf_ + ft.name_pos,
            ft.name_len),
        string_view(
            buf_ + ft.value_pos,
            ft.value_len) };
}

std::size_t
headers::
count(field id) const noexcept
{
    std::size_t n = 0;
    auto const* ft =
        &detail::get_ftab(
            buf_ + cap_)[0];
    for(auto i = count_;
            i > 0; --i, --ft)
        if(ft->id == id)
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
find(field id) const noexcept ->
    iterator
{
    auto const* ft =
        &detail::get_ftab(
            buf_ + cap_)[0];
    for(auto i = 0;
            i < count_; ++i, --ft)
        if(ft->id == id)
            return iterator(this, i);
    return iterator(this, count_);
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

std::size_t
headers::
find_next(
    std::size_t after,
    field id) const noexcept
{
    std::size_t i = after;
    auto const* ft =
        &detail::get_ftab(
            buf_ + cap_)[
                after];
    for(;--ft,++i < count_;)
        if(ft->id == id)
            break;
    return i;
}

std::size_t
headers::
find_next(
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

string_view
headers::
get_const_buffer() const noexcept
{
    if(buf_)
        return string_view(
            buf_ + start_len_,
            fields_len_ + 2);
    return empty_;
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
    if(! buf_)
        return;
    count_ = 0;
    start_len_ =
        empty_.size() - 2;
    fields_len_ = 0;
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
swap(headers& h) noexcept
{
    std::swap(buf_, h.buf_);
    std::swap(cap_, h.cap_);
    std::swap(empty_, h.empty_);
    std::swap(count_, h.count_);
    std::swap(start_len_, h.start_len_);
    std::swap(fields_len_, h.fields_len_);
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
            start_len_ +
            fields_len_ + 2);
    return empty_;
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
    BOOST_ASSERT(cap_ < n);
    auto const growth = align_up(
        (cap_ + cap_ / 2));
    if(growth < cap_)
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
set_start_line(
    std::size_t n)
{
    if(! buf_)
    {
        // new buffer
        auto al =
            alloc(n + 2, 0);
        buf_ = al.buf;
        cap_ =
            al.capacity;
        count_ = 0;
        start_len_ = n;
        fields_len_ = 0;
        buf_[n] = '\r';
        buf_[n+1] = '\n';
        return buf_;
    }

    if(n == fields_len_)
    {
        // no change in size
        return buf_;
    }

    auto const need =
        bytes_needed(
            n + fields_len_,
            count_);
    if(need <= cap_)
    {
        // existing buffer
        std::memmove(
            buf_ + n,
            buf_ + start_len_,
            fields_len_);
        start_len_ = n;
        buf_[n] = '\r';
        buf_[n+1] = '\n';
        return buf_;
    }

    // grow
    auto al = alloc(
        n + fields_len_,
        count_);
    std::memmove(
        al.buf + n,
        buf_ + start_len_,
        fields_len_);
    delete[] buf_;
    buf_ = al.buf;
    cap_ = al.capacity;
    start_len_ = n;
    buf_[n] = '\r';
    buf_[n+1] = '\n';
    return buf_;
}

void
headers::
insert(
    field id,
    string_view name,
    string_view value,
    std::size_t before)
{
    detail::copied_strings cs(
        str_impl());
    auto const extra =
        name.size() + 2 +
        value.size() + 2;
    auto new_cap = align_up(
        start_len_ +
            fields_len_ +
            extra + 2 +
        (count_ + 1) + 2 *
            sizeof(detail::fitem));

    if(! buf_)
    {
        // first allocation
        BOOST_ASSERT(
            empty_.size() < 40);
        auto constexpr min_cap =
            align_up(256 + 8 *
            sizeof(detail::fitem));
        // prevent small allocs
        if( new_cap < min_cap)
            new_cap = min_cap;
        buf_ = new char[new_cap];

        // copy default start-line
        // since we are inserting a field
        BOOST_ASSERT(
            empty_.size() >= 2);
        BOOST_ASSERT(start_len_ ==
            empty_.size() - 2);
        BOOST_ASSERT(
            fields_len_ == 0);
        std::memcpy(
            buf_,
            empty_.data(),
            start_len_);
        cap_ = new_cap;
    }
    else if(new_cap > cap_)
    {
        // reallocate

    }
    else
    {
        // handle self modifying params
        name = cs.maybe_copy(name);
        value = cs.maybe_copy(value);
    }

    auto const ft = detail::get_ftab(
        buf_ + cap_);
    auto* fi = &ft[before];
    if(before < count_)
    {
        std::memmove(
            buf_ + fi->pos + extra,
            buf_ + fi->pos,
            fields_len_ - fi->pos);
        std::memmove(
            &ft[count_],
            &ft[count_ - 1],
            (count_ - before) *
                sizeof(*fi));
        for(std::size_t i = count_;
                i > before; ++i)
        {
            ft[i] = ft[i-1];
            ft[i].add(extra);
        }
    }
    else
    {
        fi->pos = static_cast<
            off_t>(fields_len_);
        auto dest = buf_ +
            start_len_ +
                fields_len_ +
                extra;
        dest[0] = '\r';
        dest[1] = '\n'; 
    }

    // write the inserted field
    auto dest = buf_ +
        start_len_ + fi->pos;
    fi->id = id;
    fi->name_len = static_cast<
        off_t>(name.size());
    fi->value_len = static_cast<
        off_t>(value.size());
    fi->name_pos = static_cast<
        off_t>(dest - buf_);
    std::memcpy(
        dest,
        name.data(),
        name.size());
    dest += name.size();
    *dest++ = ':';
    *dest++ = ' ';
    fi->value_pos = static_cast<
        off_t>(dest - buf_);
    std::memcpy(
        dest,
        value.data(),
        value.size());
    dest += value.size();
    *dest++ = '\r';
    *dest++ = '\n';
    fields_len_ += extra;
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
        i_ = h_->find_next(
            i_, (*h_)[i_].id);
    }
    else
    {
        i_ = h_->find_next(
            i_, (*h_)[i_].name);
    }
    return *this;
}

} // http_proto
} // boost

#endif

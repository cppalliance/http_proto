//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
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

/*  Implementation notes

    When buf_ == nullptr, the serialized string
        is equivalent to s_owner_[owner_]

    fields_len_ does not include the final CRLF,
        which is always present.
*/

string_view const
headers::
s_owner_[3] = {
    { "\r\n" },
    { "GET / HTTP/1.1\r\n\r\n" },
    { "HTTP/1.1 200 OK\r\n\r\n" }
};

// also applies min capacity
std::size_t
headers::
align_up(std::size_t n) noexcept
{
    constexpr std::size_t
        min_cap = 256 +
        8 * sizeof(detail::fitem);
    if( n < min_cap)
        n = min_cap;
    auto const a = sizeof(
        detail::fitem);
    auto const new_cap =
        a * ((n + a - 1) / a);
    return new_cap;
}

// returns minimum capacity to hold
// size characters and count table
// entries, including alignment.
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
    int owner) noexcept
    : owner_(owner)
{
    owner_default();
}

// copy-construct as owner,
// extra param is just a placeholder
// to create a separate overload
headers::
headers(
    headers const& other,
    int owner)
    : owner_(owner)
{
    BOOST_ASSERT(
        owner_ == other.owner_);
    if(other.buf_ == nullptr)
    {
        owner_default();
        return;
    }
    auto const tab_size =
        other.count_ *
            sizeof(detail::fitem);
    // copy the start line
    auto new_cap = align_up(
        other.start_len_ +
        other.fields_len_ +
            2 + tab_size);
    buf_ = new char[new_cap];
    cap_ = new_cap;
    count_ = other.count_;
    start_len_ = other.start_len_;
    fields_len_ =
        other.fields_len_;
    std::memcpy(
        buf_,
        other.buf_,
        start_len_ +
            fields_len_ + 2);
    std::memcpy(
        buf_ + cap_ - tab_size,
        other.buf_ +
            other.cap_ - tab_size,
        tab_size);
}

// swap everything
void
headers::
owner_swap(
    headers& other) noexcept
{
    std::swap(buf_, other.buf_);
    std::swap(cap_, other.cap_);
    std::swap(count_, other.count_);
    std::swap(start_len_, other.start_len_);
    std::swap(fields_len_, other.fields_len_);
}

// set default-constructed
// state for owner
void
headers::
owner_default() noexcept
{
    buf_ = nullptr;
    cap_ = 0;
    count_ = 0;
    start_len_ = s_owner_[
      owner_].size() - 2;
    fields_len_ = 0;
}

// return serialized string
// including the start-line
string_view
headers::
owner_str() const noexcept
{
    if(buf_)
        return string_view(buf_,
            start_len_ +
            fields_len_ + 2);
    return s_owner_[owner_];
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
    : headers(0)
{
}

// other becomes defaulted,
// this gets only fields
headers::
headers(headers&& other) noexcept
    : owner_(0)
{
    buf_ = other.buf_;
    cap_ = other.cap_;
    count_ = other.count_;
    start_len_ = 0;
    fields_len_ = other.fields_len_;
    if(buf_ && other.start_len_ > 0)
    {
        // remove start-line
        std::memmove(
            buf_, buf_ +
                other.start_len_,
            fields_len_ + 2);
    }
    other.owner_default();
}

// doesn't copy start-line
headers::
headers(headers const& other)
    : headers(0)
{
    if(other.buf_ == nullptr)
        return;
    auto const tab_size =
        other.count_ *
            sizeof(detail::fitem);
    // don't copy the start line
    auto new_cap = align_up(
        other.fields_len_ +
            2 + tab_size);
    buf_ = new char[new_cap];
    cap_ = new_cap;
    count_ = other.count_;
    fields_len_ =
        other.fields_len_;
    std::memcpy(
        buf_,
        other.buf_ +
            other.start_len_,
        fields_len_ + 2);
    std::memcpy(
        buf_ + cap_ - tab_size,
        other.buf_ +
            other.cap_ - tab_size,
        tab_size);
    auto ft = detail::get_ftab(
        buf_ + cap_);
    for(std::size_t i = 0;
            i < count_; ++i)
        ft[i].add(
            0 - other.start_len_);
}

// copy fields only
headers&
headers::
operator=(
    headers const& other)
{
    if(other.buf_ == nullptr)
    {
        // empty container,
        // keep capacity
        count_ = 0;
        start_len_ = 0;
        fields_len_ = 0;
        return *this;
    }
    auto const tab_size =
        other.count_ *
            sizeof(detail::fitem);
    auto const new_cap = align_up(
        start_len_ +
        other.fields_len_ + 2 +
        tab_size);
    if(cap_ < new_cap)
    {
        // allocate new
        auto buf = new char[new_cap];
        if(buf_)
        {
            std::memcpy(
                buf,
                buf_,
                start_len_);
            delete[] buf_;
        }
        else
        {
            std::memcpy(
                buf,
                s_owner_[owner_].data(),
                start_len_);
        }
        buf_ = buf;
        cap_ = new_cap;
    }
    count_ = other.count_;
    fields_len_ = other.fields_len_;
    std::memcpy(
        buf_ + start_len_,
        other.buf_ +
            other.start_len_,
        fields_len_ + 2);
    std::memcpy(
        buf_ + cap_ - tab_size,
        other.buf_ +
            other.cap_ - tab_size,
        tab_size);
    return *this;
}

//------------------------------------------------
//
// Observers
//
//------------------------------------------------

// excludes start-line
headers::
operator headers_view() const noexcept
{
    return headers_view(
        owner_str().data(),
        cap_,
        count_,
        start_len_,
        fields_len_);
}

auto
headers::
operator[](
    std::size_t i) const noexcept ->
        value_type const
{
    BOOST_ASSERT(i < count_);
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
    for(std::size_t i = 0;
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
    return s_owner_[owner_];
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
    start_len_ = s_owner_[
        owner_].size() - 2;
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

// swap ONLY the fields
void
headers::
swap(headers& h)
{
    std::swap(buf_, h.buf_);
    std::swap(cap_, h.cap_);
    std::swap(count_, h.count_);
    std::swap(start_len_, h.start_len_);
    std::swap(fields_len_, h.fields_len_);
}

//------------------------------------------------
//
// private
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

    if(n == start_len_)
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
        owner_str());
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
        auto empty = s_owner_[owner_];
        auto const min_cap =
            align_up(256 + 8 *
            sizeof(detail::fitem));
        // prevent small allocs
        if( new_cap < min_cap)
            new_cap = min_cap;
        buf_ = new char[new_cap];

        // copy default start-line
        // since we are inserting a field
        BOOST_ASSERT(start_len_ ==
            empty.size() - 2);
        BOOST_ASSERT(
            fields_len_ == 0);
        std::memcpy(
            buf_,
            empty.data(),
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

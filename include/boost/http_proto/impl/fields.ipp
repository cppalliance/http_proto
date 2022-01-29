//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_FIELDS_IPP
#define BOOST_HTTP_PROTO_IMPL_FIELDS_IPP

#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/assert/source_location.hpp>
#include <string>

namespace boost {
namespace http_proto {

fields::
fields(
    ctor_params const& init) noexcept
    : fields_view(init)
    , buf_(init.buf)
    , kind_(init.kind)
{
}

fields::
fields(
    char kind) noexcept
    : fields_view(kind)
    , buf_(nullptr)
    , kind_(kind)
{
}

// copy with start-line
fields::
fields(
    fields_view const& fv,
    char kind)
    : fields(
    [&fv, kind]
    {
        ctor_params init;
        if(! is_default(fv.cbuf_))
        {
            // copy start line and fields
            auto n = detail::buffer_needed(
                fv.end_pos_, fv.count_);
            auto buf = new char[n];
            std::memcpy(
                buf, fv.cbuf_, fv.end_pos_);
            fv.write_table(buf + n);
            init.cbuf = buf;
            init.buf_len = n;
            init.start_len = fv.start_len_;
            init.end_pos = fv.end_pos_;
            init.count = fv.count_;
            init.buf = buf;
            init.kind = kind;
            return init;
        }

        // default buffer
        init.cbuf = fv.cbuf_;
        init.buf_len = 0;
        init.start_len = fv.start_len_;
        init.end_pos = fv.end_pos_;
        init.count = fv.count_;
        init.buf = nullptr;
        init.kind = kind;
        return init;       
    }())
{
}

//------------------------------------------------
//
// Special Members
//
//------------------------------------------------

fields::
~fields()
{
    if(buf_)
        delete[] buf_;
}

fields::
fields() noexcept
    : fields(0)
{
}

fields::
fields(
    fields&& other) noexcept
    : fields(other.kind_)
{
    this->swap(other);
}

fields::
fields(
    fields const& f)
    : fields(static_cast<
        fields_view const&>(f))
{
}

// copy without start-line
fields::
fields(
    fields_view const& f)
    : fields(
    [&f]
    {
        ctor_params init;
        if(f.count_ > 0)
        {
            // copy fields
            auto n = detail::buffer_needed(
                f.end_pos_ - f.start_len_,
                    f.count_);
            auto buf = new char[n];
            std::memcpy(
                buf,
                f.cbuf_ + f.start_len_,
                f.end_pos_ - f.start_len_);
            f.write_table(buf + n);
            init.cbuf = buf;
            init.buf_len = n;
            init.start_len = 0;
            init.end_pos =
                f.end_pos_ - f.start_len_;
            init.count = f.count_;
            init.buf = buf;
            init.kind = 0;
            return init;
        }

        // default buffer
        auto const s =
            default_buffer(0);
        init.cbuf = s.data();
        init.buf_len = 0;
        init.start_len = s.size() - 2;
        init.end_pos = s.size();
        init.count = 0;
        init.buf = nullptr;
        init.kind = 0;
        return init;       
    }())
{
}

fields&
fields::
operator=(
    fields&& f) noexcept
{
    fields tmp(std::move(f));
    tmp.swap(*this);
    return *this;
}

fields&
fields::
operator=(
    fields const& f)
{
    *this = static_cast<
        fields_view const&>(f);
    return *this;
}

// copy fields in f
// without start-line
fields&
fields::
operator=(
    fields_view const& f)
{
    BOOST_ASSERT(kind_ == 0);
    if(is_default(f.cbuf_))
    {
        fields tmp;
        tmp.swap(*this);
        return *this;
    }
    auto const n0 =
        f.end_pos_ -
        f.start_len_;
    auto const n =
        detail::buffer_needed(
            n0, f.count_);
    if(buf_len_ < n)
    {
        // copy with strong
        // exception safety
        fields tmp(f);
        tmp.swap(*this);
        return *this;
    }
    // use existing capacity
    std::memcpy(
        buf_,
        f.cbuf_ +
            f.start_len_,
        n0);
    f.write_table(
        buf_ + buf_len_);
    start_len_ = 0;
    end_pos_ = static_cast<
        off_t>(n0);
    count_ = f.count_;
    return *this;
}

//------------------------------------------------

void
fields::
clear() noexcept
{
    if(! buf_)
        return;
    auto s =
        default_buffer(kind_);
    end_pos_ = static_cast<
        off_t>(s.size());
    start_len_ = end_pos_ - 2;
    count_ = 0;
    std::memcpy(
        buf_,
        s.data(),
        end_pos_);
}

void
fields::
reserve(std::size_t n)
{
    // align up
    n = detail::buffer_needed(n, 0);
    if(n <= buf_len_)
        return;
    auto buf = new char[n];
    if(buf_len_ > 0)
    {
        std::memcpy(
            buf,
            cbuf_,
            end_pos_);
        write_table(buf + n);
        delete[] buf_;
    }
    else
    {
        // default buffer
        std::memcpy(
            buf,
            cbuf_,
            end_pos_);
    }
    buf_len_ = n;
    cbuf_ = buf;
    buf_ = buf;
}

void
fields::
shrink_to_fit() noexcept
{
    if(detail::buffer_needed(
        end_pos_, count_) >=
            buf_len_)
        return;
    fields(*this, 0).swap(*this);
}

auto
fields::
emplace(
    iterator before,
    field id,
    string_view value) ->
        iterator
{
    insert_impl(
        id,
        to_string(id),
        value,
        before.i_);
    return before;
}

auto
fields::
emplace(
    iterator before,
    string_view name,
    string_view value) ->
        iterator
{
    insert_impl(
        string_to_field(name),
        name,
        value,
        before.i_);
    return before;
}

void
fields::
emplace_back(
    field id,
    string_view value)
{
    insert_impl(
        id,
        to_string(id),
        value,
        count_);
}

void
fields::
emplace_back(
    string_view name,
    string_view value)
{
    insert_impl(
        string_to_field(
            name),
        name,
        value,
        count_);
}

auto
fields::
erase(iterator it) noexcept ->
    iterator
{
    BOOST_ASSERT(it.i_ < count_);
    BOOST_ASSERT(buf_ != nullptr);
    detail::fields_table ft(
        buf_ + buf_len_);
    auto const p0 =
        offset(ft, it.i_);
    auto const p1 =
        offset(ft, it.i_ + 1);
    std::memmove(
        buf_ + p0,
        buf_ + p1,
        end_pos_ - p1);
    auto const n = p1 - p0;
    --count_;
    for(std::size_t i = it.i_;
            i < count_; ++i)
        ft[i] = ft[i + 1] - n;
    end_pos_ = static_cast<
        off_t>(end_pos_ - n);
    return iterator(this, it.i_);
}

std::size_t
fields::
erase(
    field id) noexcept
{
    auto it = find(id);
    if(it != end())
    {
        erase(it);
        return 1;
    }
    return 0;
}

std::size_t
fields::
erase(
    string_view name) noexcept
{
    auto it = find(name);
    if(it != end())
    {
        erase(it);
        return 1;
    }
    return 0;
}

std::size_t
fields::
erase_all(
    field id) noexcept
{
    std::size_t n = 0;
    std::size_t i = count_;
    while(i > 0)
    {
        --i;
        iterator it(this, i);
        if(it->id != id)
            continue;
        erase(it);
        ++n;
    }
    return n;
}

std::size_t
fields::
erase_all(
    string_view name) noexcept
{
    std::size_t n = 0;
    std::size_t i = count_;
    while(i > 0)
    {
        --i;
        iterator it(this, i);
        if(bnf::iequals(
            it->name, name))
        {
            erase(it);
            ++n;
        }
    }
    return n;
}

//------------------------------------------------

void
fields::
swap(fields& other)
{
    using std::swap;
    static_cast<fields_view&>(
        *this).swap(other);
    swap(buf_, other.buf_);
    swap(kind_, other.kind_);
}

//------------------------------------------------

// return i-th field absolute offset
std::size_t
fields::
offset(
    detail::fields_table const& ft,
    std::size_t i) const noexcept
{
    if(i < count_)
        return start_len_ + ft[i].np;
    // make final CRLF the last "field"
    BOOST_ASSERT(i == count_);
    return end_pos_ - 2;
}

void
fields::
insert_impl(
    field id,
    string_view name,
    string_view value,
    std::size_t before)
{
    BOOST_ASSERT(before <= count_);
    auto const n0 =
        name.size() + 2 +
        value.size() + 2;
    auto const n1 = end_pos_ + n0;
    // VFALCO enforce minimum new_size=19
    if(n1 > max_off_t)
        detail::throw_length_error(
            "too large",
            BOOST_CURRENT_LOCATION);
    auto const n =
        detail::buffer_needed(
            n1, count_ + 1);
    if(buf_len_ >= n)
    {
        BOOST_ASSERT(buf_ != nullptr);
        detail::fields_table ft(
            buf_ + buf_len_);
        auto pos = offset(ft, before);
        char* dest = buf_ + pos;
        std::memmove(
            dest + n0,
            dest,
            end_pos_ - pos);
        name.copy(dest, name.size());
        dest += name.size();
        *dest++ = ':';
        *dest++ = ' ';
        value.copy(dest, value.size());
        dest += value.size();
        *dest++ = '\r';
        *dest++ = '\n';
        for(std::size_t i = count_;
            i > before; --i)
            ft[i] = ft[i-1] + n0;
        pos -= start_len_;
        auto& e = ft[before];
        e.np = static_cast<
            off_t>(pos);
        e.nn = static_cast<
            off_t>(name.size());
        e.vp = static_cast<off_t>(
            pos + name.size() + 2);
        e.vn = static_cast<
            off_t>(value.size());
        e.id = id;
        end_pos_ = static_cast<
            off_t>(end_pos_ + n0);
        ++count_;
    }
    else if(buf_len_ > 0)
    {
        // grow
        BOOST_ASSERT(buf_ != nullptr);
        auto buf = new char[n];

        // copy existing fields
        // and write new field
        detail::fields_table ft0(
            buf_ + buf_len_);
        auto pos = offset(ft0, before);
        char* dest = buf;
        std::memcpy(
            dest,
            buf_,
            pos);
        dest += pos;
        name.copy(dest, name.size());
        dest += name.size();
        *dest++ = ':';
        *dest++ = ' ';
        value.copy(dest, value.size());
        dest += value.size();
        *dest++ = '\r';
        *dest++ = '\n';
        std::memcpy(
            dest,
            buf_ + pos,
            end_pos_ - pos);

        // write table
        ft0.copy(buf + n, before);
        detail::fields_table ft(buf + n);
        for(auto i = before;
            i < count_; ++i)
            ft[i + 1] = ft0[i] + n0;

        auto& e = ft[before];
        e.np = static_cast<
            off_t>(pos);
        e.nn = static_cast<
            off_t>(name.size());
        e.vp = static_cast<off_t>(
            pos + name.size() + 2);
        e.vn = static_cast<
            off_t>(value.size());
        e.id = id;

        delete[] buf_;
        buf_ = buf;
        cbuf_ = buf;
        buf_len_ = n;
        end_pos_ = static_cast<
            off_t>(n1);
        count_ += 1;
    }
    else
    {
        BOOST_ASSERT(buf_ == nullptr);
        BOOST_ASSERT(before == 0);
        BOOST_ASSERT(count_ == 0);

        // initial allocation
        auto s = default_buffer(kind_);
        buf_ = new char[n];
        cbuf_ = buf_;
        buf_len_ = n;
        start_len_ = static_cast<
            off_t>(s.size() - 2);
        end_pos_ = static_cast<
            off_t>(n1);
        count_ = 1;

        // start line
        char* dest = buf_;
        s.copy(dest, start_len_);
        dest += start_len_;

        // write field
        name.copy(dest, name.size());
        dest += name.size();
        *dest++ = ':';
        *dest++ = ' ';
        value.copy(dest, value.size());
        dest += value.size();
        *dest++ = '\r';
        *dest++ = '\n';

        *dest++ = '\r';
        *dest++ = '\n';

        // write table
        detail::fields_table ft(
            buf_ + buf_len_);
        auto& e = ft[before];
        e.id = id;
        e.np = 0;
        e.nn = static_cast<
            off_t>(name.size());
        e.vp = static_cast<
            off_t>(name.size() + 2);
        e.vn = static_cast<
            off_t>(value.size());
    }
}

char*
fields::
set_start_line_impl(
    std::size_t n)
{
    if( n > start_len_ ||
        buf_ == nullptr)
    {
        // allocate or grow
        if( n > start_len_ &&
            n - start_len_ >
                max_off_t - end_pos_)
            detail::throw_length_error(
                "too large",
                BOOST_CURRENT_LOCATION);
        auto n0 = detail::buffer_needed(
            n + end_pos_ - start_len_,
            count_);
        auto buf = new char[n0];
        if(buf_ != nullptr)
        {
            std::memcpy(
                buf + n,
                buf_ + start_len_,
                end_pos_ - start_len_);
            detail::fields_table ft(
                buf_ + buf_len_);
            ft.copy(buf + n0, count_);
            delete[] buf_;
        }
        else
        {
            std::memcpy(
                buf + n,
                cbuf_ + start_len_,
                end_pos_ - start_len_);
        }
        buf_ = buf;
        cbuf_ = buf;
        end_pos_ = static_cast<
            off_t>(end_pos_ +
                n - start_len_);
        start_len_ = static_cast<
            off_t>(n);
        buf_len_ = n0;
        return buf_;
    }

    // shrink
    std::memmove(
        buf_ + n,
        buf_ + start_len_,
        end_pos_ - start_len_);
    end_pos_ = static_cast<
        off_t>(end_pos_ -
            start_len_ + n);
    start_len_ = static_cast<
        off_t>(n);
    return buf_;
}

} // http_proto
} // boost

#endif

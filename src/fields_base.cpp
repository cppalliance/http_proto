//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/header.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields_base.hpp>
#include <boost/http_proto/header_limits.hpp>
#include <boost/http_proto/rfc/token_rule.hpp>

#include "src/detail/move_chars.hpp"
#include "src/rfc/detail/rules.hpp"

#include <boost/assert.hpp>
#include <boost/assert/source_location.hpp>
#include <boost/core/detail/string_view.hpp>
#include <boost/system/result.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/error.hpp>
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/token_rule.hpp>

namespace boost {
namespace http_proto {

namespace {

std::size_t
align_down(
    void * ptr,
    std::size_t size,
    std::size_t alignment)
{
    auto addr = reinterpret_cast<std::uintptr_t>(ptr);
    auto aligned_end = (addr + size) & ~(alignment - 1);

    if(aligned_end > addr)
        return aligned_end - addr;

    return 0;
}

void
verify_field_name(
    core::string_view name,
    system::error_code& ec)
{
    auto rv = grammar::parse(
        name, detail::field_name_rule);
    if(rv.has_error())
    {
        ec = BOOST_HTTP_PROTO_ERR(
            error::bad_field_name);
    }
}

system::result<detail::field_value_rule_t::value_type>
verify_field_value(
    core::string_view value)
{
    auto it = value.begin();
    auto end = value.end();
    auto rv =
        grammar::parse(it, end, detail::field_value_rule);
    if( rv.has_error() )
    {
        if( rv.error() == condition::need_more_input )
            return error::bad_field_value;
        return rv.error();
    }

    if( rv->has_crlf )
        return error::bad_field_smuggle;

    if( it != end )
        return error::bad_field_value;

    return rv;
}

} // namespace

class fields_base::
    op_t
{
    fields_base& self_;
    core::string_view* s0_;
    core::string_view* s1_;
    char* buf_ = nullptr;
    char const* cbuf_ = nullptr;
    std::size_t cap_ = 0;

public:
    explicit
    op_t(
        fields_base& self,
        core::string_view* s0 = nullptr,
        core::string_view* s1 = nullptr) noexcept
        : self_(self)
        , s0_(s0)
        , s1_(s1)
    {
    }

    ~op_t()
    {
        if(buf_)
            delete[] buf_;
    }

    char const*
    buf() const noexcept
    {
        return buf_;
    }

    char const*
    cbuf() const noexcept
    {
        return cbuf_;
    }

    char*
    end() const noexcept
    {
        return buf_ + cap_;
    }

    table
    tab() const noexcept
    {
        return table(end());
    }

    bool
    reserve(std::size_t n);

    bool
    grow(
        std::size_t extra_char,
        std::size_t extra_field);

    void
    move_chars(
        char* dest,
        char const* src,
        std::size_t n) const noexcept;
};

bool
fields_base::
op_t::
reserve(
    std::size_t n)
{
    if(n > self_.max_cap_)
    {
        // max capacity exceeded
        detail::throw_length_error();
    }
    if(n <= self_.h_.cap)
        return false;
    auto buf = new char[n];
    buf_ = self_.h_.buf;
    cbuf_ = self_.h_.cbuf;
    cap_ = self_.h_.cap;
    self_.h_.buf = buf;
    self_.h_.cbuf = buf;
    self_.h_.cap = n;
    return true;
}

bool
fields_base::
op_t::
grow(
    std::size_t extra_char,
    std::size_t extra_field)
{
    if(extra_field > detail::header::max_offset - self_.h_.count)
        detail::throw_length_error();

    if(extra_char > detail::header::max_offset - self_.h_.size)
        detail::throw_length_error();

    return reserve(
        detail::header::bytes_needed(
            self_.h_.size + extra_char,
            self_.h_.count + extra_field));
}

void
fields_base::
op_t::
move_chars(
    char* dest,
    char const* src,
    std::size_t n) const noexcept
{
    detail::move_chars(
        dest, src, n, s0_, s1_);
}

//------------------------------------------------

fields_base::
prefix_op_t::
prefix_op_t(
    fields_base& self,
    std::size_t new_prefix,
    core::string_view* s0,
    core::string_view* s1)
    : self_(self)
    , new_prefix_(static_cast<
        offset_type>(new_prefix))
{
    if(self.h_.size - self.h_.prefix + new_prefix
        > detail::header::max_offset)
        detail::throw_length_error();

    // memmove happens in the destructor
    // to avoid overlaping with start line.
    if(new_prefix_ < self_.h_.prefix
        && !self.h_.is_default())
        return;

    auto new_size = static_cast<offset_type>(
        self.h_.size - self.h_.prefix + new_prefix_);

    if(new_size > self.h_.cap)
    {
        // static storage will always throw which is
        // intended since they cannot reallocate.
        if(self.max_cap_ < new_size)
            detail::throw_length_error();

        auto bytes_needed =
            detail::header::bytes_needed(
                new_size,
                self.h_.count);

        char* p = new char[bytes_needed];
        std::memcpy(
            p + new_prefix_,
            self.h_.cbuf + self.h_.prefix,
            self.h_.size - self.h_.prefix);
        self.h_.copy_table(p + bytes_needed);

        // old buffer gets released in the destructor
        // to avoid invalidating any string_views
        // that may still reference it.
        buf_        = self.h_.buf;
        self.h_.buf = p;
        self.h_.cap = bytes_needed;
    }
    else
    {
        // memmove to the right and update any
        // string_views that reference that region.
        detail::move_chars(
            self.h_.buf + new_prefix_,
            self.h_.cbuf + self.h_.prefix,
            self.h_.size - self.h_.prefix,
            s0,
            s1);
    }

    self.h_.cbuf   = self.h_.buf;
    self.h_.size   = new_size;
    self.h_.prefix = new_prefix_;
}

fields_base::
prefix_op_t::
~prefix_op_t()
{
    if(new_prefix_ < self_.h_.prefix)
    {
        std::memmove(
            self_.h_.buf + new_prefix_,
            self_.h_.cbuf + self_.h_.prefix,
            self_.h_.size - self_.h_.prefix);

        self_.h_.size =
            self_.h_.size - self_.h_.prefix + new_prefix_;
        self_.h_.prefix = new_prefix_;
    }
    else if(buf_)
    {
        delete[] buf_;
    }
}

//------------------------------------------------

fields_base::
fields_base(
    detail::kind k) noexcept
    : fields_view_base(&h_)
    , h_(k)
{
}

fields_base::
fields_base(
    detail::kind k,
    char* storage,
    std::size_t cap) noexcept
    : fields_base(
        *detail::header::get_default(k), storage, cap)
{
}

// copy s and parse it
fields_base::
fields_base(
    detail::kind k,
    core::string_view s)
    : fields_view_base(&h_)
    , h_(detail::empty{k})
{
    auto n = detail::header::count_crlf(s);
    if(h_.kind == detail::kind::fields)
    {
        if(n < 1)
            detail::throw_invalid_argument();
        n -= 1;
    }
    else
    {
        if(n < 2)
            detail::throw_invalid_argument();
        n -= 2;
    }
    op_t op(*this);
    op.grow(s.size(), n);
    s.copy(h_.buf, s.size());
    system::error_code ec;
    // VFALCO This is using defaults?
    header_limits lim;
    h_.parse(s.size(), lim, ec);
    if(ec.failed())
        detail::throw_system_error(ec);
}

// copy s and parse it
fields_base::
fields_base(
    detail::kind k,
    char* storage,
    std::size_t cap,
    core::string_view s)
    : fields_view_base(&h_)
    , h_(detail::empty{k})
    , external_storage_(true)
{
    h_.cbuf = storage;
    h_.buf = storage;
    h_.cap = align_down(
        storage,
        cap,
        alignof(detail::header::entry));
    max_cap_ = h_.cap;

    auto n = detail::header::count_crlf(s);
    if(h_.kind == detail::kind::fields)
    {
        if(n < 1)
            detail::throw_invalid_argument();
        n -= 1;
    }
    else
    {
        if(n < 2)
            detail::throw_invalid_argument();
        n -= 2;
    }

    if(detail::header::bytes_needed(
        s.size(), n)
            >= h_.cap)
        detail::throw_length_error();

    s.copy(h_.buf, s.size());
    system::error_code ec;
    // VFALCO This is using defaults?
    header_limits lim;
    h_.parse(s.size(), lim, ec);
    if(ec.failed())
        detail::throw_system_error(ec);
}

// construct a complete copy of h
fields_base::
fields_base(
    detail::header const& h)
    : fields_view_base(&h_)
    , h_(h.kind)
{
    if(h.is_default())
        return;

    // allocate and copy the buffer
    op_t op(*this);
    op.grow(h.size, h.count);
    h.assign_to(h_);
    std::memcpy(
        h_.buf, h.cbuf, h.size);
    h.copy_table(h_.buf + h_.cap);
}

// construct a complete copy of h
fields_base::
fields_base(
    detail::header const& h,
    char* storage,
    std::size_t cap)
    : fields_view_base(&h_)
    , h_(h.kind)
    , external_storage_(true)
{
    h_.cbuf = storage;
    h_.buf = storage;
    h_.cap = align_down(
        storage,
        cap,
        alignof(detail::header::entry));
    max_cap_ = h_.cap;

    if(detail::header::bytes_needed(
        h.size, h.count)
            >= h_.cap)
        detail::throw_length_error();

    h.assign_to(h_);
    std::memcpy(
        h_.buf, h.cbuf, h.size);
    h.copy_table(h_.buf + h_.cap);
}

//------------------------------------------------

fields_base::
~fields_base()
{
    if(h_.buf && !external_storage_)
        delete[] h_.buf;
}

//------------------------------------------------
//
// Capacity
//
//------------------------------------------------

void
fields_base::
clear() noexcept
{
    if(! h_.buf)
        return;
    using H =
        detail::header;
    auto const& h =
        *H::get_default(
            h_.kind);
    h.assign_to(h_);
    std::memcpy(
        h_.buf,
        h.cbuf,
        h_.size);
}

void
fields_base::
reserve_bytes(
    std::size_t n)
{
    op_t op(*this);
    if(! op.reserve(n))
        return;
    std::memcpy(
        h_.buf, op.cbuf(), h_.size);
    auto const nt =
        sizeof(entry) * h_.count;
    if(nt > 0)
        std::memcpy(
            h_.buf + h_.cap - nt,
            op.end() - nt,
            nt);
}

void
fields_base::
shrink_to_fit()
{
    if(detail::header::bytes_needed(
        h_.size, h_.count) >=
            h_.cap)
        return;

    if(external_storage_)
        return;

    fields_base tmp(h_);
    tmp.h_.swap(h_);
}


void
fields_base::
set_max_capacity_in_bytes(std::size_t n)
{
    if(n < h_.cap)
        detail::throw_invalid_argument();
    max_cap_ = n;
}

//------------------------------------------------
//
// Modifiers
//
//------------------------------------------------

auto
fields_base::
erase(
    iterator it) noexcept -> iterator
{
    auto const id = it->id.value_or(
        detail::header::unknown_field);
    raw_erase(it.i_);
    h_.on_erase(id);
    return it;
}

std::size_t
fields_base::
erase(
    field id) noexcept
{
    auto const i0 = h_.find(id);
    if(i0 == h_.count)
        return 0;
    return erase_all(i0, id);
}

std::size_t
fields_base::
erase(
    core::string_view name) noexcept
{
    auto const i0 = h_.find(name);
    if(i0 == h_.count)
        return 0;
    auto const ft = h_.tab();
    auto const id = ft[i0].id;
    if(id == detail::header::unknown_field)
        return erase_all(i0, name);
    return erase_all(i0, id);
}

//------------------------------------------------

void
fields_base::
set(
    iterator it,
    core::string_view value,
    system::error_code& ec)
{
    auto rv = verify_field_value(value);
    if(rv.has_error())
    {
        ec = rv.error();
        return;
    }

    value = rv->value;
    bool has_obs_fold = rv->has_obs_fold;

    auto const i = it.i_;
    auto tab = h_.tab();
    auto const& e0 = tab[i];
    auto const pos0 = offset(i);
    auto const pos1 = offset(i + 1);
    std::ptrdiff_t dn =
        value.size() -
        it->value.size();
    if( value.empty() &&
        ! it->value.empty())
        --dn; // remove SP
    else if(
        it->value.empty() &&
        ! value.empty())
        ++dn; // add SP

    op_t op(*this, &value);
    if( dn > 0 &&
        op.grow(value.size() -
            it->value.size(), 0))
    {
        // reallocated
        auto dest = h_.buf +
            pos0 + e0.nn + 1;
        std::memcpy(
            h_.buf,
            op.buf(),
            dest - h_.buf);
        if(! value.empty())
        {
            *dest++ = ' ';
            value.copy(
                dest,
                value.size());
            if( has_obs_fold )
                detail::remove_obs_fold(
                    dest, dest + value.size());
            dest += value.size();
        }
        *dest++ = '\r';
        *dest++ = '\n';
        std::memcpy(
            h_.buf + pos1 + dn,
            op.buf() + pos1,
            h_.size - pos1);
        std::memcpy(
            h_.buf + h_.cap -
                sizeof(entry) * h_.count,
            &op.tab()[h_.count - 1],
            sizeof(entry) * h_.count);
    }
    else
    {
        // copy the value first
        auto dest = h_.buf + pos0 +
            it->name.size() + 1;
        if(! value.empty())
        {
            *dest++ = ' ';
            value.copy(
                dest,
                value.size());
            if( has_obs_fold )
                detail::remove_obs_fold(
                    dest, dest + value.size());
            dest += value.size();
        }
        op.move_chars(
            h_.buf + pos1 + dn,
            h_.buf + pos1,
            h_.size - pos1);
        *dest++ = '\r';
        *dest++ = '\n';
    }
    {
        // update tab
        auto ft = h_.tab();
        for(std::size_t j = h_.count - 1;
                j > i; --j)
            ft[j] = ft[j] + dn;
        auto& e = ft[i];
        e.vp = e.np + e.nn +
            1 + ! value.empty();
        e.vn = static_cast<
            offset_type>(value.size());
        h_.size = static_cast<
            offset_type>(h_.size + dn);
    }
    auto const id = it->id.value_or(
        detail::header::unknown_field);
    if(h_.is_special(id))
    {
        // replace first char of name
        // with null to hide metadata
        char saved = h_.buf[pos0];
        auto& e = h_.tab()[i];
        e.id = detail::header::unknown_field;
        h_.buf[pos0] = '\0';
        h_.on_erase(id);
        h_.buf[pos0] = saved; // restore
        e.id = id;
        h_.on_insert(id, it->value);
    }
}

// erase existing fields with id
// and then add the field with value
void
fields_base::
set(
    field id,
    core::string_view value,
    system::error_code& ec)
{
    auto rv = verify_field_value(value);
    if(rv.has_error())
    {
        ec = rv.error();
        return;
    }

    auto const i0 = h_.find(id);
    if(i0 != h_.count)
    {
        // field exists
        auto const ft = h_.tab();
        {
            // provide strong guarantee
            auto const n0 =
                h_.size - length(i0);
            auto const n =
                ft[i0].nn + 2 +
                    rv->value.size() + 2;
            // VFALCO missing overflow check
            reserve_bytes(n0 + n);
        }
        erase_all(i0, id);
    }

    insert_unchecked(
        id,
        to_string(id),
        rv->value,
        h_.count,
        rv->has_obs_fold);
}

// erase existing fields with name
// and then add the field with value
void
fields_base::
set(
    core::string_view name,
    core::string_view value,
    system::error_code& ec)
{
    verify_field_name(name , ec);
    if(ec.failed())
        return;

    auto rv = verify_field_value(value);
    if(rv.has_error())
    {
        ec = rv.error();
        return;
    }

    auto const i0 = h_.find(name);
    if(i0 != h_.count)
    {
        // field exists
        auto const ft = h_.tab();
        auto const id = ft[i0].id;
        {
            // provide strong guarantee
            auto const n0 =
                h_.size - length(i0);
            auto const n =
                ft[i0].nn + 2 +
                    rv->value.size() + 2;
            // VFALCO missing overflow check
            reserve_bytes(n0 + n);
        }
        // VFALCO simple algorithm but
        // costs one extra memmove
        if(id != detail::header::unknown_field)
            erase_all(i0, id);
        else
            erase_all(i0, name);
    }
    insert_unchecked(
        string_to_field(name),
        name,
        rv->value,
        h_.count,
        rv->has_obs_fold);
}

//------------------------------------------------
//
// (implementation)
//
//------------------------------------------------

// copy start line and fields
void
fields_base::
copy_impl(
    detail::header const& h)
{
    BOOST_ASSERT(
        h.kind == ph_->kind);

    auto const n =
        detail::header::bytes_needed(
            h.size, h.count);
    if(n <= h_.cap && (!h.is_default() || external_storage_))
    {
        // no realloc
        h.assign_to(h_);
        h.copy_table(
            h_.buf + h_.cap);
        std::memcpy(
            h_.buf,
            h.cbuf,
            h.size);
        return;
    }

    // static storages cannot reallocate
    if(external_storage_)
        detail::throw_length_error();

    fields_base tmp(h);
    tmp.h_.swap(h_);
}

void
fields_base::
insert_impl(
    optional<field> id,
    core::string_view name,
    core::string_view value,
    std::size_t before,
    system::error_code& ec)
{
    verify_field_name(name, ec);
    if(ec.failed())
        return;

    auto rv = verify_field_value(value);
    if(rv.has_error())
    {
        ec = rv.error();
        return;
    }

    insert_unchecked(
        id,
        name,
        rv->value,
        before,
        rv->has_obs_fold);
}

void
fields_base::
insert_unchecked(
    optional<field> id,
    core::string_view name,
    core::string_view value,
    std::size_t before,
    bool has_obs_fold)
{
    auto const tab0 = h_.tab_();
    auto const pos = offset(before);
    auto const n =
        name.size() +       // name
        1 +                 // ':'
        ! value.empty() +   // [SP]
        value.size() +      // value
        2;                  // CRLF

    op_t op(*this, &name, &value);
    if(op.grow(n, 1))
    {
        // reallocated
        if(pos > 0)
            std::memcpy(
                h_.buf,
                op.cbuf(),
                pos);
        if(before > 0)
            std::memcpy(
                h_.tab_() - before,
                tab0 - before,
                before * sizeof(entry));
        std::memcpy(
            h_.buf + pos + n,
            op.cbuf() + pos,
            h_.size - pos);
    }
    else
    {
        op.move_chars(
            h_.buf + pos + n,
            h_.buf + pos,
            h_.size - pos);
    }

    // serialize
    {
        auto dest = h_.buf + pos;
        name.copy(dest, name.size());
        dest += name.size();
        *dest++ = ':';
        if(! value.empty())
        {
            *dest++ = ' ';
            value.copy(
                dest, value.size());
            if( has_obs_fold )
                detail::remove_obs_fold(
                    dest, dest + value.size());
            dest += value.size();
        }
        *dest++ = '\r';
        *dest = '\n';
    }

    // update table
    auto const tab = h_.tab_();
    {
        auto i = h_.count - before;
        if(i > 0)
        {
            auto p0 = tab0 - h_.count;
            auto p = tab - h_.count - 1;
            do
            {
                *p++ = *p0++ + n;
            }
            while(--i);
        }
    }
    auto& e = tab[0 - static_cast<std::ptrdiff_t>(before) - 1];
    e.np = static_cast<offset_type>(
        pos - h_.prefix);
    e.nn = static_cast<
        offset_type>(name.size());
    e.vp = static_cast<offset_type>(
        pos - h_.prefix +
            name.size() + 1 +
            ! value.empty());
    e.vn = static_cast<
        offset_type>(value.size());
    e.id = id.value_or(
        detail::header::unknown_field);

    // update container
    h_.count++;
    h_.size = static_cast<
        offset_type>(h_.size + n);
    h_.on_insert(e.id, value);
}

void
fields_base::
raw_erase(
    std::size_t i) noexcept
{
    BOOST_ASSERT(i < h_.count);
    BOOST_ASSERT(h_.buf != nullptr);
    auto const p0 = offset(i);
    auto const p1 = offset(i + 1);
    std::memmove(
        h_.buf + p0,
        h_.buf + p1,
        h_.size - p1);
    auto const n = p1 - p0;
    --h_.count;
    auto ft = h_.tab();
    for(;i < h_.count; ++i)
        ft[i] = ft[i + 1] - n;
    h_.size = static_cast<
        offset_type>(h_.size - n);
}

// erase n fields matching id
// without updating metadata
void
fields_base::
raw_erase_n(
    field id,
    std::size_t n) noexcept
{
    // iterate in reverse
    auto e = &h_.tab()[h_.count];
    auto const e0 = &h_.tab()[0];
    while(n > 0)
    {
        BOOST_ASSERT(e != e0);
        ++e; // decrement
        if(e->id == id)
        {
            raw_erase(e0 - e);
            --n;
        }
    }
}

// erase all fields with id
// and update metadata
std::size_t
fields_base::
erase_all(
    std::size_t i0,
    field id) noexcept
{
    BOOST_ASSERT(
        id != detail::header::unknown_field);
    std::size_t n = 1;
    std::size_t i = h_.count - 1;
    auto const ft = h_.tab();
    while(i > i0)
    {
        if(ft[i].id == id)
        {
            raw_erase(i);
            ++n;
        }
        // go backwards to
        // reduce memmoves
        --i;
    }
    raw_erase(i0);
    h_.on_erase_all(id);
    return n;
}

// erase all fields with name
// when id == detail::header::unknown_field
std::size_t
fields_base::
erase_all(
    std::size_t i0,
    core::string_view name) noexcept
{
    std::size_t n = 1;
    std::size_t i = h_.count - 1;
    auto const ft = h_.tab();
    auto const* p = h_.cbuf + h_.prefix;
    while(i > i0)
    {
        core::string_view s(
            p + ft[i].np, ft[i].nn);
        if(s == name)
        {
            raw_erase(i);
            ++n;
        }
        // go backwards to
        // reduce memmoves
        --i;
    }
    raw_erase(i0);
    return n;
}

// return i-th field absolute offset
std::size_t
fields_base::
offset(
    std::size_t i) const noexcept
{
    if(i == 0)
        return h_.prefix;
    if(i < h_.count)
        return h_.prefix + h_.tab()[i].np;
    // make final CRLF the last "field"
    return h_.size - 2;
}

// return i-th field absolute length
std::size_t
fields_base::
length(
    std::size_t i) const noexcept
{
    return
        offset(i + 1) -
        offset(i);
}

} // http_proto
} // boost

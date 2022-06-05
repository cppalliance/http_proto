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
#include <boost/http_proto/fields_view.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <string>

namespace boost {
namespace http_proto {

fields::
fields() noexcept
    : fields_base(detail::kind::fields)
{
}

fields::
fields(
    fields&& other) noexcept
    : fields_base(other.h_.kind)
{
    this->swap(other);
}

fields::
fields(
    fields const& f)
    : fields_base(f,
        detail::kind::fields)
{
}

// copy without start-line
fields::
fields(
    fields_view_base const& f)
    : fields_base(
    [&f]
    {
        detail::header h(
            detail::kind::fields);
        if(f.h_.count > 0)
        {
            // copy fields
            auto n = detail::buffer_needed(
                f.h_.size - f.h_.prefix,
                    f.h_.count);
            auto buf = new char[n];
            std::memcpy(
                buf,
                f.h_.cbuf + f.h_.prefix,
                f.h_.size - f.h_.prefix);
            f.write_table(buf + n);
            h.cbuf = buf;
            h.cap = n;
            h.prefix = 0;
            h.size =
                f.h_.size - f.h_.prefix;
            h.count = f.h_.count;
            h.buf = buf;
            return h;
        }

        // default buffer
        auto const s = default_buffer(
            detail::kind::fields);
        h.cbuf = s.data();
        h.cap = 0;
        h.prefix = static_cast<
            off_t>(s.size() - 2);
        h.size = h.prefix + 2;
        h.count = 0;
        h.buf = nullptr;
        return h;       
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
    fields tmp(f);
    tmp.swap(*this);
    return *this;
}

// copy fields in f
// without start-line
fields&
fields::
operator=(
    fields_view const& f)
{
    BOOST_ASSERT(h_.kind ==
        detail::kind::fields);
    if(is_default(f.h_.cbuf))
    {
        fields tmp;
        tmp.swap(*this);
        return *this;
    }
    auto const n0 =
        f.h_.size -
        f.h_.prefix;
    auto const n =
        detail::buffer_needed(
            n0, f.h_.count);
    if(h_.cap < n)
    {
        // copy with strong
        // exception safety
        fields tmp(f);
        tmp.swap(*this);
        return *this;
    }
    // use existing capacity
    std::memcpy(
        h_.buf,
        f.h_.cbuf +
            f.h_.prefix,
        n0);
    f.write_table(
        h_.buf + h_.cap);
    h_.prefix = 0;
    h_.size = static_cast<
        off_t>(n0);
    h_.count = f.h_.count;
    return *this;
}

} // http_proto
} // boost

#endif

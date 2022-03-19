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
#include <boost/http_proto/detail/copied_strings.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <boost/assert/source_location.hpp>
#include <string>

namespace boost {
namespace http_proto {

fields::
fields() noexcept
    : fields_base(0)
{
}

fields::
fields(
    fields&& other) noexcept
    : fields_base(other.kind_)
{
    this->swap(other);
}

fields::
fields(
    fields const& f)
    : fields_base(f, 0)
{
}

// copy without start-line
fields::
fields(
    fields_view_base const& f)
    : fields_base(
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

} // http_proto
} // boost

#endif

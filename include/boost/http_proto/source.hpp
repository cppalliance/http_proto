//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SOURCE_HPP
#define BOOST_HTTP_PROTO_SOURCE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

/** A source of buffers containing data
*/
struct source
{
    virtual
    ~source() = 0;

    virtual
    bool
    more() const noexcept = 0;

    virtual
    void
    prepare(
        const_buffers& cb,
        error_code& ec) = 0;

    virtual
    void
    consume(std::size_t n) noexcept = 0;
};

//------------------------------------------------

class string_view_source
    : public source
{
    const_buffer cb_;

public:
    explicit
    string_view_source(
        string_view s) noexcept
        : cb_(s.data(), s.size())
    {
    }

    bool
    more() const noexcept override
    {
        return cb_.size() > 0;
    }

    void
    prepare(
        const_buffers& cb,
        error_code& ec) override
    {
        ec = {};
        cb = { &cb_, 1 };
    }

    void
    consume(std::size_t n) noexcept override
    {
        cb_ += n;
    }
};

} // http_proto
} // boost

#endif

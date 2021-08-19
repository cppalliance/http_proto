//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_HEADERS_VIEW_HPP
#define BOOST_HTTP_PROTO_HEADERS_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

class headers_view
{
    using size_type = std::uint16_t;

public:
    class iterator;
    using const_iterator = iterator;

    class value_type;

    headers_view(headers_view const&) = default;
    headers_view& operator=(
        headers_view const&) = default;

    std::size_t
    size() const noexcept;

    bool
    empty() const noexcept
    {
        return size() == 0;
    }

    BOOST_HTTP_PROTO_DECL
    const_iterator
    begin() const noexcept;

    BOOST_HTTP_PROTO_DECL
    const_iterator
    end() const noexcept;
};

//------------------------------------------------

class headers_view::value_type
{
    friend class headers_view;

public:
    string_view
    field_str() const noexcept;

    http_proto::field
    field() const noexcept;

    string_view const
    value() const noexcept;

    value_type const*
    operator->() const noexcept
    {
        return this;
    }
};

} // http_proto
} // boost

#include <boost/http_proto/impl/headers_view.hpp>

#endif

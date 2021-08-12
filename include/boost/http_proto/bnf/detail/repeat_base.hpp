//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BNF_DETAIL_REPEAT_BASE_HPP
#define BOOST_HTTP_PROTO_BNF_DETAIL_REPEAT_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/bnf/any_element.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {
namespace bnf {
namespace detail {

class repeat_base
    : public any_element
{
protected:
    std::size_t n_,
    std::size_t m_,

    BOOST_HTTP_PROTO_DECL
    char const*
    begin(
        char const* start,
        char const* end,
        error_code& ec);

    BOOST_HTTP_PROTO_DECL
    char const*
    increment(
        char const* start,
        char const* end,
        error_code& ec);
};

} // detail
} // bnf
} // http_proto
} // boost

#endif

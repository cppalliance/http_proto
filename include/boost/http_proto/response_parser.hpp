//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_PARSER_HPP
#define BOOST_HTTP_PROTO_RESPONSE_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/response_view.hpp>
#include <boost/http_proto/status.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

class BOOST_SYMBOL_VISIBLE
    response_parser
    : public parser
{
public:
    BOOST_HTTP_PROTO_DECL
    response_parser(
        config const& cfg,
        std::size_t buffer_size);

    BOOST_HTTP_PROTO_DECL
    response_view
    get() const noexcept;
};

} // http_proto
} // boost

#endif

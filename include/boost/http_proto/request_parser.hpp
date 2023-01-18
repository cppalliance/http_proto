//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_PARSER_HPP
#define BOOST_HTTP_PROTO_REQUEST_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/method.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/request_view.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstddef>
#include <utility>

namespace boost {
namespace http_proto {

class BOOST_SYMBOL_VISIBLE
    request_parser
    : public parser
{
public:
    /** Configuration settings for parsing requests
    */
    struct config : config_base
    {
        config() noexcept
        {
            max_body_size = 64 * 1024;
        }
    };

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    request_parser();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    request_parser(
        std::size_t buffer_size);

    /** Constructor
    */
    template<class P0, class... Pn>
    request_parser(
        std::size_t buffer_size,
        P0&& p0,
        Pn&&... pn)
        : request_parser(buffer_size)
    {
        this->apply_params(
            std::forward<P0>(p0),
            std::forward<Pn>(pn)...);
    }

    /** Prepare for the next message on the stream.
    */
    void
    start()
    {
        start_impl();
    }

    /** Prepare for the next message on the stream.
    */
    template<class P>
    void
    start(P&& p)
    {
        apply_start(
            std::forward<P>(p));
        start_impl();
    }

    /** Return the parsed request headers.
    */
    BOOST_HTTP_PROTO_DECL
    request_view
    get() const noexcept;
};

} // http_proto
} // boost

#endif

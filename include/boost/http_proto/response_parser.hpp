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

#ifdef BOOST_HTTP_PROTO_DOCS

/** Constant for indicating a HEAD response

    This value may be passed to the
    @ref response::start function.
*/
constexpr __implementation_defined__ head_response;

#else
struct head_response_t{};
constexpr head_response_t head_response{};
#endif

class BOOST_SYMBOL_VISIBLE
    response_parser
    : public parser
{
public:
    /** Configuration settings for parsing requests
    */
    struct config : config_base
    {
        config() noexcept
        {
            max_body_size = 1024 * 1024;
        }
    };
    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    response_parser();

    /** Constructor
    */
    BOOST_HTTP_PROTO_DECL
    explicit
    response_parser(
        std::size_t buffer_size);

    /** Constructor
    */
    template<class P0, class... Pn>
    response_parser(
        std::size_t buffer_size,
        P0&& p0,
        Pn&&... pn)
        : response_parser(buffer_size)
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

    /** Return the parsed response headers.
    */
    BOOST_HTTP_PROTO_DECL
    response_view
    get() const noexcept;

private:
    friend class parser;

    template<class P>
    void apply_start(P&& p)
    {
        parser::apply_start(
            std::forward<P>(p));
    }

    BOOST_HTTP_PROTO_DECL void apply_start(head_response_t);
};

} // http_proto
} // boost

#endif

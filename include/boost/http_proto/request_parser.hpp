//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_REQUEST_PARSER_HPP
#define BOOST_HTTP_PROTO_REQUEST_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/basic_parser.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstddef>

namespace boost {
namespace http_proto {

enum class what
{
    none,
    header,
    body,
    chunk_header,
    chunk_body,
    chunk_final,
    end
};

class request_parser
    : public basic_parser
{
public:
#if 0
    /** Return the type of the current structured element.
    */
    beast2::what
    what() const noexcept;

    /** Return true if more input buffer data is needed.
    */
    bool
    need_more() const noexcept;

    /** Reserve space in the input buffer.
    */
    void
    reserve(std::size_t n);

    /** Attempt to advance the state of the parser to the next structured element.

        @return `false` if more data is needed.
    */
    bool
    next(error_code& ec);

    /** Return the request header.
    */
    request_header
    header();

    /** Return the chunk header.
    */
    chunk_header
    chunk_header();

    template<class Body>
    void
    attach_body();
#endif

private:
    void
    parse_start_line(
        char const*& in, char const* last,
        error_code& ec) override;

    static
    void
    parse_method(
        char const*& it, char const* last,
        string_view& result, error_code& ec);

    static
    void
    parse_target(
        char const*& it, char const* last,
        string_view& result, error_code& ec);

    void
    finish_header(
        error_code& ec) override;
};

#if 0
struct basic_body
{
    virtual void content_length(std::uint64_t n) = 0;
};

class string_body
{
    std::string s_;
    std::size_t n_ = 0;

public:
    void content_length(std::uint64_t n)
    {
        //if(n > std::string::max_size())
            // throw
        s_.resize(static_cast<
            std::size_t>(n));
        n_ = 0;
    }

    void write(net::const_buffer b)
    {
        n_ += net::buffer_copy(
            net::mutable_buffer(
                &s_[0] + n_,
                s_.size() - n_),
            b);
    }
};
#endif

} // http_proto
} // boost

#endif

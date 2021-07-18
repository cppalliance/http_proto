//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_BASIC_PARSER_HPP
#define BOOST_HTTP_PROTO_BASIC_PARSER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <cstddef>
#include <cstdint>

namespace boost {
namespace http_proto {

class basic_parser
{
    enum class state;

    char* buffer_;
    std::size_t capacity_;          // allocated size
    std::size_t size_;              // valid part of buffer

    state state_;
    net::mutable_buffer mb_;
    std::uint32_t header_limit_;    // max header size
    std::size_t skip_;              // offset to continue parse

protected:
    static unsigned constexpr flagSkipBody              = 1<<  0;
    static unsigned constexpr flagEager                 = 1<<  1;
    static unsigned constexpr flagGotSome               = 1<<  2;
    static unsigned constexpr flagHasBody               = 1<<  3;
    static unsigned constexpr flagHTTP11                = 1<<  4;
    static unsigned constexpr flagNeedEOF               = 1<<  5;
    static unsigned constexpr flagExpectCRLF            = 1<<  6;
    static unsigned constexpr flagConnectionClose       = 1<<  7;
    static unsigned constexpr flagConnectionUpgrade     = 1<<  8;
    static unsigned constexpr flagConnectionKeepAlive   = 1<<  9;
    static unsigned constexpr flagContentLength         = 1<< 10;
    static unsigned constexpr flagChunked               = 1<< 11;
    static unsigned constexpr flagUpgrade               = 1<< 12;
    static unsigned constexpr flagFinalChunk            = 1<< 13;

    unsigned f_;                    // flags

    static bool is_digit(char) noexcept;
    static bool is_print(char) noexcept;

public:
    ~basic_parser();

    basic_parser() noexcept;

    bool
    need_more() const noexcept;

    mutable_buffers
    prepare();

    std::size_t
    commit(
        std::size_t n,
        error_code& ec);

    // temp stuff
    bool is_done() const noexcept
    {
        return false;
    }

private:
    void
    maybe_need_more(
        char const* p,
        std::size_t n,
        error_code& ec) noexcept;

    static
    char const*
    find_eom(
        char const* p,
        char const* last) noexcept;

    void
    parse_fields(
        char const*& p,
        char const* last,
        error_code& ec);

protected:
    virtual
    void
    parse_start_line(
        char const*& in,
        char const* last,
        error_code& ec) = 0;

    virtual
    void
    finish_header(
        error_code& ec) = 0;

    static
    void
    parse_version(
        char const*& it, char const* last,
        int& result, error_code& ec) noexcept;
};

} // http_proto
} // boost

#endif

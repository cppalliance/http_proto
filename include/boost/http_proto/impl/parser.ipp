//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_PARSER_IPP
#define BOOST_HTTP_PROTO_IMPL_PARSER_IPP

#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/service/zlib_service.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/assert.hpp>
#include <boost/none.hpp>
#include <memory>

namespace boost {
namespace http_proto {

//------------------------------------------------
/*
    Four body styles for `parser`
        * Specify a DynamicBuffer
        * Specify a Sink
        * Read from a parser::stream
        * in-place

Buffer Usage

|                                               | begin
| H |   p   |                               | f | read headers
| H |   p   |                           | T | f | set T body
| H |   p   |                       | C | T | f | make codec C
| H |   p           |       b       | C | T | f | decode p into b
| H |       p       |       b       | C | T | f | read/parse loop
| H |                                   | T | f | destroy codec
| H |                                   | T | f | finished

H   headers
C   codec
T   body
f   table
p   partial payload
b   body data

- We can compact the headers:
    move the table downwards to
    squeeze out the unused space

*/
//------------------------------------------------

struct parser_service
    : service
{
    parser::config_base cfg;
    std::size_t space_needed = 0;
    zlib::deflate_decoder_service const*
        deflate_svc = nullptr;

    parser_service(
        context& ctx,
        parser::config_base const& cfg_);
};

parser_service::
parser_service(
    context& ctx,
    parser::config_base const& cfg_)
        : cfg(cfg_)
{
/*
    | fb |      cb0      |      cb1     | C | T | f |

    fb  flat_buffer         headers.max_size
    cb0 circular_buffer     min_in_place_body
    cb1 circular_buffer     min_in_place_body
    C   codec               max_codec
    T   body                max_type_erase
    f   table               max_table_space

*/
    // validate
    //if(cfg.min_prepare > cfg.max_prepare)
        //detail::throw_invalid_argument();

    // fb, f
    space_needed +=
        cfg.headers.valid_space_needed();

    // cb0, cb1
    space_needed +=
        2 * cfg.min_in_place_body;

    // T
    space_needed += cfg.max_type_erase;

    // max(C...)
    std::size_t C = 0;
    {
        if(cfg.apply_deflate_decoder)
        {
            deflate_svc = &ctx.get_service<
                zlib::deflate_decoder_service>();
            auto const n = 
                deflate_svc->space_needed();
            if( C < n)
                C = n;
        }
    }
    space_needed += C;
}

void
install_parser_service(
    context& ctx,
    parser::config_base const& cfg)
{
    ctx.make_service<
        parser_service>(cfg);
}

//------------------------------------------------

parser::
parser(
    context& ctx,
    detail::kind k)
    : ctx_(ctx)
    , svc_(ctx.get_service<
        parser_service>())
    , h_(detail::empty{k})
{
    auto const n =
        svc_.space_needed;
    ws_.allocate(n);
    h_.cap = n;
    reset();
}

//------------------------------------------------
//
// Special Members
//
//------------------------------------------------

parser::
~parser()
{
}

parser::
parser(
    parser&&) noexcept = default;

//------------------------------------------------
//
// Modifiers
//
//------------------------------------------------

// prepare for a new stream
void
parser::
reset() noexcept
{
    ws_.clear();
    st_ = state::need_start;
    body_ = body::in_place;
    head_response_ = false;
    got_eof_ = false;
}

void
parser::
start_impl(
    bool head_response)
{
    std::size_t leftover = 0;
    switch(st_)
    {
    default:
    case state::need_start:
        BOOST_ASSERT(h_.size == 0);
        BOOST_ASSERT(fb_.size() == 0);
        BOOST_ASSERT(! got_eof_);
        break;

    case state::headers:
        // can't call start() twice
        detail::throw_logic_error();

    case state::headers_done:
    case state::body:
        // previous message was unfinished
        detail::throw_logic_error();

    case state::complete:
        if(fb_.size() > 0)
        {
            // headers with no body
            BOOST_ASSERT(h_.size > 0);
            fb_.consume(h_.size);
            leftover = fb_.size();
            // move unused octets to front
            buffers::buffer_copy(
                buffers::mutable_buffer(
                    ws_.data(),
                    leftover),
                fb_.data());
        }
        else
        {
            // leftover data after body
        }
        break;
    }
/*
| fb |      cb0      |      cb1     | C | T | f |
*/
    // start with fb+cb0
    fb_ = {
        ws_.data(), // VFALCO this might need an offset
        svc_.cfg.headers.max_size +
            svc_.cfg.min_in_place_body,
        leftover };

    h_ = detail::header(
        detail::empty{h_.kind});
    h_.buf = reinterpret_cast<
        char*>(ws_.data());
    h_.cbuf = h_.buf;
    h_.cap = ws_.size();

    st_ = state::headers;
    body_ = body::in_place;

    BOOST_ASSERT(! head_response ||
        h_.kind == detail::kind::response);
    head_response_ = head_response;
}

auto
parser::
prepare() ->
    mutable_buffers_type
{
    switch(st_)
    {
    default:
    case state::need_start:
        // forgot to call start()
        detail::throw_logic_error();

    case state::headers:
    {
        BOOST_ASSERT(h_.size <
            svc_.cfg.headers.max_size);
        auto n = fb_.capacity() - fb_.size();
        if( n > svc_.cfg.max_prepare)
            n = svc_.cfg.max_prepare;
        return {
            fb_.prepare(n),
            buffers::mutable_buffer{} };
    }

    case state::headers_done:
    {
        // reserve headers
        ws_.reserve_front(h_.size);

        // reserve table at the back
        ws_.reserve_back(h_.table_space());

        st_ = state::body;
        // VFALCO set up body buffer
        BOOST_FALLTHROUGH;
    }

    case state::body:
    {
        //if(body_ == body::dynamic)
        auto n = cb0_.capacity() -
            cb0_.size();
        if( n > svc_.cfg.max_prepare)
            n = svc_.cfg.max_prepare;
        return cb0_.prepare(n);
    }

    case state::complete:
        // forgot to call start()
        detail::throw_logic_error();
    }
}

void
parser::
commit(
    std::size_t n)
{
    // Can't commit after eof
    if(got_eof_)
        detail::throw_logic_error();

    switch(st_)
    {
    default:
    case state::need_start:
        // forgot to call start()
        detail::throw_logic_error();

    case state::headers:
        fb_.commit(n);
        break;

    case state::headers_done:
        // forgot to call prepare()
        detail::throw_logic_error();

    case state::body:
        cb0_.commit(n);
        break;

    case state::complete:
        // forgot to call start()
        detail::throw_logic_error();
    }
}

void
parser::
commit_eof()
{
    switch(st_)
    {
    default:
    case state::need_start:
        // forgot to call prepare()
        detail::throw_logic_error();

    case state::headers:
        got_eof_ = true;
        break;

    case state::headers_done:
        // forgot to call prepare()
        detail::throw_logic_error();

    case state::body:
        got_eof_ = true;
        break;

    case state::complete:
        // Can't commit eof when
        // message is complete.
        detail::throw_logic_error();
    }
}

//-----------------------------------------------

// process input data then
// eof if input data runs out.
void
parser::
parse(
    error_code& ec)
{
    switch(st_)
    {
    default:
    case state::need_start:
        // forgot to call start()
        detail::throw_logic_error();

    case state::headers:
    {
        BOOST_ASSERT(h_.buf == ws_.data());
        BOOST_ASSERT(h_.cbuf == ws_.data());
        auto const new_size = fb_.size();
        h_.parse(new_size, svc_.cfg.headers, ec);
        if(! ec.failed())
        {
            if( h_.md.payload != payload::none &&
                ! head_response_)
            {
                if(h_.md.payload == payload::size)
                    remain_ = h_.md.payload_size;

                // Deliver headers to caller
                st_ = state::headers_done;
                break;
            }
            // no payload
            st_ = state::complete;
            break;
        }
        if(ec == grammar::error::need_more)
        {
            if(! got_eof_)
                break;
            if(h_.size > 0)
            {
                // Connection closed before
                // message is complete.
                ec = BOOST_HTTP_PROTO_ERR(
                    error::incomplete);
                return;
            }

            // Connection closed
            // cleanly.
            ec = BOOST_HTTP_PROTO_ERR(
                error::end_of_stream);
            return;
        }
        BOOST_ASSERT(ec.failed());
        return;
    }

    case state::headers_done:
    {
        // This is a no-op
        // VFALCO Is this right?
        ec = {};
        break;
    }

    case state::body:
    {
        parse_body(ec);
        if(ec.failed())
            return;
        st_ = state::complete;
        break;
    }

    case state::complete:
        break;
    }
}

//------------------------------------------------

auto
parser::
get_stream() ->
    stream
{
    // body type already chosen
    if(body_ != body::in_place)
        detail::throw_logic_error();

    body_ = body::stream;
    return stream(*this);
}

//------------------------------------------------

string_view
parser::
release_buffered_data() noexcept
{
    return {};
}

//------------------------------------------------
//
// Implementation
//
//------------------------------------------------

auto
parser::
safe_get_header() const ->
    detail::header const*
{
    switch(st_)
    {
    default:
    case state::need_start:
    case state::headers:
        // Headers not received yet
        detail::throw_logic_error();

    case state::headers_done:
        break;

    case state::body:
        // Headers received and discarded
        detail::throw_logic_error();

    case state::complete:
        // VFALCO Could be OK
        break;
    }
    return &h_;
}

void
parser::
on_set_body()
{
}

void
parser::
parse_body(
    error_code& ec)
{
    if(body_ == body::in_place)
    {
        ec = {};
        return;
    }
    if(body_ == body::dynamic)
    {
        ec = {};
        return;
    }
    if(body_ == body::sink)
    {
        ec = {};
        return;
    }
    if(body_ == body::stream)
    {
        ec = {};
        return;
    }
}

void
parser::
parse_chunk(
    error_code& ec)
{
    ec = {};
}

} // http_proto
} // boost

#endif

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
#include <memory>

namespace boost {
namespace http_proto {

//------------------------------------------------
/*
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

    A "plain payload" has no actionable transfer
        encoding.

    A "buffered payload" is any payload which is
        not plain. A second buffer is required
        for reading.
*/
//------------------------------------------------

class parser_service
    : public service
{
public:
    parser::config_base cfg;
    std::size_t space_needed = 0;
    std::size_t max_codec = 0;
    zlib::deflate_decoder_service const*
        deflate_svc = nullptr;

    parser_service(
        context& ctx,
        parser::config_base const& cfg_);

    std::size_t
    max_overread() const noexcept
    {
        return
            cfg.headers.max_size +
            cfg.min_buffer;
    }
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
    cb0 circular_buffer     min_buffer
    cb1 circular_buffer     min_buffer
    C   codec               max_codec
    T   body                max_type_erase
    f   table               max_table_space

*/
    // validate
    //if(cfg.min_prepare > cfg.max_prepare)
        //detail::throw_invalid_argument();

    if( cfg.min_buffer < 1 ||
        cfg.min_buffer > cfg.body_limit)
        detail::throw_invalid_argument();

    if(cfg.max_prepare < 1)
        detail::throw_invalid_argument();

    // VFALCO TODO OVERFLOW CHECING
    {
        //fb_.size() - h_.size +
        //svc_.cfg.min_buffer +
        //svc_.cfg.min_buffer +
        //svc_.max_codec;
    }

    // VFALCO OVERFLOW CHECKING ON THIS
    space_needed +=
        cfg.headers.valid_space_needed();

    // cb0_, cb1_
    // VFALCO OVERFLOW CHECKING ON THIS
    space_needed +=
        cfg.min_buffer +
        cfg.min_buffer;

    // T
    space_needed += cfg.max_type_erase;

    // max_codec
    {
        if(cfg.apply_deflate_decoder)
        {
            deflate_svc = &ctx.get_service<
                zlib::deflate_decoder_service>();
            auto const n = 
                deflate_svc->space_needed();
            if( max_codec < n)
                max_codec = n;
        }
    }
    space_needed += max_codec;
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
    , st_(state::reset)
{
    auto const n =
        svc_.space_needed;
    ws_.allocate(n);
    h_.cap = n;
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
    st_ = state::start;
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
    case state::reset:
        // reset must be called first
        detail::throw_logic_error();

    case state::start:
        BOOST_ASSERT(h_.size == 0);
        BOOST_ASSERT(fb_.size() == 0);
        BOOST_ASSERT(! got_eof_);
        break;

    case state::header:
        // start() called twice
        detail::throw_logic_error();

    case state::body:
        // current message is incomplete
        detail::throw_logic_error();

    case state::complete:
    {
        // overread data is always in cb0_

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
    }

    ws_.clear();

    fb_ = {
        ws_.data(),
        svc_.cfg.headers.max_size +
            svc_.cfg.min_buffer,
        leftover };
    BOOST_ASSERT(
        fb_.capacity() == svc_.max_overread());

    h_ = detail::header(
        detail::empty{h_.kind});
    h_.buf = reinterpret_cast<
        char*>(ws_.data());
    h_.cbuf = h_.buf;
    h_.cap = ws_.size();

    BOOST_ASSERT(! head_response ||
        h_.kind == detail::kind::response);
    head_response_ = head_response;

    body_ = body::in_place;
    st_ = state::header;
}

auto
parser::
prepare() ->
    mutable_buffers_type
{
    switch(st_)
    {
    default:
    case state::reset:
        // reset must be called first
        detail::throw_logic_error();

    case state::start:
        // start must be called first
        detail::throw_logic_error();

    case state::header:
    {
        BOOST_ASSERT(h_.size <
            svc_.cfg.headers.max_size);
        auto n = fb_.capacity() - fb_.size();
        BOOST_ASSERT(n <= svc_.max_overread());
        if( n > svc_.cfg.max_prepare)
            n = svc_.cfg.max_prepare;
        mbp_[0] = fb_.prepare(n);
        return mutable_buffers_type(
            &mbp_[0], 1);
    }

    case state::body:
    {
        // buffered payload
        if(body_buf_ != &cb0_)
        {
            auto n = cb0_.capacity() -
                cb0_.size();
            if( n > svc_.cfg.max_prepare)
                n = svc_.cfg.max_prepare;
            mbp_ = cb0_.prepare(n);
            return mutable_buffers_type(mbp_);
        }

        // plain payload

        if( body_ == body::in_place ||
            body_ == body::sink)
        {
            auto n = cb0_.capacity() -
                cb0_.size();
            if( n > svc_.cfg.max_prepare)
                n = svc_.cfg.max_prepare;
            mbp_ = cb0_.prepare(n);
            BOOST_ASSERT(mbp_[1].size() == 0);
            return mutable_buffers_type(
                &mbp_[0], 1);
        }

        if(body_ == body::dynamic)
        {
            BOOST_ASSERT(dyn_ != nullptr);
            if(h_.md.payload == payload::size)
            {
                // exact size
                std::size_t n =
                    h_.md.payload_size - dyn_->size();
                if( n > svc_.cfg.max_prepare)
                    n = svc_.cfg.max_prepare;
                return dyn_->prepare(n);
            }

            // heuristic size
            std::size_t n = svc_.cfg.min_buffer;
            if( n > svc_.cfg.max_prepare)
                n = svc_.cfg.max_prepare;
            return dyn_->prepare(n);
        }

        // VFALCO TODO
        if(body_ == body::stream)
            detail::throw_logic_error();

        // VFALCO TODO
        detail::throw_logic_error();
    }

    case state::complete:
        // We allow the call for callers
        // who want normalized usage, but
        // just return a 0-sized sequence.
        return mutable_buffers_type{};
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
    case state::reset:
        // reset must be called first
        detail::throw_logic_error();

    case state::start:
        // forgot to call start()
        detail::throw_logic_error();

    case state::header:
        fb_.commit(n);
        break;

    case state::body:
        if(body_buf_ != &cb0_)
        {
            // buffered body
            cb0_.commit(n);
            break;
        }
        if(body_ == body::in_place)
        {
            cb0_.commit(n);
            break;
        }
        if(body_ == body::dynamic)
        {
            dyn_->commit(n);
            break;
        }
        if(body_ == body::sink)
        {
            cb0_.commit(n);
            break;
        }
        if(body_ == body::stream)
        {
            // VFALCO TODO
            detail::throw_logic_error();
        }
        break;

    case state::complete:
        // intended no-op
        break;
    }
}

void
parser::
commit_eof()
{
    switch(st_)
    {
    default:
    case state::reset:
        // reset must be called first
        detail::throw_logic_error();

    case state::start:
        // forgot to call prepare()
        detail::throw_logic_error();

    case state::header:
        got_eof_ = true;
        break;

    case state::body:
        got_eof_ = true;
        break;

    case state::complete:
        // can't commit eof when complete
        detail::throw_logic_error();
    }
}

//-----------------------------------------------

// process input data then
// eof if input data runs out.
void
parser::
parse(
    system::error_code& ec)
{
    ec = {};
    switch(st_)
    {
    default:
    case state::reset:
        // reset must be called first
        detail::throw_logic_error();

    case state::start:
        // start must be called first
        detail::throw_logic_error();

    case state::header:
    {
        BOOST_ASSERT(h_.buf == static_cast<
            void const*>(ws_.data()));
        BOOST_ASSERT(h_.cbuf == static_cast<
            void const*>(ws_.data()));
        auto const new_size = fb_.size();
        h_.parse(new_size, svc_.cfg.headers, ec);
        if(ec == condition::need_more_input)
        {
            if(! got_eof_)
            {
                // headers incomplete
                return;
            }
            if(h_.size == 0)
            {
                // stream closed cleanly
                ec = BOOST_HTTP_PROTO_ERR(
                    error::end_of_stream);
                return;
            }

            // stream closed with a
            // partial message received
            ec = BOOST_HTTP_PROTO_ERR(
                error::incomplete);
            return;
        }
        if(ec.failed())
        {
            // other error,
            //
            // VFALCO map this to a bad
            // request or bad response error?
            //
            st_ = state::reset; // unrecoverable
            return;
        }

        // headers are complete
        on_headers(ec);
        if(ec.failed())
            return;
        if(st_ == state::complete)
            break;
        BOOST_ASSERT(st_ == state::body);
        BOOST_FALLTHROUGH;
    }

    case state::body:
    {
        if(body_ == body::in_place)
        {
            if(filt_)
            {
                // apply filter
                detail::throw_logic_error();
            }
            if(h_.md.payload == payload::size)
            {
                if(cb0_.size() <
                    h_.md.payload_size)
                {
                    ec = BOOST_HTTP_PROTO_ERR(
                        error::need_data);
                    return;
                }
                st_ = state::complete;
                break;
            }
            BOOST_ASSERT(h_.md.payload ==
                payload::to_eof);
            if(! got_eof_)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::need_data);
                return;
            }
            st_ = state::complete;
            break;
        }

        // VFALCO TODO
        detail::throw_logic_error();
    }

    case state::complete:
    {
        // This is a no-op except when set_body
        // was called and we have in-place data.
        switch(body_)
        {
        default:
        case body::in_place:
            break;

        case body::dynamic:
        {
            if(body_buf_->size() == 0)
                break;
            BOOST_ASSERT(dyn_->size() == 0);
            auto n = buffers::buffer_copy(
                dyn_->prepare(
                    body_buf_->size()),
                body_buf_->data());
            body_buf_->consume(n);
            break;
        }

        case body::sink:
        {
            if(body_buf_->size() == 0)
                break;
            auto rv = sink_->write(
                body_buf_->data(), false);
            body_buf_->consume(rv.bytes);
            if(rv.ec.failed())
            {
                ec = rv.ec;
                st_ = state::reset; // unrecoverable
                return;
            }
            break;
        }

        case body::stream:
            // VFALCO TODO
            detail::throw_logic_error();
        }
    }
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

string_view
parser::
in_place_body() const
{
    // not in place
    if(body_ != body::in_place)
        detail::throw_logic_error();

    // incomplete
    if(st_ != state::complete)
        detail::throw_logic_error();

    buffers::const_buffer_pair bs = body_buf_->data();
    BOOST_ASSERT(bs[1].size() == 0);
    return string_view(static_cast<
        char const*>(bs[0].data()),
            bs[0].size());
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
    case state::start:
    case state::header:
        // Headers not received yet
        detail::throw_logic_error();

    case state::body:
    case state::complete:
        // VFALCO check if headers are discarded?
        break;
    }
    return &h_;
}

// Called immediately after
// complete headers are received
void
parser::
on_headers(
    system::error_code& ec)
{
    auto const overread =
        fb_.size() - h_.size;
    BOOST_ASSERT(
        overread <= svc_.max_overread());

    // reserve headers + table
    ws_.reserve_front(h_.size);
    ws_.reserve_back(h_.table_space());

    // no payload
    if( h_.md.payload == payload::none ||
        head_response_)
    {
        // set cb0_ to overread
        cb0_ = {
            ws_.data(),
            fb_.capacity() - h_.size,
            overread };
        st_ = state::complete;
        return;
    }

    // metadata error
    if(h_.md.payload == payload::error)
    {
        // VFALCO This needs looking at
        ec = BOOST_HTTP_PROTO_ERR(
            error::bad_payload);
        st_ = state::reset; // unrecoverable
        return;
    }

    // calculate filter
    filt_ = nullptr;

    // plain payload
    if( h_.md.payload != payload::chunked &&
        ! filt_)
    {
        // payload size is known
        if(h_.md.payload == payload::size)
        {
            if(h_.md.payload_size >
                svc_.cfg.body_limit)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::body_too_large);
                st_ = state::reset; // unrecoverable
                return;
            }
            auto n0 =
                fb_.capacity() - h_.size +
                svc_.cfg.min_buffer +
                svc_.max_codec;
            // limit the capacity of cb0_ so
            // that going over max_overread
            // is impossible.
            if( n0 > h_.md.payload_size &&
                n0 - h_.md.payload_size >=
                    svc_.max_overread())
                n0 =
                    h_.md.payload_size +
                    svc_.max_overread();
            BOOST_ASSERT(n0 <= ws_.size());
            cb0_ = {
                ws_.data(),
                n0,
                overread };
            body_buf_ = &cb0_;
            st_ = state::body;
            return;
        }

        // payload to eof
        // overread is not applicable
        auto const n0 =
            fb_.capacity() - h_.size +
            svc_.cfg.min_buffer +
            svc_.max_codec;
        BOOST_ASSERT(n0 <= ws_.size());
        cb0_ = {
            ws_.data(),
            n0,
            overread };
        body_buf_ = &cb0_;
        st_ = state::body;
        return;
    }

    // buffered payload
    auto const n0 =
        fb_.capacity() - h_.size;
    BOOST_ASSERT(
        n0 <= svc_.max_overread());
    auto n1 = svc_.cfg.min_buffer;
    if(! filt_)
        n1 += svc_.max_codec;
    BOOST_ASSERT(
        n0 + n1 <= ws_.size());
    cb0_ = {
        ws_.data(),
        n0,
        overread };
    cb1_ = {
        ws_.data() + n0,
        n1 };
    body_buf_ = &cb1_;
    st_ = state::body;
    return;
}

// Called at the end of set_body
void
parser::
on_set_body()
{
    // This function is called after all
    // limit checking and calculation of
    // chunked or filter.

    BOOST_ASSERT(got_header());

    if(h_.md.payload == payload::size)
    {
        if(body_ == body::dynamic)
        {
            dyn_->prepare(
                h_.md.payload_size);
            return;
        }
        if(body_ == body::sink)
        {
            //sink_->size_hint(&h_.md.payload_size);
            return;
        }
        return;
    }

    return;
}

} // http_proto
} // boost

#endif

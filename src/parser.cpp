//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

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

/*
    Principles for fixed-size buffer design

    axiom 1:
        To read data you must have a buffer.

    axiom 2:
        The size of the HTTP header is not
        known in advance.

    conclusion 3:
        A single I/O can produce a complete
        HTTP header and additional payload
        data.

    conclusion 4:
        A single I/O can produce multiple
        complete HTTP headers, complete
        payloads, and a partial header or
        payload.

    axiom 5:
        A process is in one of two states:
            1. at or below capacity
            2. above capacity

    axiom 6:
        A program which can allocate an
        unbounded number of resources can
        go above capacity.

    conclusion 7:
        A program can guarantee never going
        above capacity if all resources are
        provisioned at program startup.

    corollary 8:
        `parser` and `serializer` should each
        allocate a single buffer of calculated
        size, and never resize it.

    axiom #:
        A parser and a serializer are always
        used in pairs.

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

    "payload" is the bytes coming in from
        the stream.

    "body" is the logical body, after transfer
        encoding is removed. This can be the
        same as the payload.

    A "plain payload" is when the payload and
        body are identical (no transfer encodings).

    A "buffered payload" is any payload which is
        not plain. A second buffer is required
        for reading.

    "overread" is additional data received past
    the end of the headers when reading headers,
    or additional data received past the end of
    the message payload.
*/
//-----------------------------------------------

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
    | fb |     cb0     |     cb1     | C | T | f |

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

    // round up to alignof(detail::header::entry)
    auto const al = alignof(
        detail::header::entry);
    space_needed = al * ((
        space_needed + al - 1) / al);
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
//
// Special Members
//
//------------------------------------------------

parser::
parser(
    context& ctx,
    detail::kind k)
    : ctx_(ctx)
    , svc_(ctx.get_service<
        parser_service>())
    , h_(detail::empty{k})
    , eb_(nullptr)
    , st_(state::reset)
{
    auto const n =
        svc_.space_needed;
    ws_.allocate(n);
    h_.cap = n;
}

//------------------------------------------------

parser::
~parser()
{
}

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
    eb_ = nullptr;
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
        // reset required on eof
        if(got_eof_)
            detail::throw_logic_error();
        break;

    case state::header:
        if(fb_.size() == 0)
        {
            // start() called twice
            detail::throw_logic_error();
        }
        BOOST_FALLTHROUGH;

    case state::body:
    case state::set_body:
        // current message is incomplete
        detail::throw_logic_error();

    case state::complete:
    {
        // remove partial body.
        if(body_buf_ == &cb0_)
            cb0_.consume(static_cast<std::size_t>(body_avail_));

        if(cb0_.size() > 0)
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
    BOOST_ASSERT(fb_.capacity() ==
        svc_.max_overread());

    h_ = detail::header(
        detail::empty{h_.kind});
    h_.buf = reinterpret_cast<
        char*>(ws_.data());
    h_.cbuf = h_.buf;
    h_.cap = ws_.size();

    BOOST_ASSERT(! head_response ||
        h_.kind == detail::kind::response);
    head_response_ = head_response;

    // begin with in_place mode
    how_ = how::in_place;
    st_ = state::header;
    nprepare_ = 0;
}

auto
parser::
prepare() ->
    mutable_buffers_type
{
    nprepare_ = 0;

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
        nprepare_ = n;
        return mutable_buffers_type(
            &mbp_[0], 1);
    }

    case state::body:
    {
        if(got_eof_)
            return mutable_buffers_type{};

    do_body:
        if(! is_plain())
        {
            // buffered payload
            auto n = cb0_.capacity() -
                cb0_.size();
            if( n > svc_.cfg.max_prepare)
                n = svc_.cfg.max_prepare;
            mbp_ = cb0_.prepare(n);
            nprepare_ = n;
            return mutable_buffers_type(mbp_);
        }

        // plain payload

        if(how_ == how::in_place)
        {
            auto n =
                body_buf_->capacity() -
                body_buf_->size();
            if( n > svc_.cfg.max_prepare)
                n = svc_.cfg.max_prepare;
            mbp_ = body_buf_->prepare(n);
            nprepare_ = n;
            return mutable_buffers_type(mbp_);
        }

        if(how_ == how::elastic)
        {
            // Overreads are not allowed, or
            // else the caller will see extra
            // unrelated data.

            if(h_.md.payload == payload::size)
            {
                // set_body moves avail to dyn
                BOOST_ASSERT(body_buf_->size() == 0);
                BOOST_ASSERT(body_avail_ == 0);
                auto n = static_cast<std::size_t>(payload_remain_);
                if( n > svc_.cfg.max_prepare)
                    n = svc_.cfg.max_prepare;
                nprepare_ = n;
                return eb_->prepare(n);
            }

            BOOST_ASSERT(
                h_.md.payload == payload::to_eof);
            std::size_t n = 0;
            if(! got_eof_)
            {
                // calculate n heuristically
                n = svc_.cfg.min_buffer;
                if( n > svc_.cfg.max_prepare)
                    n = svc_.cfg.max_prepare;
                {
                    // apply max_size()
                    auto avail =
                        eb_->max_size() -
                            eb_->size();
                    if( n > avail)
                        n = avail;
                }
                // fill capacity() first,
                // to avoid an allocation
                {
                    auto avail =
                        eb_->capacity() -
                            eb_->size();
                    if( n > avail &&
                            avail != 0)
                        n = avail;
                }
                if(n == 0)
                {
                    // dynamic buffer is full
                    // attempt a 1 byte read so
                    // we can detect overflow
                    BOOST_ASSERT(
                        body_buf_->size() == 0);
                    // handled in init_dynamic
                    BOOST_ASSERT(
                        body_avail_ == 0);
                    mbp_ = body_buf_->prepare(1);
                    nprepare_ = 1;
                    return
                        mutable_buffers_type(mbp_);
                }
            }
            nprepare_ = n;
            return eb_->prepare(n);
        }

        // VFALCO TODO
        if(how_ == how::pull)
            detail::throw_logic_error();

        // VFALCO TODO
        detail::throw_logic_error();
    }

    case state::set_body:
    {
        BOOST_ASSERT(is_plain());

        if(how_ == how::elastic)
        {
            // attempt to transfer in-place
            // body into the dynamic buffer.
            system::error_code ec;
            init_dynamic(ec);
            if(! ec.failed())
            {
                if(st_ == state::body)
                    goto do_body;
                BOOST_ASSERT(
                    st_ == state::complete);
                return mutable_buffers_type{};
            }

            // not enough room, so we
            // return this error from parse()
            return
                mutable_buffers_type{};
        }

        if(how_ == how::sink)
        {
            // this is a no-op, to get the
            // caller to call parse next.
            return mutable_buffers_type{};
        }

        // VFALCO TODO
        detail::throw_logic_error();
    }

    case state::complete:
        // intended no-op
        return mutable_buffers_type{};
    }
}

void
parser::
commit(
    std::size_t n)
{
    switch(st_)
    {
    default:
    case state::reset:
    {
        // reset must be called first
        detail::throw_logic_error();
    }

    case state::start:
    {
        // forgot to call start()
        detail::throw_logic_error();
    }

    case state::header:
    {
        if(n > nprepare_)
        {
            // n can't be greater than size of
            // the buffers returned by prepare()
            detail::throw_invalid_argument();
        }

        if(got_eof_)
        {
            // can't commit after EOF
            detail::throw_logic_error();
        }

        nprepare_ = 0; // invalidate
        fb_.commit(n);
        break;
    }

    case state::body:
    {
        if(n > nprepare_)
        {
            // n can't be greater than size of
            // the buffers returned by prepare()
            detail::throw_invalid_argument();
        }

        BOOST_ASSERT(! got_eof_ || n == 0);

        if(! is_plain())
        {
            // buffered payload
            cb0_.commit(n);
            break;
        }

        // plain payload

        if(how_ == how::in_place)
        {
            BOOST_ASSERT(body_buf_ == &cb0_);
            cb0_.commit(n);
            if(h_.md.payload == payload::size)
            {
                if(cb0_.size() <
                    h_.md.payload_size)
                {
                    body_avail_ += n;
                    payload_remain_ -= n;
                    break;
                }
                body_avail_ = h_.md.payload_size;
                payload_remain_ = 0;
                st_ = state::complete;
                break;
            }

            BOOST_ASSERT(
                h_.md.payload == payload::to_eof);
            body_avail_ += n;
            break;
        }

        if(how_ == how::elastic)
        {
            if(eb_->size() < eb_->max_size())
            {
                BOOST_ASSERT(body_avail_ == 0);
                BOOST_ASSERT(
                    body_buf_->size() == 0);
                eb_->commit(n);
            }
            else
            {
                // If we get here then either
                // n==0 as a no-op, or n==1 for
                // an intended one byte read.
                BOOST_ASSERT(n <= 1);
                body_buf_->commit(n);
                body_avail_ += n;
            }
            body_total_ += n;
            if(h_.md.payload == payload::size)
            {
                BOOST_ASSERT(
                    n <= payload_remain_);
                payload_remain_ -= n;
                if(payload_remain_ == 0)
                    st_ = state::complete;
            }
            break;
        }

        if(how_ == how::sink)
        {
            cb0_.commit(n);
            break;
        }

        if(how_ == how::pull)
        {
            // VFALCO TODO
            detail::throw_logic_error();
        }
        break;
    }

    case state::set_body:
    {
        if(n > nprepare_)
        {
            // n can't be greater than size of
            // the buffers returned by prepare()
            detail::throw_invalid_argument();
        }

        BOOST_ASSERT(is_plain());
        BOOST_ASSERT(n == 0);
        if( how_ == how::elastic ||
            how_ == how::sink)
        {
            // intended no-op
            break;
        }

        // VFALCO TODO
        detail::throw_logic_error();
    }

    case state::complete:
    {
        BOOST_ASSERT(nprepare_ == 0);

        if(n > 0)
        {
            // n can't be greater than size of
            // the buffers returned by prepare()
            detail::throw_invalid_argument();
        }

        // intended no-op
        break;
    }
    }
}

void
parser::
commit_eof()
{
    nprepare_ = 0; // invalidate

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

    case state::set_body:
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

            if(fb_.size() == 0)
            {
                // stream closed cleanly
                st_ = state::complete;
                ec = BOOST_HTTP_PROTO_ERR(
                    error::end_of_stream);
                return;
            }

            // stream closed with a
            // partial message received
            st_ = state::reset;
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
        BOOST_FALLTHROUGH;
    }

    case state::body:
    {
    do_body:
        BOOST_ASSERT(st_ == state::body);
        BOOST_ASSERT(
            h_.md.payload != payload::none);
        BOOST_ASSERT(
            h_.md.payload != payload::error);
        if(h_.md.payload == payload::chunked)
        {
            // VFALCO parse chunked
            detail::throw_logic_error();
        }
        else if(filt_)
        {
            // VFALCO TODO apply filter
            detail::throw_logic_error();
        }

        if(how_ == how::in_place)
        {
            BOOST_ASSERT(body_avail_ ==
                body_buf_->size());
            if(h_.md.payload == payload::size)
            {
                if(body_avail_ <
                    h_.md.payload_size)
                {
                    if(got_eof_)
                    {
                        // incomplete
                        ec = BOOST_HTTP_PROTO_ERR(
                            error::incomplete);
                        return;
                    }
                    if(body_buf_->capacity() == 0)
                    {
                        // in_place buffer limit
                        ec = BOOST_HTTP_PROTO_ERR(
                            error::in_place_overflow);
                        return;
                    }
                    ec = BOOST_HTTP_PROTO_ERR(
                        error::need_data);
                    return;
                }
                BOOST_ASSERT(body_avail_ ==
                    h_.md.payload_size);
                st_ = state::complete;
                break;
            }
            if(body_avail_ > svc_.cfg.body_limit)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::body_too_large);
                st_ = state::reset; // unrecoverable
                return;
            }
            if( h_.md.payload == payload::chunked ||
                ! got_eof_)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::need_data);
                return;
            }
            BOOST_ASSERT(got_eof_);
            st_ = state::complete;
            break;
        }

        if(how_ == how::elastic)
        {
            // state already updated in commit
            if(h_.md.payload == payload::size)
            {
                BOOST_ASSERT(body_total_ <
                    h_.md.payload_size);
                BOOST_ASSERT(payload_remain_ > 0);
                if(body_avail_ != 0)
                {
                    BOOST_ASSERT(
                        eb_->max_size() -
                            eb_->size() <
                        payload_remain_);
                    ec = BOOST_HTTP_PROTO_ERR(
                        error::buffer_overflow);
                    st_ = state::reset; // unrecoverable
                    return;
                }
                if(got_eof_)
                {
                    ec = BOOST_HTTP_PROTO_ERR(
                        error::incomplete);
                    st_ = state::reset; // unrecoverable
                    return;
                }
                return;
            }
            BOOST_ASSERT(
                h_.md.payload == payload::to_eof);
            if( eb_->size() == eb_->max_size() &&
                body_avail_ > 0)
            {
                // got here from the 1-byte read
                ec = BOOST_HTTP_PROTO_ERR(
                    error::buffer_overflow);
                st_ = state::reset; // unrecoverable
                return;
            }
            if(got_eof_)
            {
                BOOST_ASSERT(body_avail_ == 0);
                st_ = state::complete;
                break;
            }
            BOOST_ASSERT(body_avail_ == 0);
            break;
        }

        // VFALCO TODO
        detail::throw_logic_error();
    }

    case state::set_body:
    {
        BOOST_ASSERT(is_plain());

        // transfer in_place data into set body

        if(how_ == how::elastic)
        {
            init_dynamic(ec);
            if(! ec.failed())
            {
                if(st_ == state::body)
                    goto do_body;
                BOOST_ASSERT(
                    st_ == state::complete);
                break;
            }
            st_ = state::reset; // unrecoverable
            return;
        }

        if(how_ == how::sink)
        {
            auto n = body_buf_->size();
            if(h_.md.payload == payload::size)
            {
                // sink_->size_hint(h_.md.payload_size, ec);

                if(n < h_.md.payload_size)
                {
                    auto rv = sink_->write(
                        body_buf_->data(), false);
                    BOOST_ASSERT(rv.ec.failed() ||
                        rv.bytes == body_buf_->size());
                    BOOST_ASSERT(
                        rv.bytes >= body_avail_);
                    BOOST_ASSERT(
                        rv.bytes < payload_remain_);
                    body_buf_->consume(rv.bytes);
                    body_avail_ -= rv.bytes;
                    body_total_ += rv.bytes;
                    payload_remain_ -= rv.bytes;
                    if(rv.ec.failed())
                    {
                        ec = rv.ec;
                        st_ = state::reset; // unrecoverable
                        return;
                    }
                    st_ = state::body;
                    goto do_body;
                }

                n = static_cast<std::size_t>(h_.md.payload_size);
            }
            // complete
            BOOST_ASSERT(body_buf_ == &cb0_);
            auto rv = sink_->write(
                body_buf_->data(), true);
            BOOST_ASSERT(rv.ec.failed() ||
                rv.bytes == body_buf_->size());
            body_buf_->consume(rv.bytes);
            if(rv.ec.failed())
            {
                ec = rv.ec;
                st_ = state::reset; // unrecoverable
                return;
            }
            st_ = state::complete;
            return;
        }

        // VFALCO TODO
        detail::throw_logic_error();
    }

    case state::complete:
    {
        // This is a no-op except when set_body
        // was called and we have in-place data.
        switch(how_)
        {
        default:
        case how::in_place:
            break;

        case how::elastic:
        {
            if(body_buf_->size() == 0)
                break;
            BOOST_ASSERT(eb_->size() == 0);
            auto n = buffers::buffer_copy(
                eb_->prepare(
                    body_buf_->size()),
                body_buf_->data());
            body_buf_->consume(n);
            break;
        }

        case how::sink:
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

        case how::pull:
            // VFALCO TODO
            detail::throw_logic_error();
        }
    }
    }
}

//------------------------------------------------

auto
parser::
pull_some() ->
    const_buffers_type
{
    return {};
}

core::string_view
parser::
body() const noexcept
{
    switch(st_)
    {
    default:
    case state::reset:
    case state::start:
    case state::header:
    case state::body:
    case state::set_body:
        // not complete
        return {};

    case state::complete:
        if(how_ != how::in_place)
        {
            // not in_place
            return {};
        }
        auto cbp = body_buf_->data();
        BOOST_ASSERT(cbp[1].size() == 0);
        BOOST_ASSERT(cbp[0].size() >= body_avail_);
        return core::string_view(
            static_cast<char const*>(
                cbp[0].data()),
            static_cast<std::size_t>(body_avail_));
    }
}

core::string_view
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
    // headers must be received
    if( ! got_header() ||
        fb_.size() == 0) // happens on eof
        detail::throw_logic_error();

    return &h_;
}

bool
parser::
is_plain() const noexcept
{
    return ! filt_ &&
        h_.md.payload !=
            payload::chunked;
}

// Called immediately after complete headers
// are received. We leave fb_ as-is to indicate
// whether any data was received before eof.
//
void
parser::
on_headers(
    system::error_code& ec)
{
    auto const overread = fb_.size() - h_.size;
    BOOST_ASSERT(
        overread <= svc_.max_overread());

    // metadata error
    if(h_.md.payload == payload::error)
    {
        // VFALCO This needs looking at
        ec = BOOST_HTTP_PROTO_ERR(
            error::bad_payload);
        st_ = state::reset; // unrecoverable
        return;
    }

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
        body_avail_ = 0;
        body_total_ = 0;
        body_buf_ = &cb0_;
        st_ = state::complete;
        return;
    }

    // calculate filter
    filt_ = nullptr; // VFALCO TODO

    if(is_plain())
    {
        // plain payload

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
                n0 = static_cast<std::size_t>(h_.md.payload_size) +
                    svc_.max_overread();
            BOOST_ASSERT(n0 <= ws_.size());
            cb0_ = { ws_.data(), n0, overread };
            body_buf_ = &cb0_;
            body_avail_ = cb0_.size();
            if( body_avail_ >= h_.md.payload_size)
                body_avail_ = h_.md.payload_size;
            body_total_ = body_avail_;
            payload_remain_ =
                h_.md.payload_size - body_total_;
            st_ = state::body;
            return;
        }

        // overread is not applicable
        BOOST_ASSERT(
            h_.md.payload == payload::to_eof);
        auto const n0 =
            fb_.capacity() - h_.size +
            svc_.cfg.min_buffer +
            svc_.max_codec;
        BOOST_ASSERT(n0 <= ws_.size());
        cb0_ = { ws_.data(), n0, overread };
        body_buf_ = &cb0_;
        body_avail_ = cb0_.size();
        body_total_ = body_avail_;
        st_ = state::body;
        return;
    }

    // buffered payload
    auto const n0 = fb_.capacity() - h_.size;
    BOOST_ASSERT(n0 <= svc_.max_overread());
    auto n1 = svc_.cfg.min_buffer;
    if(! filt_)
        n1 += svc_.max_codec;
    BOOST_ASSERT(n0 + n1 <= ws_.size());
    cb0_ = { ws_.data(), n0, overread };
    cb1_ = { ws_.data() + n0, n1 };
    body_buf_ = &cb1_;
    body_avail_ = 0;
    body_total_ = 0;
    st_ = state::body;
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

    nprepare_ = 0; // invalidate

    if(how_ == how::elastic)
    {
        if(h_.md.payload == payload::none)
        {
            BOOST_ASSERT(st_ == state::complete);
            return;
        }

        st_ = state::set_body;
        return;
    }

    if(how_ == how::sink)
    {
        if(h_.md.payload == payload::none)
        {
            BOOST_ASSERT(st_ == state::complete);
            // force a trip through parse so
            // we can calculate any error.
            st_ = state::set_body;
            return;
        }

        st_ = state::set_body;
        return;
    }

    // VFALCO TODO
    detail::throw_logic_error();
}

void
parser::
init_dynamic(
    system::error_code& ec)
{
    // attempt to transfer in-place
    // body into the dynamic buffer.
    BOOST_ASSERT(
        body_avail_ == body_buf_->size());
    BOOST_ASSERT(
        body_total_ == body_avail_);
    auto const space_left =
        eb_->max_size() - eb_->size();

    if(h_.md.payload == payload::size)
    {
        if(space_left < h_.md.payload_size)
        {
            ec = BOOST_HTTP_PROTO_ERR(
                error::buffer_overflow);
            return;
        }
        // reserve the full size
        eb_->prepare(static_cast<std::size_t>(h_.md.payload_size));
        // transfer in-place body
        auto n = static_cast<std::size_t>(body_avail_);
        if( n > h_.md.payload_size)
            n = static_cast<std::size_t>(h_.md.payload_size);
        eb_->commit(
            buffers::buffer_copy(
                eb_->prepare(n),
                body_buf_->data()));
        BOOST_ASSERT(body_avail_ == n);
        BOOST_ASSERT(body_total_ == n);
        BOOST_ASSERT(payload_remain_ ==
            h_.md.payload_size - n);
        body_buf_->consume(n);
        body_avail_ = 0;
        if(n < h_.md.payload_size)
        {
            BOOST_ASSERT(
                body_buf_->size() == 0);
            st_ = state::body;
            return;
        }
        // complete
        st_ = state::complete;
        return;
    }

    BOOST_ASSERT(h_.md.payload ==
        payload::to_eof);
    if(space_left < body_avail_)
    {
        ec = BOOST_HTTP_PROTO_ERR(
            error::buffer_overflow);
        return;
    }
    eb_->commit(
        buffers::buffer_copy(
            eb_->prepare(static_cast<std::size_t>(body_avail_)),
            body_buf_->data()));
    body_buf_->consume(static_cast<std::size_t>(body_avail_));
    body_avail_ = 0;
    BOOST_ASSERT(
        body_buf_->size() == 0);
    st_ = state::body;
}

} // http_proto
} // boost

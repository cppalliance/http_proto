//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/parser.hpp>

#include "src/detail/brotli_filter_base.hpp"
#include "src/detail/zlib_filter_base.hpp"

#include <boost/assert.hpp>
#include <boost/buffers/copy.hpp>
#include <boost/buffers/front.hpp>
#include <boost/buffers/prefix.hpp>
#include <boost/buffers/size.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/rts/brotli/decode.hpp>
#include <boost/rts/zlib/error.hpp>
#include <boost/rts/zlib/inflate.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/hexdig_chars.hpp>

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

namespace {
class chained_sequence
{
    char const* pos_;
    char const* end_;
    char const* begin_b_;
    char const* end_b_;

public:
    chained_sequence(buffers::const_buffer_pair const& cbp)
        : pos_(static_cast<char const*>(cbp[0].data()))
        , end_(pos_ + cbp[0].size())
        , begin_b_(static_cast<char const*>(cbp[1].data()))
        , end_b_(begin_b_ + cbp[1].size())
    {
    }

    char const*
    next() noexcept
    {
        ++pos_;
        // most frequently taken branch
        if(pos_ < end_)
            return pos_;

        // bring the second range
        if(begin_b_ != end_b_)
        {
            pos_ = begin_b_;
            end_ = end_b_;
            begin_b_ = end_b_;
            return pos_;
        }

        // undo the increament
        pos_ = end_;
        return nullptr;
    }

    bool
    is_empty() const noexcept
    {
        return pos_ == end_;
    }

    char
    value() const noexcept
    {
        return *pos_;
    }

    std::size_t
    size() const noexcept
    {
        return (end_ - pos_) + (end_b_ - begin_b_);
    }
};

std::uint64_t
parse_hex(
    chained_sequence& cs,
    system::error_code& ec) noexcept
{
    std::uint64_t v   = 0;
    std::size_t init_size = cs.size();
    while(!cs.is_empty())
    {
        auto n = grammar::hexdig_value(cs.value());
        if(n < 0)
        {
            if(init_size == cs.size())
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::bad_payload);
                return 0;
            }
            return v;
        }

        // at least 4 significant bits are free
        if(v > (std::numeric_limits<std::uint64_t>::max)() >> 4)
        {
            ec = BOOST_HTTP_PROTO_ERR(
                error::bad_payload);
            return 0;
        }

        v = (v << 4) | static_cast<std::uint64_t>(n);
        cs.next();
    }
    ec = BOOST_HTTP_PROTO_ERR(
        error::need_data);
    return 0;
}

void
find_eol(
    chained_sequence& cs,
    system::error_code& ec) noexcept
{
    while(!cs.is_empty())
    {
        if(cs.value() == '\r')
        {
            if(!cs.next())
                break;
            if(cs.value() != '\n')
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::bad_payload);
                return;
            }
            cs.next();
            return;
        }
        cs.next();
    }
    ec = BOOST_HTTP_PROTO_ERR(
        error::need_data);
}

void
parse_eol(
    chained_sequence& cs,
    system::error_code& ec) noexcept
{
    if(cs.size() >= 2)
    {
        // we are sure size is at least 2
        if(cs.value() == '\r' && *cs.next() == '\n')
        {
            cs.next();
            return;
        }
        ec = BOOST_HTTP_PROTO_ERR(
            error::bad_payload);
        return;
    }
    ec = BOOST_HTTP_PROTO_ERR(
        error::need_data);
}

void
skip_trailer_headers(
    chained_sequence& cs,
    system::error_code& ec) noexcept
{
    while(!cs.is_empty())
    {
        if(cs.value() == '\r')
        {
            if(!cs.next())
                break;
            if(cs.value() != '\n')
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::bad_payload);
                return;
            }
            cs.next();
            return;
        }
        // skip to the end of field
        find_eol(cs, ec);
        if(ec)
            return;
    }
    ec = BOOST_HTTP_PROTO_ERR(
        error::need_data);
}

template<class UInt>
std::size_t
clamp(
    UInt x,
    std::size_t limit = (std::numeric_limits<
        std::size_t>::max)()) noexcept
{
    if(x >= limit)
        return limit;
    return static_cast<std::size_t>(x);
}

class zlib_filter
    : public detail::zlib_filter_base
{
    rts::zlib::inflate_service& svc_;

public:
    zlib_filter(
        rts::context& ctx,
        http_proto::detail::workspace& ws,
        int window_bits)
        : zlib_filter_base(ws)
        , svc_(ctx.get_service<rts::zlib::inflate_service>())
    {
        system::error_code ec = static_cast<rts::zlib::error>(
            svc_.init2(strm_, window_bits));
        if(ec != rts::zlib::error::ok)
            detail::throw_system_error(ec);
    }

private:
    virtual
    results
    do_process(
        buffers::mutable_buffer out,
        buffers::const_buffer in,
        bool more) noexcept override
    {
        strm_.next_out  = static_cast<unsigned char*>(out.data());
        strm_.avail_out = saturate_cast(out.size());
        strm_.next_in   = static_cast<unsigned char*>(const_cast<void *>(in.data()));
        strm_.avail_in  = saturate_cast(in.size());

        auto rs = static_cast<rts::zlib::error>(
            svc_.inflate(
                strm_,
                more ? rts::zlib::no_flush : rts::zlib::finish));

        results rv;
        rv.out_bytes = saturate_cast(out.size()) - strm_.avail_out;
        rv.in_bytes  = saturate_cast(in.size()) - strm_.avail_in;
        rv.finished  = (rs == rts::zlib::error::stream_end);

        if(rs < rts::zlib::error::ok && rs != rts::zlib::error::buf_err)
            rv.ec = rs;

        return rv;
    }
};

class brotli_filter
    : public detail::brotli_filter_base
{
    rts::brotli::decode_service& svc_;
    rts::brotli::decoder_state* state_;

public:
    brotli_filter(
        rts::context& ctx,
        http_proto::detail::workspace&)
        : svc_(ctx.get_service<rts::brotli::decode_service>())
    {
        // TODO: use custom allocator
        state_ = svc_.create_instance(nullptr, nullptr, nullptr);

        if(!state_)
            detail::throw_bad_alloc();
    }

    ~brotli_filter()
    {
        svc_.destroy_instance(state_);
    }

private:
    virtual
    results
    do_process(
        buffers::mutable_buffer out,
        buffers::const_buffer in,
        bool more) noexcept override
    {
        auto* next_in = reinterpret_cast<const std::uint8_t*>(in.data());
        auto available_in = in.size();
        auto* next_out = reinterpret_cast<std::uint8_t*>(out.data());
        auto available_out = out.size();

        auto rs = svc_.decompress_stream(
            state_,
            &available_in,
            &next_in,
            &available_out,
            &next_out,
            nullptr);

        results rv;
        rv.in_bytes  = in.size()  - available_in;
        rv.out_bytes = out.size() - available_out;
        rv.finished  = svc_.is_finished(state_);

        if(!more && rs == rts::brotli::decoder_result::needs_more_input)
            rv.ec = BOOST_HTTP_PROTO_ERR(error::bad_payload);

        if(rs == rts::brotli::decoder_result::error)
            rv.ec = BOOST_HTTP_PROTO_ERR(
                svc_.get_error_code(state_));

        return rv;
    }
};

} // namespace

class parser_service
    : public rts::service
{
public:
    parser::config_base cfg;
    std::size_t space_needed = 0;
    std::size_t max_codec = 0;

    parser_service(
        rts::context&,
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
        if(cfg.apply_deflate_decoder || cfg.apply_gzip_decoder)
        {
            // TODO: Account for the number of allocations and
            // their overhead in the workspace.

            // https://www.zlib.net/zlib_tech.html
            std::size_t n =
                (1 << cfg.zlib_window_bits) +
                (7 * 1024) +
                #ifdef __s390x__
                5768 +
                #endif
                detail::workspace::space_needed<
                    zlib_filter>();

            if(max_codec < n)
                max_codec = n;
        }
        space_needed += max_codec;

        // round up to alignof(detail::header::entry)
        auto const al = alignof(
            detail::header::entry);
        space_needed = al * ((
            space_needed + al - 1) / al);
    }

    std::size_t
    max_overread() const noexcept
    {
        return
            cfg.headers.max_size +
            cfg.min_buffer;
    }
};

void
install_parser_service(
    rts::context& ctx,
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
parser(rts::context& ctx, detail::kind k)
    : ctx_(ctx)
    , svc_(ctx.get_service<parser_service>())
    , ws_(svc_.space_needed)
    , h_(detail::empty{ k })
    , st_(state::reset)
    , got_header_(false)
{
}

parser::
~parser()
{
}

//--------------------------------------------
//
// Observers
//
//--------------------------------------------

bool
parser::got_header() const noexcept
{
    return got_header_;
}

bool
parser::is_complete() const noexcept
{
    return st_ >= state::complete_in_place;
}

//------------------------------------------------
//
// Modifiers
//
//------------------------------------------------

void
parser::
reset() noexcept
{
    ws_.clear();
    st_ = state::start;
    got_header_ = false;
    got_eof_ = false;
}

void
parser::start()
{
    start_impl(false);
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

    case state::header_done:
    case state::body:
    case state::set_body:
        // current message is incomplete
        detail::throw_logic_error();

    case state::complete_in_place:
        // remove available body.
        if(is_plain())
            cb0_.consume(body_avail_);
        BOOST_FALLTHROUGH;

    case state::complete:
    {
        // move leftovers to front

        ws_.clear();
        leftover = cb0_.size();

        auto* dest = reinterpret_cast<char*>(ws_.data());
        auto cbp   = cb0_.data();
        auto* a    = static_cast<char const*>(cbp[0].data());
        auto* b    = static_cast<char const*>(cbp[1].data());
        auto an    = cbp[0].size();
        auto bn    = cbp[1].size();

        if(bn == 0)
        {
            std::memmove(dest, a, an);
        }
        else
        {
            // if `a` can fit between `dest` and `b`, shift `b` to the left
            // and copy `a` to its position. if `a` fits perfectly, the
            // shift will be of size 0.
            // if `a` requires more space, shift `b` to the right and
            // copy `a` to its position. this process may require multiple
            // iterations and should be done chunk by chunk to prevent `b`
            // from overlapping with `a`.
            do
            {
                // clamp right shifts to prevent overlap with `a`
                auto* bp = (std::min)(dest + an, const_cast<char*>(a) - bn);
                b = static_cast<char const*>(std::memmove(bp, b, bn));

                // a chunk or all of `a` based on available space
                auto chunk_a = static_cast<std::size_t>(b - dest);
                std::memcpy(dest, a, chunk_a); // never overlap
                an   -= chunk_a;
                dest += chunk_a;
                a    += chunk_a;
            } while(an);
        }

        break;
    }
    }

    ws_.clear();

    fb_ = {
        ws_.data(),
        svc_.cfg.headers.max_size + svc_.cfg.min_buffer,
        leftover };

    BOOST_ASSERT(
        fb_.capacity() == svc_.max_overread() - leftover);

    BOOST_ASSERT(
        head_response == false ||
        h_.kind == detail::kind::response);

    h_ = detail::header(detail::empty{h_.kind});
    h_.buf = reinterpret_cast<char*>(ws_.data());
    h_.cbuf = h_.buf;
    h_.cap = ws_.size();

    st_ = state::header;
    how_ = how::in_place;

    // reset to the configured default
    body_limit_ = svc_.cfg.body_limit;

    body_total_ = 0;
    payload_remain_ = 0;
    chunk_remain_ = 0;
    body_avail_ = 0;
    nprepare_ = 0;

    filter_ = nullptr;
    eb_ = nullptr;
    sink_ = nullptr;

    got_header_ = false;
    head_response_ = head_response;
    needs_chunk_close_ = false;
    trailer_headers_ = false;
    chunked_body_ended = false;
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
        BOOST_ASSERT(
            h_.size < svc_.cfg.headers.max_size);
        std::size_t n = fb_.capacity() - fb_.size();
        BOOST_ASSERT(n <= svc_.max_overread());
        n = clamp(n, svc_.cfg.max_prepare);
        mbp_[0] = fb_.prepare(n);
        nprepare_ = n;
        return mutable_buffers_type(&mbp_[0], 1);
    }

    case state::header_done:
        // forgot to call parse()
        detail::throw_logic_error();

    case state::body:
    {
        if(got_eof_)
        {
            // forgot to call parse()
            detail::throw_logic_error();
        }

        if(! is_plain())
        {
            // buffered payload
            std::size_t n = cb0_.capacity();
            n = clamp(n, svc_.cfg.max_prepare);
            nprepare_ = n;
            mbp_ = cb0_.prepare(n);
            return mutable_buffers_type(mbp_);
        }
        else
        {
            switch(how_)
            {
            default:
            case how::in_place:
            case how::sink:
            {
                std::size_t n = cb0_.capacity();
                n = clamp(n, svc_.cfg.max_prepare);

                if(h_.md.payload == payload::size)
                {
                    if(n > payload_remain_)
                    {
                        std::size_t overread =
                            n - static_cast<std::size_t>(payload_remain_);
                        if(overread > svc_.max_overread())
                            n = static_cast<std::size_t>(payload_remain_) +
                                svc_.max_overread();
                    }
                }
                else
                {
                    BOOST_ASSERT(
                        h_.md.payload == payload::to_eof);
                    n = clamp(body_limit_remain() + 1, n);
                }

                nprepare_ = n;
                mbp_ = cb0_.prepare(n);
                return mutable_buffers_type(mbp_);
            }
            case how::elastic:
            {
                BOOST_ASSERT(cb0_.size() == 0);
                BOOST_ASSERT(body_avail_ == 0);

                std::size_t n = svc_.cfg.min_buffer;

                if(h_.md.payload == payload::size)
                {
                    // Overreads are not allowed, or
                    // else the caller will see extra
                    // unrelated data.
                    n = clamp(payload_remain_, n);
                }
                else
                {
                    BOOST_ASSERT(
                        h_.md.payload == payload::to_eof);
                    n = clamp(body_limit_remain() + 1, n);
                    n = clamp(n, eb_->max_size() - eb_->size());
                    // fill capacity first to avoid an allocation
                    std::size_t avail =
                        eb_->capacity() - eb_->size();
                    if(avail != 0)
                        n = clamp(n, avail);

                    if(n == 0)
                    {
                        // dynamic buffer is full
                        // attempt a 1 byte read so
                        // we can detect overflow
                        nprepare_ = 1;
                        mbp_ = cb0_.prepare(1);
                        return mutable_buffers_type(mbp_);
                    }
                }

                n = clamp(n, svc_.cfg.max_prepare);
                BOOST_ASSERT(n != 0);
                nprepare_ = n;
                return eb_->prepare(n);
            }
            }
        }
    }

    case state::set_body:
        // forgot to call parse()
        detail::throw_logic_error();

    case state::complete_in_place:
    case state::complete:
        // already complete
        detail::throw_logic_error();
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

    case state::header_done:
    {
        // forgot to call parse()
        detail::throw_logic_error();
    }

    case state::body:
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
        if(is_plain() && how_ == how::elastic)
        {
            if(eb_->max_size() == eb_->size())
            {
                // borrowed 1 byte from
                // cb0_ in prepare()
                BOOST_ASSERT(n <= 1);
                cb0_.commit(n);
            }
            else
            {
                eb_->commit(n);
                payload_remain_ -= n;
                body_total_     += n;
            }
        }
        else
        {
            cb0_.commit(n);
        }
        break;
    }

    case state::set_body:
    {
        // forgot to call parse()
        detail::throw_logic_error();
    }

    case state::complete_in_place:
    case state::complete:
    {
        // already complete
        detail::throw_logic_error();
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

    case state::header_done:
        // forgot to call parse()
        detail::throw_logic_error();

    case state::body:
        got_eof_ = true;
        break;

    case state::set_body:
        // forgot to call parse()
        detail::throw_logic_error();

    case state::complete_in_place:
    case state::complete:
        // can't commit eof when complete
        detail::throw_logic_error();
    }
}

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

        h_.parse(fb_.size(), svc_.cfg.headers, ec);

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
                st_ = state::reset;
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
        else if(ec.failed())
        {
            // other error,
            //
            // VFALCO map this to a bad
            // request or bad response error?
            //
            st_ = state::reset; // unrecoverable
            return;
        }

        got_header_ = true;

        // reserve headers + table
        ws_.reserve_front(h_.size);
        ws_.reserve_back(h_.table_space());

        // no payload
        if(h_.md.payload == payload::none ||
            head_response_)
        {
            // octets of the next message
            auto overread = fb_.size() - h_.size;
            cb0_ = { ws_.data(), overread, overread };
            ws_.reserve_front(overread);
            st_ = state::complete_in_place;
            return;
        }

        st_ = state::header_done;
        break;
    }

    case state::header_done:
    {
        // metadata error
        if(h_.md.payload == payload::error)
        {
            // VFALCO This needs looking at
            ec = BOOST_HTTP_PROTO_ERR(
                error::bad_payload);
            st_ = state::reset; // unrecoverable
            return;
        }

        // overread currently includes any and all octets that
        // extend beyond the current end of the header
        // this can include associated body octets for the
        // current message or octets of the next message in the
        // stream, e.g. pipelining is being used
        auto const overread = fb_.size() - h_.size;
        BOOST_ASSERT(overread <= svc_.max_overread());

        auto cap = fb_.capacity() + overread +
            svc_.cfg.min_buffer;

        // reserve body buffers first, as the decoder
        // must be installed after them.
        auto const p = ws_.reserve_front(cap);

        switch(h_.md.content_encoding.coding)
        {
        case content_coding::deflate:
            if(!svc_.cfg.apply_deflate_decoder)
                goto no_filter;
            filter_ = &ws_.emplace<zlib_filter>(
                ctx_, ws_, svc_.cfg.zlib_window_bits);
            break;

        case content_coding::gzip:
            if(!svc_.cfg.apply_deflate_decoder)
                goto no_filter;
            filter_ = &ws_.emplace<zlib_filter>(
                ctx_, ws_, svc_.cfg.zlib_window_bits + 16);
            break;

        case content_coding::br:
            if(!svc_.cfg.apply_brotli_decoder)
                goto no_filter;
            filter_ = &ws_.emplace<brotli_filter>(
                ctx_, ws_);
            break;

        no_filter:
        default:
            cap += svc_.max_codec;
            ws_.reserve_front(svc_.max_codec);
            break;
        }

        if(is_plain() || how_ == how::elastic)
        {
            cb0_ = { p, cap, overread };
            cb1_ = {};
        }
        else
        {
            // buffered payload
            std::size_t n0 = (overread > svc_.cfg.min_buffer)
                ? overread
                : svc_.cfg.min_buffer;
            std::size_t n1 = svc_.cfg.min_buffer;

            cb0_ = { p      , n0, overread };
            cb1_ = { p + n0 , n1 };
        }

        if(h_.md.payload == payload::size)
        {
            if(!filter_ &&
                body_limit_ < h_.md.payload_size)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::body_too_large);
                st_ = state::reset;
                return;
            }
            payload_remain_ = h_.md.payload_size;
        }

        st_ = state::body;
        BOOST_FALLTHROUGH;
    }

    case state::body:
    {
    do_body:
        BOOST_ASSERT(st_ == state::body);
        BOOST_ASSERT(h_.md.payload != payload::none);
        BOOST_ASSERT(h_.md.payload != payload::error);

        auto set_state_to_complete = [&]()
        {
            if(how_ == how::in_place)
            {
                st_ = state::complete_in_place;
                return;
            }
            st_ = state::complete;
        };

        if(h_.md.payload == payload::chunked)
        {
            for(;;)
            {
                if(chunk_remain_ == 0
                    && !chunked_body_ended)
                {
                    auto cs = chained_sequence(cb0_.data());
                    auto check_ec = [&]()
                    {
                        if(ec == condition::need_more_input && got_eof_)
                        {
                            ec = BOOST_HTTP_PROTO_ERR(error::incomplete);
                            st_ = state::reset;
                        }
                    };

                    if(needs_chunk_close_)
                    {
                        parse_eol(cs, ec);
                        if(ec)
                        {
                            check_ec();
                            return;
                        }
                    }
                    else if(trailer_headers_)
                    {
                        skip_trailer_headers(cs, ec);
                        if(ec)
                        {
                            check_ec();
                            return;
                        }
                        cb0_.consume(cb0_.size() - cs.size());
                        chunked_body_ended = true;
                        continue;
                    }
                    
                    auto chunk_size = parse_hex(cs, ec);
                    if(ec)
                    {
                        check_ec();
                        return;
                    }

                    // skip chunk extensions
                    find_eol(cs, ec);
                    if(ec)
                    {
                        check_ec();
                        return;
                    }

                    cb0_.consume(cb0_.size() - cs.size());
                    chunk_remain_ = chunk_size;

                    needs_chunk_close_ = true;
                    if(chunk_remain_ == 0)
                    {
                        needs_chunk_close_ = false;
                        trailer_headers_ = true;
                        continue;
                    }
                }

                if(cb0_.size() == 0 && !chunked_body_ended)
                {
                    if(got_eof_)
                    {
                        ec = BOOST_HTTP_PROTO_ERR(
                            error::incomplete);
                        st_ = state::reset;
                        return;
                    }

                    ec = BOOST_HTTP_PROTO_ERR(
                        error::need_data);
                    return;
                }

                if(filter_)
                {
                    chunk_remain_ -= apply_filter(
                        ec,
                        clamp(chunk_remain_, cb0_.size()),
                        !chunked_body_ended);

                    if(ec || chunked_body_ended)
                        return;
                }
                else
                {
                    const std::size_t chunk_avail =
                        clamp(chunk_remain_, cb0_.size());
                    const auto chunk =
                        buffers::prefix(cb0_.data(), chunk_avail);

                    if(body_limit_remain() < chunk_avail)
                    {
                        ec = BOOST_HTTP_PROTO_ERR(
                            error::body_too_large);
                        st_ = state::reset;
                        return;
                    }

                    switch(how_)
                    {
                    case how::in_place:
                    {
                        auto copied = buffers::copy(
                            cb1_.prepare(cb1_.capacity()),
                            chunk);
                        chunk_remain_ -= copied;
                        body_avail_   += copied;
                        body_total_   += copied;
                        cb0_.consume(copied);
                        cb1_.commit(copied);
                        if(cb1_.capacity() == 0
                            && !chunked_body_ended)
                        {
                            ec = BOOST_HTTP_PROTO_ERR(
                                error::in_place_overflow);
                            return;
                        }
                        break;
                    }
                    case how::sink:
                    {
                        auto sink_rs = sink_->write(
                            chunk, !chunked_body_ended);
                        chunk_remain_ -= sink_rs.bytes;
                        body_total_   += sink_rs.bytes;
                        cb0_.consume(sink_rs.bytes);
                        if(sink_rs.ec.failed())
                        {
                            body_avail_ += 
                                chunk_avail - sink_rs.bytes;
                            ec  = sink_rs.ec;
                            st_ = state::reset;
                            return;
                        }
                        break;
                    }
                    case how::elastic:
                    {
                        if(eb_->max_size() - eb_->size()
                            < chunk_avail)
                        {
                            ec = BOOST_HTTP_PROTO_ERR(
                                error::buffer_overflow);
                            st_ = state::reset;
                            return;
                        }
                        buffers::copy(
                            eb_->prepare(chunk_avail),
                            chunk);
                        chunk_remain_ -= chunk_avail;
                        body_total_   += chunk_avail;
                        cb0_.consume(chunk_avail);
                        eb_->commit(chunk_avail);
                        break;
                    }
                    }

                    if(chunked_body_ended)
                    {
                        set_state_to_complete();
                        return;
                    }
                }
            }
        }
        else
        {
            // non-chunked payload

            const std::size_t payload_avail = [&]()
            {
                auto ret = cb0_.size();
                if(!filter_)
                    ret -= body_avail_;
                if(h_.md.payload == payload::size)
                    return clamp(payload_remain_, ret);
                // payload::eof
                return ret;
            }();

            const bool is_complete = [&]()
            {
                if(h_.md.payload == payload::size)
                    return payload_avail == payload_remain_;
                // payload::eof
                return got_eof_;
            }();

            if(filter_)
            {
                payload_remain_ -= apply_filter(
                    ec, payload_avail, !is_complete);
                if(ec || is_complete)
                    return;
            }
            else
            {
                // plain body

                if(h_.md.payload == payload::to_eof)
                {
                    if(body_limit_remain() < payload_avail)
                    {
                        ec = BOOST_HTTP_PROTO_ERR(
                            error::body_too_large);
                        st_ = state::reset;
                        return;
                    }
                }

                switch(how_)
                {
                case how::in_place:
                {
                    payload_remain_ -= payload_avail;
                    body_avail_     += payload_avail;
                    body_total_     += payload_avail;
                    if(cb0_.capacity() == 0 && !is_complete)
                    {
                        ec = BOOST_HTTP_PROTO_ERR(
                            error::in_place_overflow);
                        return;
                    }
                    break;
                }
                case how::sink:
                {
                    payload_remain_ -= payload_avail;
                    body_total_     += payload_avail;
                    auto sink_rs = sink_->write(
                        buffers::prefix(
                            cb0_.data(),
                            payload_avail),
                        !is_complete);
                    cb0_.consume(sink_rs.bytes);
                    if(sink_rs.ec.failed())
                    {
                        body_avail_ += 
                            payload_avail - sink_rs.bytes;
                        ec  = sink_rs.ec;
                        st_ = state::reset;
                        return;
                    }
                    break;
                }
                case how::elastic:
                {
                    // payload_remain_ and body_total_
                    // are already updated in commit()

                    // cb0_ contains data
                    if(payload_avail != 0)
                    {
                        if(eb_->max_size() - eb_->size()
                            < payload_avail)
                        {
                            ec = BOOST_HTTP_PROTO_ERR(
                                error::buffer_overflow);
                            st_ = state::reset;
                            return;
                        }
                        // only happens when an elastic body
                        // is attached in header_done state
                        buffers::copy(
                            eb_->prepare(payload_avail),
                            cb0_.data());
                        cb0_.consume(payload_avail);
                        eb_->commit(payload_avail);
                        payload_remain_ -= payload_avail;
                        body_total_ += payload_avail;
                    }
                    break;
                }
                }

                if(is_complete)
                {
                    set_state_to_complete();
                    return;
                }
            }

            if(h_.md.payload == payload::size && got_eof_)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::incomplete);
                st_ = state::reset;
                return;
            }

            ec = BOOST_HTTP_PROTO_ERR(
                error::need_data);
            return;
        }

        break;
    }

    case state::set_body:
    case state::complete_in_place:
    {
        auto& body_buf = is_plain() ? cb0_ : cb1_;

        switch(how_)
        {
        case how::in_place:
            return; // no-op
        case how::sink:
        {
            auto rs = sink_->write(
                buffers::prefix(
                    body_buf.data(),
                    body_avail_),
                st_ == state::set_body);
            body_buf.consume(rs.bytes);
            body_avail_ -= rs.bytes;
            if(rs.ec.failed())
            {
                ec  = rs.ec;
                st_ = state::reset;
                return;
            }
            break;
        }
        case how::elastic:
        {
            if(eb_->max_size() - eb_->size()
                < body_avail_)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::buffer_overflow);
                return;
            }
            buffers::copy(
                eb_->prepare(body_avail_),
                body_buf.data());
            body_buf.consume(body_avail_);
            eb_->commit(body_avail_);
            body_avail_ = 0;
            // TODO: expand cb0_ when possible?
            break;
        }
        }

        if(st_ == state::set_body)
        {
            st_ = state::body;
            goto do_body;
        }

        st_ = state::complete;
        break;
    }

    case state::complete:
        break;
    }
}

auto
parser::
pull_body() ->
    const_buffers_type
{
    switch(st_)
    {
    case state::header_done:
        return {};
    case state::body:
    case state::complete_in_place:
        cbp_ = buffers::prefix(
            (is_plain() ? cb0_ : cb1_).data(),
            body_avail_);
        return const_buffers_type(cbp_);
    default:
        detail::throw_logic_error();
    }
}

void
parser::
consume_body(std::size_t n)
{
    switch(st_)
    {
    case state::header_done:
        return;
    case state::body:
    case state::complete_in_place:
        n = clamp(n, body_avail_);
        (is_plain() ? cb0_ : cb1_).consume(n);
        body_avail_ -= n;
        return;
    default:
        detail::throw_logic_error();
    }
}

core::string_view
parser::
body() const noexcept
{
    if(st_ != state::complete_in_place)
    {
        // Precondition violation
        detail::throw_logic_error();
    }

    if(body_avail_ != body_total_)
    {
        // Precondition violation
        detail::throw_logic_error();
    }

    auto cbp = (is_plain() ? cb0_ : cb1_).data();
    BOOST_ASSERT(cbp[1].size() == 0);
    BOOST_ASSERT(cbp[0].size() == body_avail_);
    return core::string_view(
        static_cast<char const*>(cbp[0].data()),
        body_avail_);
}

core::string_view
parser::
release_buffered_data() noexcept
{
    return {};
}

void
parser::
set_body_limit(std::uint64_t n)
{
    switch(st_)
    {
    case state::header:
    case state::header_done:
        body_limit_ = n;
        break;
    case state::complete_in_place:
        // only allowed for empty bodies
        if(body_total_ == 0)
            break;
        BOOST_FALLTHROUGH;
    default:
        // set body_limit before parsing the body
        detail::throw_logic_error();
    }
}

//------------------------------------------------
//
// Implementation
//
//------------------------------------------------

void
parser::
on_set_body() noexcept
{
    BOOST_ASSERT(
        st_ == state::header_done ||
        st_ == state::body ||
        st_ == state::complete_in_place);

    nprepare_ = 0; // invalidate

    if(st_ == state::body)
        st_ = state::set_body;
}

std::size_t
parser::
apply_filter(
    system::error_code& ec,
    std::size_t payload_avail,
    bool more)
{
    std::size_t p0 = payload_avail;
    for(;;)
    {
        if(payload_avail == 0 && more)
            break;

        auto f_rs = [&](){
            BOOST_ASSERT(filter_ != nullptr);
            if(how_ == how::elastic)
            {
                std::size_t n = clamp(body_limit_remain());
                n = clamp(n, svc_.cfg.min_buffer);
                n = clamp(n, eb_->max_size() - eb_->size());

                // fill capacity first to avoid
                // an allocation
                std::size_t avail = 
                    eb_->capacity() - eb_->size();
                if(avail != 0)
                    n = clamp(n, avail);

                return filter_->process(
                    eb_->prepare(n),
                    buffers::prefix(
                        cb0_.data(),
                        payload_avail),
                    more);
            }
            else // in-place and sink 
            {
                std::size_t n = clamp(body_limit_remain());
                n = clamp(n, cb1_.capacity());

                return filter_->process(
                    buffers::mutable_buffer_span{ cb1_.prepare(n) },
                    buffers::prefix(
                        cb0_.data(),
                        payload_avail),
                    more);
            }
        }();

        cb0_.consume(f_rs.in_bytes);
        payload_avail -= f_rs.in_bytes;
        body_total_   += f_rs.out_bytes;

        switch(how_)
        {
        case how::in_place:
        {
            cb1_.commit(f_rs.out_bytes);
            body_avail_ += f_rs.out_bytes;
            if(cb1_.capacity() == 0 &&
                !f_rs.finished && f_rs.in_bytes == 0)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::in_place_overflow);
                goto done;
            }
            break;
        }
        case how::sink:
        {
            cb1_.commit(f_rs.out_bytes);
            auto sink_rs = sink_->write(
                cb1_.data(), !f_rs.finished);
            cb1_.consume(sink_rs.bytes);
            if(sink_rs.ec.failed())
            {
                ec  = sink_rs.ec;
                st_ = state::reset;
                goto done;
            }
            break;
        }
        case how::elastic:
        {
            eb_->commit(f_rs.out_bytes);
            if(eb_->max_size() - eb_->size() == 0 &&
                !f_rs.finished && f_rs.in_bytes == 0)
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::buffer_overflow);
                st_ = state::reset;
                goto done;
            }
            break;
        }
        }

        if(f_rs.ec.failed())
        {
            ec = f_rs.ec;
            st_ = state::reset;
            break;
        }

        if(body_limit_remain() == 0 &&
            !f_rs.finished && f_rs.in_bytes == 0)
        {
            ec = BOOST_HTTP_PROTO_ERR(
                error::body_too_large);
            st_ = state::reset;
            break;
        }

        if(f_rs.finished)
        {
            if(!more)
            {
                st_ = (how_ == how::in_place)
                    ? state::complete_in_place
                    : state::complete;
            }
            break;
        }
    }

done:
    return p0 - payload_avail;
}

detail::header const*
parser::
safe_get_header() const
{
    // headers must be received
    if(! got_header_)
        detail::throw_logic_error();

    return &h_;
}

bool
parser::
is_plain() const noexcept
{
    return ! filter_ &&
        h_.md.payload != payload::chunked;
}

std::uint64_t
parser::
body_limit_remain() const noexcept
{
    return body_limit_ - body_total_;
}

} // http_proto
} // boost

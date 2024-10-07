//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/parser.hpp>
#include <boost/http_proto/rfc/detail/rules.hpp>
#include <boost/http_proto/service/zlib_service.hpp>

#include "detail/filter.hpp"

#include <boost/buffers/algorithm.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/buffers/make_buffer.hpp>

#include <boost/url/grammar/ci_string.hpp>
#include <boost/url/grammar/hexdig_chars.hpp>

#include <boost/assert.hpp>

#include <array>
#include <iostream>
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

namespace {
class inflator_filter
    : public http_proto::detail::filter
{
    zlib::stream& inflator_;

public:
    inflator_filter(
        context& ctx,
        http_proto::detail::workspace& ws,
        bool use_gzip)
        : inflator_{ ctx.get_service<zlib::service>()
            .make_inflator(ws, use_gzip ? 31 : 15) }
    {
    }

    virtual filter::results
    on_process(
        buffers::mutable_buffer out,
        buffers::const_buffer in,
        bool more) override
    {
        auto flush =
            more ? zlib::flush::none : zlib::flush::finish;
        filter::results results;

        for(;;)
        {
            auto params = zlib::params{in.data(), in.size(),
                out.data(), out.size() };
            auto ec = inflator_.write(params, flush);

            results.in_bytes  += in.size() - params.avail_in;
            results.out_bytes += out.size() - params.avail_out;

            // TODO: limit the cases where buf_err is valid
            if( ec.failed() &&
                ec != zlib::error::buf_err )
            {
                results.ec = ec;
                return results;
            }

            if( ec == zlib::error::stream_end )
            {
                results.finished = true;
                return results;
            }

            in  = buffers::suffix(in, params.avail_in);
            out = buffers::suffix(out, params.avail_out);

            if( in.size() == 0 || out.size() == 0)
                return results;
        }
    }
};

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

        // swap with the second range
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
    empty() const noexcept
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

static
system::result<std::uint64_t>
parse_hex(chained_sequence& cs) noexcept
{
    std::uint64_t v   = 0;
    std::size_t init_size = cs.size();
    while(!cs.empty())
    {
        auto n = grammar::hexdig_value(cs.value());
        if(n < 0)
        {
            if(init_size == cs.size())
                BOOST_HTTP_PROTO_RETURN_EC(
                    error::bad_payload);
            return v;
        }

        // at least 4 significant bits are free
        if(v > (std::numeric_limits<std::uint64_t>::max)() >> 4)
            BOOST_HTTP_PROTO_RETURN_EC(
                error::bad_payload);

        v = (v << 4) | static_cast<std::uint64_t>(n);
        cs.next();
    }
    BOOST_HTTP_PROTO_RETURN_EC(
        error::need_data);
}

static
system::result<void>
find_eol(chained_sequence& cs) noexcept
{
    while(!cs.empty())
    {
        if(cs.value() == '\r')
        {
            if(!cs.next())
                break;
            if(cs.value() != '\n')
                BOOST_HTTP_PROTO_RETURN_EC(
                    error::bad_payload);
            cs.next();
            return {};
        }
        cs.next();
    }
    BOOST_HTTP_PROTO_RETURN_EC(
        error::need_data);
}

static
system::result<void>
parse_eol(chained_sequence& cs) noexcept
{
    if(cs.size() >= 2)
    {
        // we are sure size is at least 2
        if(cs.value() == '\r' && *cs.next() == '\n')
        {
            cs.next();
            return {};
        }
        BOOST_HTTP_PROTO_RETURN_EC(
            error::bad_payload);
    }
    BOOST_HTTP_PROTO_RETURN_EC(
        error::need_data);
}

static
system::result<void>
skip_trailer_headers(chained_sequence& cs) noexcept
{
    while(!cs.empty())
    {
        if(cs.value() == '\r')
        {
            if(!cs.next())
                break;
            if(cs.value() != '\n')
                BOOST_HTTP_PROTO_RETURN_EC(
                    error::bad_payload);
            cs.next();
            return {};
        }
        // skip to the end of field
        auto rv = find_eol(cs);
        if(rv.has_error())
            return rv.error();
    }
    BOOST_HTTP_PROTO_RETURN_EC(
        error::need_data);
}

template<class UInt>
std::size_t
clamp(UInt x, std::size_t limit) noexcept
{
    if(x >= limit)
        return limit;
    return static_cast<std::size_t>(x);
}

template <class ElasticBuffer>
system::result<void>
parse_chunked(
    detail::filter* filter,
    buffers::circular_buffer& input,
    ElasticBuffer& output,
    std::uint64_t& chunk_remain_,
    std::uint64_t& body_avail_,
    bool& needs_chunk_close_,
    bool& trailer_headers_) noexcept
{
    for(;;)
    {
        if(chunk_remain_ == 0)
        {
            auto cs = chained_sequence(input.data());

            if(trailer_headers_)
            {
                auto rs = skip_trailer_headers(cs);
                if(rs.has_error())
                    return rs;
                input.consume(input.size() - cs.size());
                return {};
            }

            if(needs_chunk_close_)
            {
                auto rs = parse_eol(cs);
                if(rs.has_error())
                    return rs;
            }

            auto chunk_size = parse_hex(cs);
            if(chunk_size.has_error())
                return chunk_size.error();

            // chunk extensions are skipped
            auto rs = find_eol(cs);
            if(rs.has_error())
                return rs;

            input.consume(input.size() - cs.size());
            chunk_remain_ = chunk_size.value();

            needs_chunk_close_ = true;
            if(chunk_remain_ == 0)
            {
                trailer_headers_ = true;
                continue;
            }
        }

        if( input.size() == 0 )
            BOOST_HTTP_PROTO_RETURN_EC(
                error::need_data);

        if( output.capacity() == 0 )
            BOOST_HTTP_PROTO_RETURN_EC(
                error::in_place_overflow);

        auto chunk = buffers::prefix(input.data(),
            clamp(chunk_remain_, input.size()));

        if( filter )
        {
            // TODO: gather available chunks and provide
            // them as a const_buffer_span
            auto rs = filter->process(
                output.prepare(output.capacity()),
                chunk,
                !trailer_headers_);

            if( rs.ec.failed() )
                return rs.ec;

            chunk_remain_ -= rs.in_bytes;
            input.consume(rs.in_bytes);
            output.commit(rs.out_bytes);
            body_avail_ += rs.out_bytes;

            if( rs.finished && chunk_remain_ != 0 )
                BOOST_HTTP_PROTO_RETURN_EC(
                    error::bad_payload);
        }
        else
        {
            auto copied = buffers::buffer_copy(
                output.prepare(output.capacity()), chunk);
            chunk_remain_ -= copied;
            input.consume(copied);
            output.commit(copied);
            body_avail_ += copied;
        }
    }
}
} // namespace

class parser_service
    : public service
{
public:
    parser::config_base cfg;
    std::size_t space_needed = 0;
    std::size_t max_codec = 0;
    zlib::service const* zlib_svc = nullptr;

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
            auto const n = ctx.get_service<
                zlib::service>().inflator_space_needed(15);
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
        if(is_plain() && (how_ == how::in_place))
            cb0_.consume(
                static_cast<std::size_t>(body_avail_));

        if(cb0_.size() > 0)
        {
            // move unused octets to front

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
        svc_.max_overread() - leftover);

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
    chunk_remain_ = 0;
    needs_chunk_close_ = false;
    trailer_headers_ = false;
    filter_ = nullptr;
    body_avail_ = 0;
    body_total_ = 0;
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
            auto n = cb0_.capacity();
            if( n > svc_.cfg.max_prepare)
                n = svc_.cfg.max_prepare;
            mbp_ = cb0_.prepare(n);
            nprepare_ = n;
            return mutable_buffers_type(mbp_);
        }

        // plain payload

        if(how_ == how::in_place)
        {
            auto n = cb0_.capacity();
            if( n > svc_.cfg.max_prepare)
                n = svc_.cfg.max_prepare;

            // TODO: payload_remain_ + svc_.max_overread() might overflow
            if( h_.md.payload == payload::size &&
                n > payload_remain_ + svc_.max_overread())
                n = static_cast<size_t>(
                    payload_remain_ + svc_.max_overread());

            mbp_ = cb0_.prepare(n);
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
        detail::throw_logic_error();
    }

    case state::set_body:
    {
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
                if(n < payload_remain_)
                {
                    body_avail_ += n;
                    body_total_ += n;
                    payload_remain_ -= n;
                    break;
                }
                body_avail_ += payload_remain_;
                body_total_ += payload_remain_;
                payload_remain_ = 0;
                st_ = state::complete;
                break;
            }

            BOOST_ASSERT(
                h_.md.payload == payload::to_eof);
            body_avail_ += n;
            body_total_ += n;
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

        if( h_.md.payload == payload::chunked )
        {
            if( how_ == how::in_place )
            {
                // TODO: parse_chunked should be a member function
                auto rv = parse_chunked(
                    filter_, cb0_, cb1_, chunk_remain_,
                    body_avail_, needs_chunk_close_, trailer_headers_);

                // TODO: check for body_limit

                if(rv.has_error()) // including error::need_data
                    ec = rv.error();
                else
                    st_ = state::complete;
                return;
            }
            else
            {
                // TODO
                detail::throw_logic_error();
            }
        }
        else if( filter_ )
        {
            if( how_ == how::in_place )
            {
                if( body_buf_->capacity() == 0 )
                {
                    // in_place buffer limit
                    ec = BOOST_HTTP_PROTO_ERR(
                        error::in_place_overflow);
                    return;
                }

                auto rs = [&]() -> detail::filter::results
                {
                    if( h_.md.payload == payload::size )
                    {
                        auto rv = filter_->process(
                            body_buf_->prepare(body_buf_->capacity()),
                            buffers::prefix(cb0_.data(), clamp(
                                payload_remain_, cb0_.size())),
                            cb0_.size() < payload_remain_);

                        payload_remain_ -= rv.in_bytes;
                        return rv;
                    }
                    BOOST_ASSERT(h_.md.payload == payload::to_eof);
                    return filter_->process(
                        body_buf_->prepare(body_buf_->capacity()),
                        cb0_.data(),
                        !got_eof_);
                }();

                ec           = rs.ec;
                body_avail_ += rs.out_bytes;
                body_total_ += rs.out_bytes;
                cb0_.consume(rs.in_bytes);
                body_buf_->commit(rs.out_bytes);

                if( body_avail_ > svc_.cfg.body_limit )
                {
                    ec  = BOOST_HTTP_PROTO_ERR(
                        error::body_too_large);
                    st_ = state::reset; // unrecoverable
                    return;
                }

                if( ec.failed() )
                {
                    st_ = state::reset; // unrecoverable
                    return;
                }

                if( rs.finished )
                {
                    if( !got_eof_ &&
                        h_.md.payload == payload::to_eof )
                    {
                        ec = BOOST_HTTP_PROTO_ERR(
                            error::need_data);
                        return;
                    }

                    st_ = state::complete;
                    return;
                }

                if( got_eof_ )
                {
                    if( body_buf_->capacity() == 0 )
                    {
                        ec  = BOOST_HTTP_PROTO_ERR(
                            error::in_place_overflow);
                        return;
                    }
                    ec  = BOOST_HTTP_PROTO_ERR(
                        error::incomplete);
                    st_ = state::reset; // unrecoverable
                    return;
                }

                ec = BOOST_HTTP_PROTO_ERR(
                    error::need_data);
                return;
            }
            else
            {
                // TODO
                detail::throw_logic_error();
            }
        }

        if(how_ == how::in_place)
        {
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
            if( body_total_ > svc_.cfg.body_limit )
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::body_too_large);
                st_ = state::reset; // unrecoverable
                return;
            }
            if( ! got_eof_ )
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
        }
    }
    }
}

//------------------------------------------------

auto
parser::
pull_body() ->
    const_buffers_type
{
    switch(st_)
    {
    case state::body:
    case state::complete:
        if(how_ != how::in_place)
            detail::throw_logic_error();
        cbp_ = buffers::prefix(body_buf_->data(),
            static_cast<std::size_t>(body_avail_));
        return const_buffers_type{ cbp_ };
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
    case state::body:
    case state::complete:
        if(how_ != how::in_place)
            detail::throw_logic_error();
        BOOST_ASSERT(n <= body_avail_);
        body_buf_->consume(n);
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
        BOOST_ASSERT(cbp[0].size() == body_avail_);
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
    return ! filter_ &&
        h_.md.payload !=
            payload::chunked;
}

// Called immediately after complete headers are received
// to setup the circular buffers for subsequent operations.
// We leave fb_ as-is to indicate whether any data was
// received before eof.
//
void
parser::
on_headers(
    system::error_code& ec)
{
    // overread currently includes any and all octets that
    // extend beyond the current end of the header
    // this can include associated body octets for the
    // current message or octets of the next message in the
    // stream, e.g. pipelining is being used
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
        head_response_ )
    {
        // set cb0_ to overread
        cb0_ = {
            ws_.data(),
            overread + fb_.capacity(),
            overread };
        body_buf_ = &cb0_;
        st_ = state::complete;
        return;
    }

    auto cap = fb_.capacity() + overread +
        svc_.cfg.min_buffer;

    // reserve body buffers first, as the decoder
    // must be installed after them.
    auto const p = ws_.reserve_front(cap);

    if( svc_.cfg.apply_deflate_decoder &&
        h_.md.content_encoding.encoding == encoding::deflate )
    {
        filter_ = &ws_.emplace<inflator_filter>(
            ctx_, ws_, false);
    }
    else if( svc_.cfg.apply_gzip_decoder &&
        h_.md.content_encoding.encoding == encoding::gzip )
    {
        filter_ = &ws_.emplace<inflator_filter>(
            ctx_, ws_, true);
    }
    else
    {
        cap += svc_.max_codec;
        ws_.reserve_front(svc_.max_codec);
    }

    if( !filter_ &&
        h_.md.payload != payload::chunked )
    {
        cb0_ = { p, cap, overread };
        body_buf_ = &cb0_;
        body_avail_ = cb0_.size();

        if( h_.md.payload == payload::size )
        {
            if( h_.md.payload_size >
                svc_.cfg.body_limit )
            {
                ec = BOOST_HTTP_PROTO_ERR(
                    error::body_too_large);
                st_ = state::reset; // unrecoverable
                return;
            }

            if( body_avail_ >= h_.md.payload_size )
                body_avail_ = h_.md.payload_size;

            payload_remain_ =
                h_.md.payload_size - body_avail_;
        }

        body_total_ = body_avail_;
        st_ = state::body;
        return;
    }

    if( h_.md.payload == payload::size )
        payload_remain_ = h_.md.payload_size;

    auto const n0 = overread > svc_.cfg.min_buffer ?
        overread : svc_.cfg.min_buffer;
    auto const n1 = cap - n0;

    cb0_ = { p      , n0, overread };
    cb1_ = { p + n0 , n1 };
    body_buf_ = &cb1_;

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

    // TODO: expand cb_0?

    // TODO: we need a better way to recover the state.
    if( !filter_ &&
        h_.md.payload == payload::size &&
        body_total_ == h_.md.payload_size)
    {
        st_ = state::complete;
        return;
    }

    st_ = state::body;
}

} // http_proto
} // boost

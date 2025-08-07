//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2024 Christian Mazakas
// Copyright (c) 2024 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/message_view_base.hpp>
#include <boost/http_proto/serializer.hpp>

#include "src/detail/brotli_filter_base.hpp"
#include "src/detail/zlib_filter_base.hpp"

#include <boost/buffers/copy.hpp>
#include <boost/buffers/prefix.hpp>
#include <boost/buffers/sans_prefix.hpp>
#include <boost/buffers/sans_suffix.hpp>
#include <boost/buffers/size.hpp>
#include <boost/core/bit.hpp>
#include <boost/core/ignore_unused.hpp>
#include <boost/rts/brotli/encode.hpp>
#include <boost/rts/zlib/compression_method.hpp>
#include <boost/rts/zlib/compression_strategy.hpp>
#include <boost/rts/zlib/deflate.hpp>
#include <boost/rts/zlib/error.hpp>
#include <boost/rts/zlib/flush.hpp>

#include <stddef.h>

namespace boost {
namespace http_proto {

namespace {

constexpr
std::size_t
crlf_len = 2;

constexpr
std::uint8_t
chunk_header_len(
    std::size_t max_chunk_size) noexcept
{
    return
        static_cast<uint8_t>(
            (core::bit_width(max_chunk_size) + 3) / 4 +
            crlf_len);
};

constexpr
std::size_t
final_chunk_len = 1 + crlf_len + crlf_len;

void
write_chunk_header(
    const buffers::mutable_buffer_pair& mbs,
    std::size_t size) noexcept
{
    static constexpr char hexdig[] =
        "0123456789ABCDEF";
    char buf[18];
    auto p = buf + 16;
    auto const n = buffers::size(mbs);
    for(std::size_t i = n - crlf_len; i--;)
    {
        *--p = hexdig[size & 0xf];
        size >>= 4;
    }
    buf[16] = '\r';
    buf[17] = '\n';
    auto copied = buffers::copy(
        mbs,
        buffers::const_buffer(p, n));
    ignore_unused(copied);
    BOOST_ASSERT(copied == n);
}

void
write_crlf(
    const buffers::mutable_buffer_pair& mbs) noexcept
{
    auto n = buffers::copy(
        mbs,
        buffers::const_buffer("\r\n", 2));
    ignore_unused(n);
    BOOST_ASSERT(n == 2);
}

void
write_final_chunk(
    const buffers::mutable_buffer_pair& mbs) noexcept
{
    auto n = buffers::copy(
        mbs,
        buffers::const_buffer(
            "0\r\n\r\n", 5));
    ignore_unused(n);
    BOOST_ASSERT(n == 5);
}

//------------------------------------------------

class zlib_filter
    : public detail::zlib_filter_base
{
    rts::zlib::deflate_service& svc_;

public:
    zlib_filter(
        const rts::context* ctx,
        http_proto::detail::workspace& ws,
        int comp_level,
        int window_bits,
        int mem_level)
        : zlib_filter_base(ws)
        , svc_(ctx->get_service<rts::zlib::deflate_service>())
    {
        system::error_code ec = static_cast<rts::zlib::error>(svc_.init2(
            strm_,
            comp_level,
            rts::zlib::deflated,
            window_bits,
            mem_level,
            rts::zlib::default_strategy));
        if(ec != rts::zlib::error::ok)
            detail::throw_system_error(ec);
    }

private:
    virtual
    std::size_t
    min_out_buffer() const noexcept override
    {
        // Prevents deflate from producing
        // zero output due to small buffer
        return 8;
    }

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
            svc_.deflate(
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
    rts::brotli::encode_service& svc_;
    rts::brotli::encoder_state* state_;

public:
    brotli_filter(
        const rts::context* ctx,
        http_proto::detail::workspace&,
        std::uint32_t comp_quality,
        std::uint32_t comp_window)
        : svc_(ctx->get_service<rts::brotli::encode_service>())
    {
        // TODO: use custom allocator
        state_ = svc_.create_instance(nullptr, nullptr, nullptr);
        if(!state_)
            detail::throw_bad_alloc();
        using encoder_parameter = rts::brotli::encoder_parameter;
        svc_.set_parameter(state_, encoder_parameter::quality, comp_quality);
        svc_.set_parameter(state_, encoder_parameter::lgwin, comp_window);
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

        using encoder_operation = 
            rts::brotli::encoder_operation;

        bool rs = svc_.compress_stream(
            state_,
            more ? encoder_operation::process : encoder_operation::finish,
            &available_in,
            &next_in,
            &available_out,
            &next_out,
            nullptr);

        results rv;
        rv.in_bytes  = in.size()  - available_in;
        rv.out_bytes = out.size() - available_out;
        rv.finished  = svc_.is_finished(state_);

        // TODO: use proper error code
        if(rs == false)
            rv.ec = error::bad_payload;

        return rv;
    }
};

} // namespace

namespace detail {

class serializer_service
    : public rts::service
{
public:
    serializer::config cfg;
    std::size_t space_needed = 0;

    serializer_service(
        rts::context&,
        serializer::config const& cfg_)
        : cfg(cfg_)
    {
        space_needed += cfg.payload_buffer;
        space_needed += cfg.max_type_erase;

        if(cfg.apply_deflate_encoder || cfg.apply_gzip_encoder)
        {
            // TODO: Account for the number of allocations and
            // their overhead in the workspace.

            // https://www.zlib.net/zlib_tech.html
            space_needed +=
                (1 << (cfg.zlib_window_bits + 2)) +
                (1 << (cfg.zlib_mem_level + 9)) +
                (6 * 1024) +
                #ifdef __s390x__
                5768 +
                #endif
                detail::workspace::space_needed<zlib_filter>();
        }
    }
};

} // detail

//------------------------------------------------

void
install_serializer_service(
    rts::context& ctx,
    serializer::config const& cfg)
{
    ctx.make_service<
        detail::serializer_service>(cfg);
}

//------------------------------------------------

serializer::
~serializer()
{
}

serializer::
serializer(
    serializer&& other) noexcept
    : ctx_(other.ctx_)
    , svc_(other.svc_)
    , ws_(std::move(other.ws_))
    , filter_(other.filter_)
    , buf_gen_(other.buf_gen_)
    , source_(other.source_)
    , out_(other.out_)
    , in_(other.in_)
    , prepped_(other.prepped_)
    , tmp_(other.tmp_)
    , state_(other.state_)
    , style_(other.style_)
    , chunk_header_len_(other.chunk_header_len_)
    , more_input_(other.more_input_)
    , is_chunked_(other.is_chunked_)
    , needs_exp100_continue_(other.needs_exp100_continue_)
    , filter_done_(other.filter_done_)
{
    // TODO: make state a class type and default
    // move ctor and assignment.
    
    // TODO: use an indirection for stream
    // interface so it stays valid after move.

    other.state_ = state::start;
}

serializer&
serializer::
operator=(
    serializer&& other) noexcept
{
    ctx_ = other.ctx_;
    svc_ = other.svc_;
    ws_ = std::move(other.ws_);
    filter_ = other.filter_;
    buf_gen_ = other.buf_gen_;
    source_ = other.source_;
    out_ = other.out_;
    in_ = other.in_;
    prepped_ = other.prepped_;
    tmp_ = other.tmp_;
    state_ = other.state_;
    style_ = other.style_;
    chunk_header_len_ = other.chunk_header_len_;
    more_input_ = other.more_input_;
    is_chunked_ = other.is_chunked_;
    needs_exp100_continue_ = other.needs_exp100_continue_;
    filter_done_ = other.filter_done_;

    other.state_ = state::start;

    return *this;
}

serializer::
serializer(const rts::context& ctx)
    : ctx_(&ctx)
    , svc_(&ctx_->get_service<
        detail::serializer_service>())
    , ws_(svc_->space_needed)
{
}

void
serializer::
reset() noexcept
{
    ws_.clear();
    state_ = state::start;
}

//------------------------------------------------

auto
serializer::
prepare() ->
    system::result<const_buffers_type>
{
    // Precondition violation
    if(state_ < state::header)
        detail::throw_logic_error();

    // Expect: 100-continue
    if(needs_exp100_continue_)
    {
        if(!is_header_done())
            return const_buffers_type(
                prepped_.begin(),
                1); // limit to header

        needs_exp100_continue_ = false;

        BOOST_HTTP_PROTO_RETURN_EC(
            error::expect_100_continue);
    }

    if(!filter_)
    {
        switch(style_)
        {
        case style::empty:
            break;

        case style::buffers:
        {
            // add more buffers if prepped_ is half empty.
            if(more_input_ &&
                prepped_.capacity() >= prepped_.size())
            {
                prepped_.slide_to_front();
                while(prepped_.capacity() != 0)
                {
                    auto buf = buf_gen_->next();
                    if(buf.size() != 0)
                    {
                        prepped_.append(buf);
                    }
                    else // buf_gen_ is empty
                    {
                        // append crlf and final chunk
                        if(is_chunked_)
                        {
                            prepped_.append(tmp_);
                            more_input_ = false;
                        }
                        break;
                    }
                }
                if(buf_gen_->is_empty() && !is_chunked_)
                    more_input_ = false;
            }
            return const_buffers_type(
                prepped_.begin(),
                prepped_.size());
        }

        case style::source:
        {
            if(out_capacity() == 0 || !more_input_)
                break;

            const auto rs = source_->read(
                out_prepare());

            out_commit(rs.bytes);

            if(rs.ec.failed())
            {
                ws_.clear();
                state_ = state::reset;
                return rs.ec;
            }

            if(rs.finished)
            {
                more_input_ = false;
                out_finish();
            }

            break;
        }

        case style::stream:
            if(out_.size() == 0 && is_header_done() && more_input_)
                BOOST_HTTP_PROTO_RETURN_EC(
                    error::need_data);
            break;
        }
    }
    else // filter
    {
        switch(style_)
        {
        case style::empty:
        {
            if(out_capacity() == 0 || filter_done_)
                break;

            const auto rs = filter_->process(
                buffers::mutable_buffer_span(
                    out_prepare()),
                {}, // empty input
                false);

            if(rs.ec.failed())
            {
                ws_.clear();
                state_ = state::reset;
                return rs.ec;
            }

            out_commit(rs.out_bytes);

            if(rs.finished)
            {
                filter_done_ = true;
                out_finish();
            }

            break;
        }

        case style::buffers:
        {
            while(out_capacity() != 0 && !filter_done_)
            {
                if(more_input_ && tmp_.size() == 0)
                {
                    tmp_ = buf_gen_->next();
                    if(tmp_.size() == 0) // buf_gen_ is empty
                        more_input_ = false;
                }

                const auto rs = filter_->process(
                    buffers::mutable_buffer_span(
                        out_prepare()),
                    { tmp_, {} },
                    more_input_);

                if(rs.ec.failed())
                {
                    ws_.clear();
                    state_ = state::reset;
                    return rs.ec;
                }

                tmp_ = buffers::sans_prefix(
                    tmp_, rs.in_bytes);
                out_commit(rs.out_bytes);

                if(rs.out_short)
                    break;

                if(rs.finished)
                {
                    filter_done_ = true;
                    out_finish();
                }
            }
            break;
        }

        case style::source:
        {
            while(out_capacity() != 0 && !filter_done_)
            {
                if(more_input_ && in_.capacity() != 0)
                {
                    const auto rs = source_->read(
                        in_.prepare(in_.capacity()));
                    if(rs.ec.failed())
                    {
                        ws_.clear();
                        state_ = state::reset;
                        return rs.ec;
                    }
                    if(rs.finished)
                        more_input_ = false;
                    in_.commit(rs.bytes);
                }

                const auto rs = filter_->process(
                    buffers::mutable_buffer_span(
                        out_prepare()),
                    in_.data(),
                    more_input_);

                if(rs.ec.failed())
                {
                    ws_.clear();
                    state_ = state::reset;
                    return rs.ec;
                }

                in_.consume(rs.in_bytes);
                out_commit(rs.out_bytes);

                if(rs.out_short)
                    break;

                if(rs.finished)
                {
                    filter_done_ = true;
                    out_finish();
                }
            }
            break;
        }

        case style::stream:
        {
            if(out_capacity() == 0 || filter_done_)
                break;

            const auto rs = filter_->process(
                buffers::mutable_buffer_span(
                    out_prepare()),
                in_.data(),
                more_input_);

            if(rs.ec.failed())
            {
                ws_.clear();
                state_ = state::reset;
                return rs.ec;
            }

            in_.consume(rs.in_bytes);
            out_commit(rs.out_bytes);

            if(rs.finished)
            {
                filter_done_ = true;
                out_finish();
            }

            if(out_.size() == 0 && is_header_done() && more_input_)
                BOOST_HTTP_PROTO_RETURN_EC(
                    error::need_data);
            break;
        }
        }
    }

    prepped_.reset(!is_header_done());
    const auto cbp = out_.data();
    if(cbp[0].size() != 0)
        prepped_.append(cbp[0]);
    if(cbp[1].size() != 0)
        prepped_.append(cbp[1]);

    return const_buffers_type(
        prepped_.begin(),
        prepped_.size());
}

void
serializer::
consume(
    std::size_t n)
{
    // Precondition violation
    if(state_ < state::header)
        detail::throw_logic_error();

    if(!is_header_done())
    {
        const auto header_remain =
            prepped_[0].size();
        if(n < header_remain)
        {
            prepped_.consume(n);
            return;
        }
        n -= header_remain;
        prepped_.consume(header_remain);
        state_ = state::body;
    }

    prepped_.consume(n);

    // no-op when out_ is not in use
    out_.consume(n);

    if(!prepped_.empty())
        return;

    if(more_input_)
        return;

    if(filter_ && !filter_done_)
        return;

    if(needs_exp100_continue_)
        return;

    // ready for next message
    reset();
}

//------------------------------------------------

detail::array_of_const_buffers
serializer::
make_array(std::size_t n)
{
    if(n > std::numeric_limits<std::uint16_t>::max())
        detail::throw_length_error();

    return {
        ws_.push_array(n,
            buffers::const_buffer{}),
        static_cast<std::uint16_t>(n) };
}

void
serializer::
start_init(
    message_view_base const& m)
{
    // Precondition violation
    if(state_ != state::start)
        detail::throw_logic_error();

    // TODO: to hold strong exception guarantee
    // `state_` should be set to state::start if an
    // exception is thrown during the start operation.
    state_ = state::header;

    // VFALCO what do we do with
    // metadata error code failures?
    // m.ph_->md.maybe_throw();

    auto const& md = m.metadata();
    needs_exp100_continue_ = md.expect.is_100_continue;

    // Transfer-Encoding
    is_chunked_ = md.transfer_encoding.is_chunked;

    // Content-Encoding
    switch (md.content_encoding.coding)
    {
    case content_coding::deflate:
        if(!svc_->cfg.apply_deflate_encoder)
            goto no_filter;
        filter_ = &ws_.emplace<zlib_filter>(
            ctx_,
            ws_,
            svc_->cfg.zlib_comp_level,
            svc_->cfg.zlib_window_bits,
            svc_->cfg.zlib_mem_level);
        filter_done_ = false;
        break;

    case content_coding::gzip:
        if(!svc_->cfg.apply_gzip_encoder)
            goto no_filter;
        filter_ = &ws_.emplace<zlib_filter>(
            ctx_,
            ws_,
            svc_->cfg.zlib_comp_level,
            svc_->cfg.zlib_window_bits + 16,
            svc_->cfg.zlib_mem_level);
        filter_done_ = false;
        break;

    case content_coding::br:
        if(!svc_->cfg.apply_brotli_encoder)
            goto no_filter;
        filter_ = &ws_.emplace<brotli_filter>(
            ctx_,
            ws_,
            svc_->cfg.brotli_comp_quality,
            svc_->cfg.brotli_comp_window);
        filter_done_ = false;
        break;

    no_filter:
    default:
        filter_ = nullptr;
        break;
    }
}

void
serializer::
start_empty(
    message_view_base const& m)
{
    start_init(m);
    style_ = style::empty;

    prepped_ = make_array(
        1 + // header
        2); // out buffer pairs

    out_init();

    if(!filter_)
        out_finish();

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    more_input_ = false;
}

void
serializer::
start_buffers(
    message_view_base const& m)
{
    using mutable_buffer =
        buffers::mutable_buffer;

    // start_init() already called 
    style_ = style::buffers;

    if(!filter_)
    {
        // limit the batch size. any remaining buffers will
        // be appended to incrementally in subsequent calls
        // to the prepare function.
        const auto batch_size = (std::min)(
            std::size_t{ 16 },
            buf_gen_->count());

        // none-chunked
        if(!is_chunked_)
        {
            prepped_ = make_array(
                1 +          // header
                batch_size); // buffers

            prepped_[0] = { m.ph_->cbuf, m.ph_->size };
            std::generate(
                prepped_.begin() + 1,
                prepped_.end(),
                [this](){ return buf_gen_->next(); });
            more_input_ = !buf_gen_->is_empty();
            return;
        }

        // chunked with length 0
        if(batch_size == 0)
        {
            prepped_ = make_array(
                1 + // header
                1); // final chunk

            mutable_buffer final_chunk = {
                ws_.reserve_front(
                    final_chunk_len),
                final_chunk_len };
            write_final_chunk(
                { final_chunk, {} });

            prepped_[0] = { m.ph_->cbuf, m.ph_->size };
            prepped_[1] = final_chunk;
            more_input_ = false;
            return;
        }

        // write all buffers as a single chunk, since
        // the total size is known in advance.

        auto const buf_size = buf_gen_->size();
        auto const header_len = 
            chunk_header_len(buf_size);

        mutable_buffer chunk_header = {
            ws_.reserve_front(header_len),
            header_len };

        write_chunk_header({ chunk_header, {} }, buf_size);

        mutable_buffer crlf_and_final_chunk = {
                ws_.reserve_front(
                    crlf_len + final_chunk_len),
                crlf_len + final_chunk_len };

        write_crlf({
            buffers::prefix(
                crlf_and_final_chunk,
                crlf_len)
            , {} });

        write_final_chunk({
            buffers::sans_prefix(
                crlf_and_final_chunk,
                crlf_len)
            , {}});

        prepped_ = make_array(
            1 + // header
            1 + // chunk header
            batch_size + // buffers
            1); // buffer or (crlf and final chunk)

        prepped_[0] = { m.ph_->cbuf, m.ph_->size };
        prepped_[1] = chunk_header;
        std::generate(
            prepped_.begin() + 2,
            prepped_.end() - 1,
            [this](){ return buf_gen_->next(); });

        more_input_ = !buf_gen_->is_empty();
        // assign the last slot
        if(more_input_)
        {
            prepped_[prepped_.size() - 1] =
                buf_gen_->next();

            // deferred until buf_gen_ is drained
            tmp_ = crlf_and_final_chunk;
        }
        else
        {
            prepped_[prepped_.size() - 1] =
                crlf_and_final_chunk;
        }
        return;
    }

    // filter

    prepped_ = make_array(
        1 + // header
        2); // out buffer pairs

    out_init();

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    tmp_ = {};
    more_input_ = true;
}

void
serializer::
start_source(
    message_view_base const& m)
{
    // start_init() already called 
    style_ = style::source;

    prepped_ = make_array(
        1 + // header
        2); // out buffer pairs

    if(filter_)
    {
        // TODO: smarter buffer distribution
        auto const n = (ws_.size() - 1) / 2;
        in_ = { ws_.reserve_front(n), n };
    }

    out_init();

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    more_input_ = true;
}

auto
serializer::
start_stream(
    message_view_base const& m) ->
        stream
{
    start_init(m);
    style_ = style::stream;

    prepped_ = make_array(
        1 + // header
        2); // out buffer pairs

    if(filter_)
    {
        // TODO: smarter buffer distribution
        auto const n = (ws_.size() - 1) / 2;
        in_ = { ws_.reserve_front(n), n };
    }

    out_init();

    prepped_[0] = { m.ph_->cbuf, m.ph_->size };
    more_input_ = true;
    return stream{ *this };
}

bool
serializer::
is_header_done() const noexcept
{
    return state_ == state::body;
}

void
serializer::
out_init()
{
    // use all the remaining buffer
    auto const n = ws_.size() - 1;
    out_ = { ws_.reserve_front(n), n };
    chunk_header_len_ =
        chunk_header_len(out_.capacity());
    if(out_capacity() == 0)
        detail::throw_length_error();
}

buffers::mutable_buffer_pair
serializer::
out_prepare() noexcept
{
    if(is_chunked_)
    {
        return buffers::sans_suffix(
            buffers::sans_prefix(
                out_.prepare(out_.capacity()),
                chunk_header_len_),
            crlf_len + final_chunk_len );
    }
    return out_.prepare(out_.capacity());
}

void
serializer::
out_commit(
    std::size_t n) noexcept
{
    if(is_chunked_)
    {
        if(n == 0)
            return;

        write_chunk_header(out_.prepare(chunk_header_len_), n);
        out_.commit(chunk_header_len_);

        out_.prepare(n);
        out_.commit(n);

        write_crlf(out_.prepare(crlf_len));
        out_.commit(crlf_len);
    }
    else
    {
        out_.commit(n);
    }
}

std::size_t
serializer::
out_capacity() const noexcept
{
    if(is_chunked_)
    {
        auto const overhead =
            chunk_header_len_ +
            crlf_len +
            final_chunk_len;
        if(out_.capacity() < overhead)
            return 0;
        return out_.capacity() - overhead;
    }
    return out_.capacity();
}

void
serializer::
out_finish() noexcept
{
    if(is_chunked_)
    {
        write_final_chunk(
            out_.prepare(final_chunk_len));
        out_.commit(final_chunk_len);
    }
}

//------------------------------------------------

bool
serializer::
stream::
is_open() const noexcept
{
    return sr_ != nullptr;
}

std::size_t
serializer::
stream::
capacity() const
{
    // Precondition violation
    if(!is_open())
        detail::throw_logic_error();

    if(sr_->filter_)
        return sr_->in_.capacity();

    return sr_->out_capacity();
}

auto
serializer::
stream::
prepare() ->
    mutable_buffers_type
{
    // Precondition violation
    if(!is_open())
        detail::throw_logic_error();

    if(sr_->filter_)
        return sr_->in_.prepare(
            sr_->in_.capacity());

    return sr_->out_prepare();
}

void
serializer::
stream::
commit(std::size_t n)
{
    // Precondition violation
    if(!is_open())
        detail::throw_logic_error();

    // Precondition violation
    if(n > capacity())
        detail::throw_invalid_argument();

    if(sr_->filter_)
        return sr_->in_.commit(n);

    sr_->out_commit(n);
}

void
serializer::
stream::
close() noexcept
{
    if(!is_open())
        return; // no-op;

    if(!sr_->filter_)
        sr_->out_finish();

    sr_->more_input_ = false;
    sr_ = nullptr;
}

serializer::
stream::
~stream()
{
    close();
}

} // http_proto
} // boost

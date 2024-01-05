//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#include <boost/http_proto/message_base.hpp>
#include <boost/http_proto/rfc/list_rule.hpp>
#include <boost/http_proto/rfc/token_rule.hpp>
#include <boost/http_proto/detail/except.hpp>
#include "detail/number_string.hpp"
#include <boost/url/grammar/parse.hpp>
#include <boost/url/grammar/ci_string.hpp>

namespace boost {
namespace http_proto {

void
message_base::
set_payload_size(
    std::uint64_t n)
{
    //if(! is_head_response())
    if(true)
    {
        // comes first for exception safety
        set_content_length(n);

        set_chunked(false);
    }
    else
    {
        // VFALCO ?
    }
}

void
message_base::
set_content_length(
    std::uint64_t n)
{
    set(field::content_length,
        detail::number_string(n));
}

void
message_base::
set_chunked(bool value)
{
    if(value)
    {
        // set chunked
        if(! h_.md.transfer_encoding.is_chunked )
        {
            append(
                field::transfer_encoding,
                "chunked");
            return;
        }
    }
    else
    {
        // clear chunked
        // VFALCO ?
    }
}

void
message_base::
set_keep_alive(bool value)
{
    if(ph_->md.connection.ec.failed())
    {
        // throw? return false?
        return;
    }

    if(ph_->md.connection.count == 0)
    {
        // no Connection field
        switch(ph_->version)
        {
        default:
        case version::http_1_1:
            if(! value)
                set(field::connection, "close");
            break;

        case version::http_1_0:
            if(value)
                set(field::connection, "keep-alive");
            break;
        }
        return;
    }

    // VFALCO TODO iterate in reverse order,
    // and cache the last iterator to use
    // for appending

    // one or more Connection fields
    auto it = begin();
    auto const erase_token =
        [&](core::string_view token)
        {
            while(it != end())
            {
                if(it->id != field::connection)
                {
                    ++it;
                    continue;
                }
                auto rv = grammar::parse(
                    it->value,
                    list_rule(token_rule, 1));
                BOOST_ASSERT(! rv.has_error());
                BOOST_ASSERT(! rv->empty());
                auto itv = rv->begin();
                if(urls::grammar::ci_is_equal(
                    *itv, token))
                {
                    if(rv->size() == 1)
                    {
                        // only one token
                        it = erase(it);
                    }
                    else
                    {
                        // first token matches
                        ++itv;
                        set(it,
                            it->value.substr(
                                (*itv).data() -
                                it->value.data()));
                        ++it;
                    }
                    continue;
                }
                // search remaining tokens
                std::string s = *itv++;
                while(itv != rv->end())
                {
                    if(! urls::grammar::ci_is_equal(
                        *itv, token))
                        s += ", " + std::string(*itv);
                    ++itv;
                }
                set(it, s);
                ++it;
            }
        };
    if(value)
    {
        if(ph_->md.connection.close)
            erase_token("close");
    }
    else
    {
        if(ph_->md.connection.keep_alive)
            erase_token("keep-alive");
    }
    switch(ph_->version)
    {
    default:
    case version::http_1_1:
        if(! value)
        {
            // add one "close" token if needed
            if(! ph_->md.connection.close)
                append(field::connection, "close");
        }
        break;

    case version::http_1_0:
        if(value)
        {
            // add one "keep-alive" token if needed
            if(! ph_->md.connection.keep_alive)
                append(field::connection, "keep-alive");
        }
        break;
    }
}

//------------------------------------------------

char*
message_base::
set_prefix_impl(
    std::size_t n)
{
    if( n > h_.prefix ||
        h_.buf == nullptr)
    {
        // allocate or grow
        if( n > h_.prefix &&
            static_cast<std::size_t>(
                n - h_.prefix) >
            static_cast<std::size_t>(
                max_offset - h_.size))
            detail::throw_length_error();

        auto n0 = detail::header::bytes_needed(
            n + h_.size - h_.prefix,
            h_.count);
        auto buf = new char[n0];
        if(h_.buf != nullptr)
        {
            std::memcpy(
                buf + n,
                h_.buf + h_.prefix,
                h_.size - h_.prefix);
            detail::header::table ft(
                h_.buf + h_.cap);
            h_.copy_table(buf + n0);
            delete[] h_.buf;
        }
        else
        {
            std::memcpy(
                buf + n,
                h_.cbuf + h_.prefix,
                h_.size - h_.prefix);
        }
        h_.buf = buf;
        h_.cbuf = buf;
        h_.size = static_cast<
            offset_type>(h_.size +
                n - h_.prefix);
        h_.prefix = static_cast<
            offset_type>(n);
        h_.cap = n0;
        return h_.buf;
    }

    // shrink
    std::memmove(
        h_.buf + n,
        h_.buf + h_.prefix,
        h_.size - h_.prefix);
    h_.size = static_cast<
        offset_type>(h_.size -
            h_.prefix + n);
    h_.prefix = static_cast<
        offset_type>(n);
    return h_.buf;
}

} // http_proto
} // boost

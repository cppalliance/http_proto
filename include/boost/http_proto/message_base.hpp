//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_MESSAGE_BASE_HPP
#define BOOST_HTTP_PROTO_MESSAGE_BASE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/fields_base.hpp>

namespace boost {
namespace http_proto {

/** Provides message metadata for requests and responses
*/
class BOOST_SYMBOL_VISIBLE
    message_base
    : public fields_base
{
    friend class request;
    friend class request_view;
    friend class response;
    friend class response_view;

    explicit
    message_base(
        detail::kind k) noexcept
        : fields_base(k)
    {
    }

    explicit
    message_base(
        detail::header const& ph) noexcept
        : fields_base(ph)
    {
    }

public:
    //--------------------------------------------
    //
    // Metadata
    //
    //--------------------------------------------

    /** Return metadata about the payload
    */
    http_proto::payload const&
    payload() const noexcept
    {
        return ph_->pay;
    }

    /** Return metadata about the Content-Length field
    */
    auto
    connection() const noexcept ->
        http_proto::connection const&
    {
        return ph_->con;
    }

    /** Return metadata about the Transfer-Encoding field
    */
    auto
    transfer_encoding() const noexcept ->
        http_proto::transfer_encoding const&
    {
        return ph_->te;
    }

    /** Return metadata about the message
    */
    auto
    metadata() const noexcept ->
        http_proto::metadata const&
    {
        return ph_->md;
    }

    //--------------------------------------------

    /** Set the payload size
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_payload_size(
        std::uint64_t n);

    /** Set the Content-Length to the specified value
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_content_length(
        std::uint64_t n);

    /** Set whether the payload is chunked.
    */
    void
    set_chunked(bool value)
    {
        set_chunked_impl(value);
    }

private:
    char* set_prefix_impl(std::size_t);

    BOOST_HTTP_PROTO_DECL
    void
    set_chunked_impl(
        bool value);
};

} // http_proto
} // boost

#endif

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
#include <boost/http_proto/message_view_base.hpp>

namespace boost {
namespace http_proto {

/** Provides message metadata for requests and responses
*/
class BOOST_SYMBOL_VISIBLE
    message_base
    : public fields_base
    , public message_view_base
{
    friend class request;
    friend class response;

    explicit
    message_base(
        detail::kind k) noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(k)
    {
    }

    explicit
    message_base(
        detail::header const& ph) noexcept
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(ph)
    {
    }

public:
    //--------------------------------------------
    //
    // Metadata
    //
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

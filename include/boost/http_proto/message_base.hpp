//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
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

    message_base(
        detail::kind k,
        string_view s)
        : fields_view_base(
            &this->fields_base::h_)
        , fields_base(k, s)
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
    BOOST_HTTP_PROTO_DECL
    void
    set_chunked(bool value);

    /** Set whether the connection should stay open.

        Even when keep-alive is set to true, the
        semantics of the other header fields may
        require the connection to be closed. For
        example when there is no content length
        specified in a response.
    */
    BOOST_HTTP_PROTO_DECL
    void
    set_keep_alive(bool value);

private:
    char* set_prefix_impl(std::size_t);
};

} // http_proto
} // boost

#endif

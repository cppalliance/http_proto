//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_RESPONSE_HPP
#define BOOST_HTTP_PROTO_RESPONSE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/basic_message.hpp>
#include <boost/http_proto/status.hpp>
#include <boost/http_proto/version.hpp>

namespace boost {
namespace http_proto {

/** Container for HTTP requests
*/
class response : public basic_message
{
    status result_ = status::ok;

public:
    BOOST_HTTP_PROTO_DECL
    response();

    BOOST_HTTP_PROTO_DECL
    explicit
    response(
        status result_code,
        http_proto::version http_version =
            http_proto::version::http_1_1,
        string_view reason =
            http_proto::obsolete_reason(status::ok));

    //--------------------------------------------
    //
    // Observers
    //
    //--------------------------------------------

    BOOST_HTTP_PROTO_DECL
    status
    result() const noexcept;

    BOOST_HTTP_PROTO_DECL
    unsigned
    result_int() const noexcept;

    BOOST_HTTP_PROTO_DECL
    string_view
    reason() const noexcept;

    //--------------------------------------------
    //
    // Modifiers
    //
    //--------------------------------------------

    BOOST_HTTP_PROTO_DECL
    void
    status_line(
        status result_code,
        http_proto::version http_version =
            http_proto::version::http_1_1,
        string_view reason = "");

private:
    string_view empty_string()
        const noexcept override;
    void do_clear() noexcept override;
};

} // http_proto
} // boost

#endif

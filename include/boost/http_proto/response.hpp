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

namespace boost {
namespace http_proto {

/** Container for HTTP requests
*/
class response : public basic_message
{
private:
    BOOST_HTTP_PROTO_DECL
    string_view
    default_data() const noexcept;
};

} // http_proto
} // boost

#endif
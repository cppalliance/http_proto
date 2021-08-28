//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_MIME_MIME_HPP
#define BOOST_HTTP_PROTO_MIME_MIME_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

class mime
{
public:
    BOOST_HTTP_PROTO_DECL
    virtual ~mime() noexcept;

    virtual
    string_view
    content_type(
        string_view s) const noexcept = 0;
};

} // http_proto
} // boost

#endif

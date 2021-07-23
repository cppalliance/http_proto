//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_CONTEXT_HPP
#define BOOST_HTTP_PROTO_CONTEXT_HPP

#include <boost/http_proto/detail/config.hpp>

namespace boost {
namespace http_proto {

class context
{
public:
    BOOST_HTTP_PROTO_DECL
    context() noexcept;

    context(context const&) = delete;
    context& operator=(
        context const&) = delete;

};

} // http_proto
} // boost

#endif

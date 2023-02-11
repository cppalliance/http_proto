//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_SERVICE_HPP
#define BOOST_HTTP_PROTO_SERVICE_SERVICE_HPP

#include <boost/http_proto/detail/config.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class services;
#endif

/** Base class for all context services
*/
struct BOOST_SYMBOL_VISIBLE
    service
{
    BOOST_HTTP_PROTO_DECL
    virtual
    ~service() = 0;

#if 0
protected:
    /** Called to perform two-phase initialization
    */
    BOOST_HTTP_PROTO_DECL
    virtual
    void
    start() = 0;
#endif

private:
    friend class services;
};

} // http_proto
} // boost

#endif

//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SINK_HPP
#define BOOST_HTTP_PROTO_SINK_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error_types.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/buffers/const_buffer_pair.hpp>

namespace boost {
namespace http_proto {

/** A sink for body data
*/
struct BOOST_SYMBOL_VISIBLE
    sink
{
    /** Destructor
    */
    BOOST_HTTP_PROTO_DECL
    virtual
    ~sink() = 0;

    /** Called when data is available
    */
    virtual
    result<void>
    write(
        buffers::const_buffer_pair src) = 0;
};

/** Metafunction which determines if T is a sink

    @see
        @ref sink.
*/
#ifdef BOOST_HTTP_PROTO_DOCS
template<class T>
using is_sink = __see_below__;
#else
template<class T>
using is_sink =
    std::is_convertible<
        typename std::decay<T>::type*,
        sink*>;
#endif

} // http_proto
} // boost

#endif

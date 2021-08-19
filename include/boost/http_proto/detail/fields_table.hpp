//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_FIELDS_TABLE_HPP
#define BOOST_HTTP_PROTO_DETAIL_FIELDS_TABLE_HPP

#include <boost/http_proto/field.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {
namespace detail {

// headers have a maximum size of 65536 chars
using off_t = std::uint16_t;

struct field_hint
{
    field f;    
    off_t pos;      // in buffer
    off_t len;     // key length
    // offset of value is pos+len+2
    // length of value is calculated
    //   from offset of the next entry
};

} // detail
} // http_proto
} // boost

#endif

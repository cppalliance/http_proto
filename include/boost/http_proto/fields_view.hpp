//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_FIELDS_VIEW_HPP
#define BOOST_HTTP_PROTO_FIELDS_VIEW_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/field.hpp>
#include <boost/http_proto/string_view.hpp>
#include <boost/http_proto/detail/fields_table.hpp>
#include <cstdlib>

namespace boost {
namespace http_proto {

/** A read-only, random access container of HTTP fields
*/
class fields_view
{
    char const* base_;
    std::size_t flen_;  // length of serialized fields
    std::size_t len_;   // size of storage
    std::size_t size_;  // number of fields

    fields_view(
        char const* base,
        std::size_t flen,
        std::size_t len,
        std::size_t size);

public:
    BOOST_HTTP_PROTO_DECL
    fields_view();
};

} // http_proto
} // boost

#include <boost/http_proto/impl/fields_view.hpp>

#endif

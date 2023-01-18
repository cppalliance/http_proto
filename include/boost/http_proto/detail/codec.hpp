//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DETAIL_CODEC_HPP
#define BOOST_HTTP_PROTO_DETAIL_CODEC_HPP

#include <boost/http_proto/error.hpp>
#include <cstdlib>

namespace boost {
namespace http_proto {
namespace detail {

struct BOOST_SYMBOL_VISIBLE codec
{
    struct results
    {
        error_code ec;
        std::size_t input_used = 0;
        std::size_t output_used = 0;
    };

    BOOST_HTTP_PROTO_DECL
    virtual ~codec();

    virtual
    results
    exchange(
        void* output,
        std::size_t output_size,
        void const* input,
        std::size_t input_size) = 0;
};

} // detail
} // http_proto
} // boost

#endif

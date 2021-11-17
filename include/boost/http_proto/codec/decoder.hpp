//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_DECODER_HPP
#define BOOST_HTTP_PROTO_DECODER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/error.hpp>
#include <cstdint>
#include <memory>

namespace boost {
namespace http_proto {

class decoder
{
public:
    struct buffers
    {
        char const* input;          // in/out
        std::size_t input_avail;    // in/out
        char*       output;         // in/out
        std::size_t output_avail;   // in/out

        std::size_t input_used;     // out
        std::size_t output_used;    // out
    };

    virtual ~decoder() = 0;

    virtual void exchange(
        buffers& b, error_code& ec) = 0;
};

//------------------------------------------------

class decoder_type
{
public:
    virtual
    std::unique_ptr<decoder>
    make_decoder() = 0;
};

} // http_proto
} // boost

#endif

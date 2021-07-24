//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_DECODER_HPP
#define BOOST_HTTP_PROTO_DECODER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/error.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

class decoder
{
public:
    struct buffers
    {
        void const* input;
        std::size_t input_size;
        void* output;
        std::size_t output_size;
    };

    virtual ~decoder() = 0;

//    virtual

};

class decoder_type
    : public context::service
{
public:
    
};

} // http_proto
} // boost

#endif

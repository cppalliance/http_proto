//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_CODEC_CODECS_HPP
#define BOOST_HTTP_PROTO_CODEC_CODECS_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class decoder_type;
class encoder_type;
#endif

class BOOST_SYMBOL_VISIBLE
    codecs
{
public:
    BOOST_HTTP_PROTO_DECL
    virtual ~codecs() noexcept;

    virtual
    void
    add_decoder(
        string_view name,
        decoder_type&) = 0;

    virtual
    decoder_type*
    find_decoder(
        string_view name) noexcept = 0;

    virtual
    void
    add_encoder(
        string_view name,
        encoder_type&) = 0;

    virtual
    encoder_type*
    find_encoder(
        string_view name) noexcept = 0;
};

} // http_proto
} // boost

#endif

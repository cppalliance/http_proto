//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_CODEC_IMPL_CODECS_IPP
#define BOOST_HTTP_PROTO_CODEC_IMPL_CODECS_IPP

#include <boost/http_proto/codec/codecs.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/bnf/ctype.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/container/map.hpp>
//#include <boost/unordered_map.hpp> // doesn't support heterogenous lookup yet
#include <string>

namespace boost {
namespace http_proto {

codecs::
~codecs() noexcept = default;

namespace detail {

class codecs_impl
    : public codecs
    , public context::service
{
    boost::container::map<
        std::string,
        decoder_type*,
        bnf::iless_pred
            > decoders_;

    boost::container::map<
        std::string,
        encoder_type*,
        bnf::iless_pred
            > encoders_;
public:
    using key_type = codecs;

    explicit
    codecs_impl(
        context&) noexcept
    {
    }

    void
    add_decoder(
        string_view name,
        decoder_type& dt) override
    {
        auto const result =
            decoders_.emplace(
                name.to_string(), &dt);
        if(result.second)
            return;
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    }

    decoder_type*
    find_decoder(
        string_view name) noexcept override
    {
        auto const result =
            decoders_.find(name);
        if(result !=
            decoders_.end())
            return result->second;
        return nullptr;
    }

    void
    add_encoder(
        string_view name,
        encoder_type& dt) override
    {
        auto const result =
            encoders_.emplace(
                name.to_string(), &dt);
        if(result.second)
            return;
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    }

    encoder_type*
    find_encoder(
        string_view name) noexcept override
    {
        auto const result =
            encoders_.find(name);
        if(result !=
            encoders_.end())
            return result->second;
        return nullptr;
    }
};

codecs&
install_codecs_service(
    context& ctx)
{
    return make_service<codecs_impl>(ctx);
}

} // detail

} // http_proto
} // boost

#endif

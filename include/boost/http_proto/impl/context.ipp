//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_CONTEXT_IPP
#define BOOST_HTTP_PROTO_IMPL_CONTEXT_IPP

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/detail/except.hpp>
#include <boost/http_proto/detail/string.hpp>
#include <boost/container/map.hpp>
//#include <boost/unordered_map.hpp> // doesn't support heterogenous lookup yet
#include <unordered_map>

namespace boost {
namespace http_proto {

struct context::data
{
    // Installed services
    std::unordered_map<
        std::type_index,
        std::unique_ptr<service>
            > services;

    // List of content decoders
    //boost::unordered_map<
    boost::container::map<
        std::string,
        decoder_type*,
        //detail::ihash,
        //detail::iequals_pred
        detail::iless_pred
            > content_decoders;

    // List of transfer decoders
    //boost::unordered_map<
    boost::container::map<
        std::string,
        decoder_type*,
        //detail::ihash,
        //detail::iequals_pred
        detail::iless_pred
            > transfer_decoders;
};

//------------------------------------------------

context::
service::
~service() = default;

//------------------------------------------------

context::
~context()
{
}

context::
context() noexcept
    : p_(new data)
{
}

void
context::
add_content_decoder(
    string_view name,
    decoder_type& dt)
{
    auto const result =
        p_->content_decoders.emplace(
            name.to_string(), &dt);
    if(result.second)
        return;
    detail::throw_out_of_range(
        BOOST_CURRENT_LOCATION);
}

void
context::
add_transfer_decoder(
    string_view name,
    decoder_type& dt)
{
    auto const result =
        p_->transfer_decoders.emplace(
            name.to_string(), &dt);
    if(result.second)
        return;
    detail::throw_out_of_range(
        BOOST_CURRENT_LOCATION);
}

decoder_type*
context::
find_content_decoder(
    string_view name) noexcept
{
    auto const result =
        p_->content_decoders.find(name);
    if(result !=
        p_->content_decoders.end())
        return result->second;
    return nullptr;
}

decoder_type*
context::
find_transfer_decoder(
    string_view name) noexcept
{
    auto const result =
        p_->transfer_decoders.find(name);
    if(result !=
        p_->transfer_decoders.end())
        return result->second;
    return nullptr;
}

//------------------------------------------------

auto
context::
find_service_impl(
    std::type_index id) noexcept ->
        service*
{
    auto it = p_->services.find(id);
    if(it != p_->services.end())
        return it->second.get();
    return nullptr;
}

auto
context::
make_service_impl(
    std::type_index id,
    std::unique_ptr<service> sp) ->
        service&    
{
    auto const result =
        p_->services.emplace(
            id, std::move(sp));
    if(! result.second)
        detail::throw_out_of_range(
            BOOST_CURRENT_LOCATION);
    return *result.first->second;
}

} // http_proto
} // boost

#endif

//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_CONTEXT_IPP
#define BOOST_HTTP_PROTO_IMPL_CONTEXT_IPP

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/detail/except.hpp>
//#include <boost/unordered_map.hpp> // doesn't support heterogenous lookup yet
#include <unordered_map>

namespace boost {
namespace http_proto {

namespace detail {
codecs& install_codecs_service(context& ctx);
mime_types& install_mime_types_service(context& ctx);
} // detail

struct context::data
{
    // Installed services
    std::unordered_map<
        detail::type_index,
        std::unique_ptr<service>
            > services;
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
    codecs_ = &detail::install_codecs_service(*this);
    mime_types_ = &detail::install_mime_types_service(*this);
}

//------------------------------------------------

auto
context::
find_service_impl(
    detail::type_index id) noexcept ->
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
    detail::type_index id,
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

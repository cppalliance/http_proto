//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_IMPL_MIME_TYPES_SERVICE_IPP
#define BOOST_HTTP_PROTO_SERVICE_IMPL_MIME_TYPES_SERVICE_IPP

#include <boost/http_proto/service/mime_types_service.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/url/grammar/ci_string.hpp>
#include <string>
#include <unordered_map>

namespace boost {
namespace http_proto {

namespace detail {

class mime_types_service_impl
    : public mime_types_service
    , public service
{
    struct element
    {
        std::string value;
        std::size_t prefix;
    };

    std::unordered_map<
        std::string,
        element,
        grammar::ci_hash,
        grammar::ci_equal> t_;

public:
    using key_type = mime_types_service;

    BOOST_STATIC_ASSERT(
        std::is_same<
            get_key_type<mime_types_service_impl>,
            mime_types_service>::value);

    struct param
    {
        string_view ext;
        string_view type;
        string_view subtype;
    };

    explicit
    mime_types_service_impl(
        context&) noexcept
    {
        init({
            { "htm",  "text", "html" },
            { "html", "text", "html" },
            { "php",  "text", "html" },
            { "css",  "text", "css" },
            { "txt",  "text", "plain" },
            { "js",   "application", "javascript" },
            { "json", "application", "json" },
            { "xml",  "application", "xml" },
            { "swf",  "application", "x-shockwave-flash" },
            { "flv",  "video", "x-flv" },
            { "png",  "image", "png" },
            { "jpe",  "image", "jpeg" },
            { "jpeg", "image", "jpeg" },
            { "jpg",  "image", "jpeg" },
            { "gif",  "image", "gif" },
            { "bmp",  "image", "bmp" },
            { "ico",  "image", "vnd.microsoft.icon" },
            { "tiff", "image", "tiff" },
            { "tif",  "image", "tiff" },
            { "svg",  "image", "svg+xml" },
            { "svgz", "image", "svg+xml" },
            //return "application/text" } // default
            });
    }

    void
    init(std::initializer_list<
        param> init)
    {
        for(auto v : init)
            insert(
                v.ext,
                v.type,
                v.subtype);
    }

    void
    insert(
        string_view ext,
        string_view type,
        string_view subtype)
    {
        std::string s;
        s.reserve(
            type.size() + 1 +
            subtype.size());
        s.append(
            type.data(),
            type.size());
        s.push_back('/');
        s.append(
            subtype.data(),
            subtype.size());
        t_.insert({
            std::string(ext),
            element {
                std::move(s),
                type.size() }});
    }

    mime_type
    find(
        string_view ext) const noexcept override
    {
        auto it = t_.find(ext);
        if(it == t_.end())
            return {};
        string_view v = it->second.value;
        return {
            v,
            v.substr(0, it->second.prefix),
            v.substr(it->second.prefix + 1) };
    }
};

} // detail

mime_types_service&
install_mime_types_service(
    context& ctx)
{
    return ctx.make_service<
        detail::mime_types_service_impl>();
}

} // http_proto
} // boost

#endif

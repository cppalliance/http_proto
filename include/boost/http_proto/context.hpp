//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_CONTEXT_HPP
#define BOOST_HTTP_PROTO_CONTEXT_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/string_view.hpp>
#include <memory>
#include <typeindex>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class decoder_type;
class encoder_type;
class codecs;
#endif

class context
{
    struct data;

    codecs* codecs_;

public:
    struct service
    {
        virtual ~service() = 0;
    };

    context(context const&) = delete;
    context& operator=(
        context const&) = delete;

    BOOST_HTTP_PROTO_DECL
    ~context();

    BOOST_HTTP_PROTO_DECL
    context() noexcept;

    codecs&
    get_codecs() noexcept
    {
        return *codecs_;
    }

    //--------------------------------------------

    /** Create a service.

        The service must not already exist.
    */
    template<
        class T,
        class... Args>
    friend
    T&
    make_service(
        context& ctx,
        Args&&... args);

    template<class T>
    friend
    T*
    find_service(
        context& ctx) noexcept;

    /** Return service T or throw an exception.
    */
    template<class T>
    friend
    T&
    get_service(
        context& ctx);

private:
    BOOST_HTTP_PROTO_DECL
    service*
    find_service_impl(
        std::type_index ti) noexcept;

    BOOST_HTTP_PROTO_DECL
    service&
    make_service_impl(
        std::type_index ti,
        std::unique_ptr<service> sp);

    std::unique_ptr<data> p_;
};

#if 0
template<class T>
bool
has_service(
    context& ctx) noexcept
{
    return find_service<T>(ctx) != nullptr;
}
#endif

} // http_proto
} // boost

#include <boost/http_proto/impl/context.hpp>

#endif

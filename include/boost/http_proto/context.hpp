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
#include <memory>

namespace boost {
namespace http_proto {

class context
{
    struct data;

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

    template<class T, class... Args>
    friend
    T&
    make_service(
        context& ctx,
        Args&&... args);

private:
    BOOST_HTTP_PROTO_DECL
    void
    insert_service(
        std::unique_ptr<service> sp);

    std::unique_ptr<data> p_;
};

} // http_proto
} // boost

#include <boost/http_proto/impl/context.hpp>

#endif

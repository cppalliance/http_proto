//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_SERIALIZER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class context;
class request_view;
class response_view;
#endif

struct buffers_pair
{
    string_view first;
    string_view second;
};

class serializer
{
    context& ctx_;
    char* buf_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t size_ = 0;
    string_view header;

public:
    serializer(context& ctx)
        : ctx_(ctx)
    {
    }

    bool
    is_complete() const noexcept;

    buffers_pair
    prepare(error_code& ec);

    void
    consume(std::size_t n);

    template<class Body>
    void
    staple(
        http_proto::request_view req,
        Body);

    template<class Body>
    void
    staple(
        http_proto::response_view res,
        Body);
};

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif

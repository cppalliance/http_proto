//
// Copyright (c) 2021 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERVICE_ZLIB_SERVICE_HPP
#define BOOST_HTTP_PROTO_SERVICE_ZLIB_SERVICE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/context.hpp>
#include <boost/http_proto/filter.hpp>
#include <boost/http_proto/service/service.hpp>
#include <boost/http_proto/detail/workspace.hpp>
#include <boost/http_proto/metadata.hpp>

namespace boost {
namespace http_proto {
namespace zlib {

struct decoder_config
{
    unsigned max_window_bits = 15;
};

//------------------------------------------------

struct deflate_decoder_service
    : service
{
    struct config : decoder_config
    {
        BOOST_HTTP_PROTO_ZLIB_DECL
        void
        install(context& ctx);
    };

    virtual
    config const&
    get_config() const noexcept = 0;

    virtual
    std::size_t
    space_needed() const noexcept = 0;

    virtual
    filter&
    make_filter(detail::workspace& ws) = 0;
};

//------------------------------------------------

namespace detail {
struct zlib_filter_impl;
}

class zlib_filter final : public filter
{
private:
    friend class serializer;

    detail::zlib_filter_impl* impl_;

    void init();


public:
    zlib_filter(http_proto::detail::workspace& ws);
    ~zlib_filter();

    zlib_filter(zlib_filter const&) = delete;
    zlib_filter& operator=(zlib_filter const&) = delete;

    filter::results
    on_process(
        buffers::mutable_buffer out,
        buffers::const_buffer in,
        bool more) override;

    BOOST_HTTP_PROTO_ZLIB_DECL
    void
    reset(enum content_coding_type coding);

    BOOST_HTTP_PROTO_ZLIB_DECL
    bool
    is_done() const noexcept;
};

} // zlib
} // http_proto
} // boost

#endif

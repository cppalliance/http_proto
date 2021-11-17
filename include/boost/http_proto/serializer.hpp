//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_SERIALIZER_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/basic_header.hpp>
#include <boost/http_proto/buffer.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstdint>

namespace boost {
namespace http_proto {

#ifndef BOOST_HTTP_PROTO_DOCS
class context;
#endif

class serializer
{
    context& ctx_;
    char* buf_ = nullptr;
    std::size_t cap_ = 0;
    std::size_t size_ = 0;
    string_view hs_;
    string_view bs_;

public:
    serializer(context& ctx)
        : ctx_(ctx)
    {
        (void)ctx_;
        (void)cap_;
        (void)size_;
    }

    ~serializer()
    {
        if(buf_)
            delete[] buf_;
    }

    bool
    is_complete() const noexcept
    {
        return true;
    }

    /** Clear the contents without affecting the capacity
    */
    void
    clear()
    {
    }

    const_buffer_pair
    prepare(error_code& ec)
    {
        ec = {};
        const_buffer_pair p;
        p.data[0] = hs_.data();
        p.size[0] = hs_.size();
        p.data[1] = bs_.data();
        p.size[1] = bs_.size();
        return p;
    }

    void
    consume(std::size_t n) noexcept
    {
        (void)n;
    }

    /** Staple a header and body together for serialization

        Any previous header or body is cleared.
    */
    template<class Body>
    void
    staple(
        http_proto::basic_header const& h,
        Body b)
    {
        clear();
        hs_ = h.get_const_buffer();
        bs_ = b;
    }

    // VFALCO chunked?
};

} // http_proto
} // boost

#include <boost/http_proto/impl/serializer.hpp>

#endif

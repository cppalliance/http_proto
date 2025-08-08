//
// Copyright (c) 2023 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP
#define BOOST_HTTP_PROTO_IMPL_SERIALIZER_HPP

#include <boost/http_proto/detail/except.hpp>

#include <numeric>

namespace boost {
namespace http_proto {

class serializer::cbs_gen
{
public:
    struct stats_t
    {
        std::size_t size = 0;
        std::size_t count = 0;
    };

    // Return the next non-empty buffer or an
    // empty buffer if none remain.
    virtual
    buffers::const_buffer
    next() = 0;

    // Return the total size and count of
    // remaining non-empty buffers.
    virtual
    stats_t
    stats() const = 0;

    // Return true if there are no remaining
    // non-empty buffers.
    virtual
    bool
    is_empty() const = 0;
};

template<class ConstBufferSequence>
class serializer::cbs_gen_impl
    : public cbs_gen
{
    using it_t = decltype(buffers::begin(
        std::declval<ConstBufferSequence>()));

    ConstBufferSequence cbs_;
    it_t curr_;

public:
    using const_buffer =
        buffers::const_buffer;

    explicit
    cbs_gen_impl(ConstBufferSequence cbs)
        : cbs_(std::move(cbs))
        , curr_(buffers::begin(cbs_))
    {
    }

    const_buffer
    next() override
    {
        while(curr_ != buffers::end(cbs_))
        {
            // triggers conversion operator
            const_buffer buf = *curr_++;
            if(buf.size() != 0)
                return buf;
        }
        return {};
    }

    stats_t
    stats() const override
    {
        stats_t r;
        for(auto it = curr_; it != buffers::end(cbs_); ++it)
        {
            // triggers conversion operator
            const_buffer buf = *it;
            if(buf.size() != 0)
            {
                r.size  += buf.size();
                r.count += 1;
            }
        }
        return r;
    }

    bool
    is_empty() const override
    {
        for(auto it = curr_; it != buffers::end(cbs_); ++it)
        {
            // triggers conversion operator
            const_buffer buf = *it;
            if(buf.size() != 0)
                return false;
        }
        return true;
    }
};

//---------------------------------------------------------

template<
    class ConstBufferSequence,
    class>
void
serializer::
start(
    message_view_base const& m,
    ConstBufferSequence&& cbs)
{
    static_assert(buffers::is_const_buffer_sequence<
            ConstBufferSequence>::value,
        "ConstBufferSequence type requirements not met");

    start_init(m);
    cbs_gen_ = std::addressof(
        ws_.emplace<cbs_gen_impl<typename
        std::decay<ConstBufferSequence>::type>>(
                std::forward<ConstBufferSequence>(cbs)));
    start_buffers(m);
}

template<
    class Source,
    class... Args,
    class>
Source&
serializer::
start(
    message_view_base const& m,
    Args&&... args)
{
    static_assert(
        !std::is_abstract<Source>::value, "");
    static_assert(
        std::is_constructible<Source, Args...>::value ||
        std::is_constructible<Source, detail::workspace&, Args...>::value,
        "The Source cannot be constructed with the given arguments");

    start_init(m);
    auto& src = construct_source<Source>(
        std::forward<Args>(args)...);
    source_ = std::addressof(src);
    start_source(m);
    return src;
}

} // http_proto
} // boost

#endif

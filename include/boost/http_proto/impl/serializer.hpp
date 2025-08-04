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

class serializer::const_buf_gen_base
{
public:
    // Return the next non-empty buffer,
    // or an empty buffer if none remain.
    virtual
    buffers::const_buffer
    next() = 0;

    // Size of remaining buffers
    virtual
    std::size_t
    size() const = 0;

    // Count of remaining non-empty buffers
    virtual
    std::size_t
    count() const = 0;

    // Return true when there is no buffer or
    // the remaining buffers are empty
    virtual
    bool
    is_empty() const = 0;
};

template<class ConstBufferSequence>
class serializer::const_buf_gen
    : public const_buf_gen_base
{
    using it_t = decltype(buffers::begin(
        std::declval<ConstBufferSequence>()));

    ConstBufferSequence cbs_;
    it_t current_;

public:
    using const_buffer =
        buffers::const_buffer;

    explicit
    const_buf_gen(ConstBufferSequence cbs)
        : cbs_(std::move(cbs))
        , current_(buffers::begin(cbs_))
    {
    }

    const_buffer
    next() override
    {
        while(current_ != buffers::end(cbs_))
        {
            const_buffer buf = *current_++;
            if(buf.size() != 0)
                return buf;
        }
        return {};
    }

    std::size_t
    size() const override
    {
        return std::accumulate(
            current_,
            buffers::end(cbs_),
            std::size_t{},
            [](std::size_t sum, const_buffer cb)
            {
                return sum + cb.size();
            });
    }

    std::size_t
    count() const override
    {
        return std::count_if(
            current_,
            buffers::end(cbs_),
            [](const_buffer cb)
            {
                return cb.size() != 0;
            });
    }

    bool
    is_empty() const override
    {
        return std::all_of(
            current_,
            buffers::end(cbs_),
            [](const_buffer cb)
            {
                return cb.size() == 0;
            });
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
    buf_gen_ = std::addressof(
        ws_.emplace<const_buf_gen<typename
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

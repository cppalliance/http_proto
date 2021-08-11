//
// Copyright (c) 2016-2019 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/vinniefalco/http_proto
//

#ifndef BOOST_HTTP_PROTO_CHAR_SET_HPP
#define BOOST_HTTP_PROTO_CHAR_SET_HPP

#include <boost/http_proto/detail/config.hpp>

namespace boost {
namespace http_proto {
namespace bnf {

class char_set_table
{
    char const* const tab_;

public:
    char_set_table(
        char const* tab) noexcept
        : tab_(tab)
    {
    }

    bool
    contains(
        char c) const noexcept
    {
        auto const u = static_cast<
            unsigned char>(c);
        return tab_[u] != 0;
    }

    template<class Char>
    Char*
    skip(
        Char* first,
        char const* end) const noexcept
    {
        while(first != end)
        {
            if(! contains(*first))
                break;
            ++first;
        }
        return first;
    }
};

//------------------------------------------------

template<bool (*F)(char)>
class char_set_function
{
public:
    bool
    contains(
        char c) const noexcept
    {
        auto const u = static_cast<
            unsigned char>(c);
        return F(u) != 0;
    }

    template<class Char>
    Char*
    skip(
        Char* first,
        char const* end) const noexcept
    {
        while(first != end)
        {
            if(! contains(*first))
                break;
            ++first;
        }
        return first;
    }
};

} // bnf
} // http_proto
} // boost

#endif

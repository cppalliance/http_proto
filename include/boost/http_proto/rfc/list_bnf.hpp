//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_RFC_LIST_BNF_HPP
#define BOOST_HTTP_PROTO_RFC_LIST_BNF_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/string_view.hpp>
#include <cstddef>

#include <boost/url/error.hpp>

namespace boost {
namespace http_proto {

template<class T>
class list_bnf_base
{
protected:
    string_view s_;
    std::size_t n_ = 0;

    list_bnf_base(
        string_view s,
        std::size_t n) noexcept
        : s_(s)
        , n_(n)
    {
    }

    static
    bool
    begin(
        char const*& it,
        char const* end,
        error_code& ec,
        T& t);

    static
    bool
    increment(
        char const*& it,
        char const* end,
        error_code& ec,
        T& t);

    static
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        std::size_t N,
        std::size_t M,
        std::size_t& n);

public:
    class iterator;

    list_bnf_base() = default;

    std::size_t
    size() const noexcept
    {
        return n_;
    }

    iterator
    begin() const noexcept;

    iterator
    end() const noexcept;
};

/** BNF for a comma-delimited list of elements

    This rule defines a list containing
    at least n and at most m of Element,
    each separated by a single comma and
    optional whitespace.

    @par BNF
    @code
    list        =  <n>#<m>element => element <n-1>*<m-1>( OWS "," OWS element )

    #element    => [ ( ("," OWS element) / element ) *( OWS "," [ OWS element ] ) ]

    1#element   => *( "," OWS ) element *( OWS "," [ OWS element ] )
    @endcode

    @see
        https://datatracker.ietf.org/doc/html/rfc7230#section-7
        https://www.rfc-editor.org/errata/eid5257

    @tparam Element The element type to use in the list
    @tparam N The minimum number of list items, which may be zero
    @tparam M The maximum number of list items.
*/
template<
    class T,
    std::size_t N = 0,
    std::size_t M = std::size_t(-1)>
class list_bnf : public list_bnf_base<T>
{
    list_bnf(
        string_view s,
        std::size_t n) noexcept
        : list_bnf_base<T>(s, n)
    {
    }

    static
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        std::size_t N_,
        std::size_t M_,
        std::size_t& n)
    {
        return list_bnf_base<T>::parse(
            it, end, ec, N_, M_, n);
    }

public:
    list_bnf() = default;

    explicit
    list_bnf(
        string_view s);

    template<
        class T_,
        std::size_t N_,
        std::size_t M_>
    friend
    bool
    parse(
        char const*& it,
        char const* end,
        error_code& ec,
        list_bnf<T_, N_, M_>& t);
};

} // http_proto
} // boost

#include <boost/http_proto/rfc/impl/list_bnf.hpp>

#endif

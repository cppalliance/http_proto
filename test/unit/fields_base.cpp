//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields_base.hpp>

#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields.hpp>
#include <boost/http_proto/request.hpp>
#include <boost/http_proto/response.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/static_assert.hpp>

#include "test_helpers.hpp"
#include "test_suite.hpp"

#include <stdexcept>
#include <vector>

namespace boost {
namespace http_proto {

// check for overflow
//BOOST_STATIC_ASSERT(
    //fields_base::max_capacity_in_bytes() >= max_offset);

struct fields_base_test
{
    static
    void
    check(
        core::string_view s0,
        void(*fn)(fields_base&),
        core::string_view s1)
    {
        // fields
        {
            fields f(s0);
            fn(f);
            BOOST_TEST_EQ(f.buffer(), s1);
            test_fields(f, s1);
        }

        // request
        {
            auto const m = std::string() +
                "GET / HTTP/1.1\r\n" +
                std::string(s0);
            auto const m1 = std::string() +
                "GET / HTTP/1.1\r\n" +
                std::string(s1);
            request req(m);
            fn(req);
            BOOST_TEST_EQ(req.buffer(), m1);
            test_fields(req, s1);
        }

        // response
        {
            auto const m = std::string() +
                "HTTP/1.1 200 OK\r\n" +
                std::string(s0);
            auto const m1 = std::string() +
                "HTTP/1.1 200 OK\r\n" +
                std::string(s1);
            response res(m);
            fn(res);
            BOOST_TEST_EQ(res.buffer(), m1);
            test_fields(res, s1);
        }
    }

    static
    void
    check_error(
        core::string_view s0,
        void(*fn)(fields_base&))
    {
        // fields
        {
            fields f(s0);
            fn(f);
            BOOST_TEST_EQ(f.buffer(), s0);
        }
    }

    void
    testCapacity()
    {
        // clear()
        {
            // default fields
            {
                fields f;
                f.clear();
                BOOST_TEST_EQ(
                    f.buffer(),
                    "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
            }

            // default request
            {
                request req;
                req.clear();
                BOOST_TEST_EQ(
                    req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), 0);
            }

            // default response
            {
                response res;
                res.clear();
                BOOST_TEST_EQ(
                    res.buffer(),
                    "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), 0);
            }

            {
                fields f("\r\n");
                auto const n =
                    f.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);
                BOOST_TEST_EQ(
                    f.buffer(),
                    "\r\n");
            }

            {
                BOOST_TEST_THROWS(
                    fields("HTTP/1.1"),
                    std::invalid_argument);
            }

            {
                fields f(
                    "digest: ffce\r\n"
                    "type: 3\r\n"
                    "\r\n");
                auto const n =
                    f.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);
                f.clear();
                BOOST_TEST_EQ(
                    f.buffer(),
                    "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), n);
            }

            {
                request req(
                    "POST / HTTP/1.1\r\n"
                    "\r\n");
                auto const n =
                    req.capacity_in_bytes();
                BOOST_TEST_EQ(
                    req.buffer(),
                    "POST / HTTP/1.1\r\n\r\n");
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), n);
            }

            {
                BOOST_TEST_THROWS(
                    request(
                        "POST / HTTP/1.1"
                        "\r\n"),
                    std::invalid_argument
                );
            }

            {
                request req(
                    "POST / HTTP/1.1\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n");
                auto const n =
                    req.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);
                req.clear();
                BOOST_TEST_EQ(
                    req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), n);
            }

            {
                response res(
                    "HTTP/1.1 404 Not Found\r\n"
                    "\r\n");
                auto const n =
                    res.capacity_in_bytes();
                BOOST_TEST_EQ(
                    res.buffer(),
                    "HTTP/1.1 404 Not Found\r\n\r\n");
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), n);
            }

            {
                BOOST_TEST_THROWS(
                    response(
                        "HTTP/1.1 404 Not Found"
                        "\r\n"),
                    std::invalid_argument);
            }

            {
                response res(
                    "HTTP/1.1 404 Not Found\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n");
                auto const n =
                    res.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);
                res.clear();
                BOOST_TEST_EQ(
                    res.buffer(),
                    "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), n);
            }
        }

        // reserve_bytes(std::size_t)
        {
            // default request
            {
                request req;
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), 0);

                // first allocation
                req.reserve_bytes(100);
                auto const n =
                    req.capacity_in_bytes();
                BOOST_TEST_GE(n, 100);
                BOOST_TEST_EQ(req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");

                // no reallocation
                req.reserve_bytes(n);
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), n);

                // no reallocation
                req.reserve_bytes(n / 2);
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), n);

                // reallocation
                req.reserve_bytes(n + 1);
                BOOST_TEST_GT(
                    req.capacity_in_bytes(), n);
                BOOST_TEST_EQ(req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
            }

            // response
            {
                core::string_view const cs =
                    "HTTP/1.1 200 OK\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                response res(cs);
                auto n = res.capacity_in_bytes();
                BOOST_TEST_GT(n, 0);

                // no reallocation
                res.reserve_bytes(n);
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), n);
                BOOST_TEST_EQ(res.buffer(), cs);
            }
        }

        // shrink_to_fit()
        {
            // default fields
            {
                fields f;
                f.shrink_to_fit();
                BOOST_TEST_EQ(
                    f.buffer(),
                    "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
            }

            // default request
            {
                request req;
                req.shrink_to_fit();
                BOOST_TEST_EQ(
                    req.buffer(),
                    "GET / HTTP/1.1\r\n\r\n");
                BOOST_TEST_EQ(
                    req.capacity_in_bytes(), 0);
            }

            // default response
            {
                response res;
                res.shrink_to_fit();
                BOOST_TEST_EQ(
                    res.buffer(),
                    "HTTP/1.1 200 OK\r\n\r\n");
                BOOST_TEST_EQ(
                    res.capacity_in_bytes(), 0);
            }

            {
                core::string_view const cs =
                    "digest: ffce\r\n"
                    "type: 3\r\n"
                    "\r\n";
                fields f(cs);
                f.reserve_bytes(
                    f.capacity_in_bytes() * 2);
                auto const n =
                    f.capacity_in_bytes();
                f.shrink_to_fit();
                BOOST_TEST_LT(
                    f.capacity_in_bytes(), n);
                BOOST_TEST_EQ(f.buffer(), cs);
            }

            {
                core::string_view const cs =
                    "POST / HTTP/1.1\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                request req(cs);
                req.reserve_bytes(
                    req.capacity_in_bytes() * 2);
                auto const n =
                    req.capacity_in_bytes();
                req.shrink_to_fit();
                BOOST_TEST_LT(
                    req.capacity_in_bytes(), n);
                BOOST_TEST_EQ(req.buffer(), cs);
            }

            {
                core::string_view const cs =
                    "HTTP/1.1 404 Not Found\r\n"
                    "User-Agent: test\r\n"
                    "Server: test\r\n"
                    "Content-Length: 0\r\n"
                    "\r\n";
                response res(cs);
                res.reserve_bytes(
                    res.capacity_in_bytes() * 2);
                auto const n =
                    res.capacity_in_bytes();
                res.shrink_to_fit();
                BOOST_TEST_LT(
                    res.capacity_in_bytes(), n);
                BOOST_TEST_EQ(res.buffer(), cs);
            }
        }
    }

    void
    testAppend()
    {
        // append(field, string_view)

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
            "Cookie: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.append(field::server, "y");
            },
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n");

        // append(string_view, string_view)

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
            "Cookie: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.append("Server", "y");
            },
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n");

        // append(string_view, string_view) rfc compliance
        //
        //    field-line     = field-name ":" OWS field-value OWS
        //
        //    field-name     = token
        //    token          = 1*tchar
        //    tchar          = "!" / "#" / "$" / "%" / "&" / "'" / "*"
        //                   / "+" / "-" / "." / "^" / "_" / "`" / "|" / "~"
        //                   / DIGIT / ALPHA
        //                   ; any VCHAR, except delimiters
        //
        //    field-value    = *field-content
        //    field-content  = field-vchar
        //                      [ 1*( SP / HTAB / field-vchar ) field-vchar ]
        //    field-vchar    = VCHAR / obs-text
        //    obs-text       = %x80-FF

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append("!#$%&'*+-.^_`|~1A", "     A \t \x80\xffZ     ");
            },
            "!#$%&'*+-.^_`|~1A: A \t \x80\xffZ\r\n"
            "\r\n");

        check(
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.append(
                    "!#$%&'*+-.^_`|~1A", "\r\n\t  \r\n   AB\r\n C  \r\n\t");

                BOOST_TEST(rv.has_value());
            },
            "!#$%&'*+-.^_`|~1A: AB   C\r\n"
            "\r\n");

        check(
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.append("A", "A\r\n\tB\r\n C");
                BOOST_TEST(rv.has_value());
            },
            "A: A  \tB   C\r\n"
            "\r\n");

        check(
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.append("A", "custom: rawr\r\n\tB\r\n C");
                BOOST_TEST(rv.has_value());
            },
            "A: custom: rawr  \tB   C\r\n"
            "\r\n");

        check(
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.append("A", "   \t    \r\n  \r\n\t  \t       \t   \r\n ");
                BOOST_TEST(rv.has_value());
            },
            "A:\r\n"
            "\r\n");

        check_error(
            "\r\n",
            [](fields_base& f)
            {
                system::result<void> rv;

                // ends with invalid obs-fold
                rv = f.append("X", "AB\r\n C  \r\n");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_value);

                // contains invalid obs-fold between {AB, C}
                rv = f.append("X", "\r\n\x09  \r\n   AB: rawr\r\nC");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_smuggle);

                rv = f.append("X", "         \r");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_value);

                // empty field name
                rv = f.append("", "ABC");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_name);

                std::vector<char const *> strs = {
                    "\r\nABC", "\rABC", "A\rBC",
                    "ABC\r",   "\nABC", "A\nBC",
                    "ABC\n",   "\r",    "\n"};

                for (auto const str : strs)
                {
                    rv = f.append("X", str);
                    BOOST_TEST(rv.has_error());
                }
            });

        // empty value should not
        // have a prepended space
        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append("X", "");
            },
            "X:\r\n"
            "\r\n");

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.append("X", "");
                f.append("Y", "");
            },
            "X:\r\n"
            "Y:\r\n"
            "\r\n");
    }

    void
    testInsert()
    {
        // insert(iterator, field, string_view)

        check(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.insert(f.find("T"), field::server, "x");
                BOOST_TEST(rv.has_value());
                BOOST_TEST(rv.value() == f.find(field::server));
            },
            "Server: x\r\n"
            "T: 1\r\n"
            "\r\n");

        check(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto pos = f.find("T");
                auto rv = f.insert(f.find("U"), field::server, "x");

                BOOST_TEST(rv.has_value());
                BOOST_TEST(rv.value() == f.find(field::server));
                BOOST_TEST(pos == f.find("T"));
            },
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "\r\n");

        check_error(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.insert(f.find("U"), field::server, "a\r\nb");
                BOOST_TEST(rv.has_error());
            });

        // insert(iterator, string_view, string_view)

        check(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.insert(f.find("T"), "Server", "x");
                BOOST_TEST(rv.has_value());
                BOOST_TEST(rv.value() == f.find("Server"));
            },
            "Server: x\r\n"
            "T: 1\r\n"
            "\r\n");

        check(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto pos = f.find("T");
                auto rv = f.insert(f.find("U"), "Server", "x");

                BOOST_TEST(rv.has_value());
                BOOST_TEST(rv.value() == f.find("Server"));
                BOOST_TEST(pos == f.find("T"));
            },
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "\r\n");

        check_error(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                system::result<typename fields_base::iterator> rv;

                rv = f.insert(f.find("U"), "Ser ver", "x");
                BOOST_TEST(rv.has_error());

                rv = f.insert(f.find("U"), " Server", "x");
                BOOST_TEST(rv.has_error());

                rv = f.insert(f.find("U"), "Server ", "x");
                BOOST_TEST(rv.has_error());
            });

        // self-intersect

        check(
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.insert(
                    f.begin(),
                    f.find(field::user_agent)->value,
                    f.find(field::connection)->value);
            },
            "boost: close\r\n"
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n");

        check(
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                // reserve
                f.reserve_bytes(f.capacity_in_bytes() * 2);
                f.insert(
                    f.begin(),
                    f.find(field::user_agent)->value,
                    f.find(field::connection)->value);
            },
            "boost: close\r\n"
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n");
    }

    void
    testErase()
    {
        // erase(iterator)

        check(
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.erase(f.find("Server"));
            },
            "\r\n");

        check(
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.erase(f.find("Server"));
            },
            "Cookie: x\r\n"
            "\r\n");

        //
        // erase(field)
        //

        // no match
        check(
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase(field::connection), 0U);
            },
            "Server: y\r\n"
            "\r\n");

        // one match
        check(
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase(field::server), 1);
            },
            "\r\n");

        // different capitalization
        check(
            "server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase(field::server), 1);
            },
            "\r\n");

        // three matches, different capitalization
        check(
            "Server: x\r\n"
            "server: y\r\n"
            "SERVER: z\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase(field::server), 3);
            },
            "\r\n");

        // three matches, three unmatched
        check(
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "Server: y\r\n"
            "Server: z\r\n"
            "V: 3\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase(field::server), 3);
            },
            "T: 1\r\n"
            "U: 2\r\n"
            "V: 3\r\n"
            "\r\n");

        //
        // erase(string_view)
        //

        // one match, different case
        check(
            "Server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("server"), 1);
            },
            "\r\n");

        // one match, different case
        check(
            "server: y\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("Server"), 1);
            },
            "\r\n");

        // three matches
        check(
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "Server: y\r\n"
            "Server: z\r\n"
            "V: 3\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("Server"), 3);
            },
            "T: 1\r\n"
            "U: 2\r\n"
            "V: 3\r\n"
            "\r\n");

        // three matches, unknown id
        check(
            "T: 1\r\n"
            "Server: Boost\r\n"
            "T: 2\r\n"
            "Connection: close\r\n"
            "T: 3\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("T"), 3);
            },
            "Server: Boost\r\n"
            "Connection: close\r\n"
            "\r\n");

        // no matches
        check(
            "Connection: keep-alive\r\n"
            "Server: Boost\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("Accept"), 0U);
            },
            "Connection: keep-alive\r\n"
            "Server: Boost\r\n"
            "\r\n");

        // unknown field name
        check(
            "Connection: keep-alive\r\n"
            "X: 1\r\n"
            "Server: Boost\r\n"
            "X: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                BOOST_TEST_EQ(
                    f.erase("X"), 2);
            },
            "Connection: keep-alive\r\n"
            "Server: Boost\r\n"
            "\r\n");
    }

    void
    testSet()
    {
        // set(iterator, string_view)

        check(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.set(f.find("T"), "2");
                BOOST_TEST(rv.has_value());
            },
            "T: 2\r\n"
            "\r\n");

        check(
            "T: abc\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("T"), "2");
            },
            "T: 2\r\n"
            "\r\n");

        check(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("T"), "abcdefghijklmnopqrstuvwxyz");
            },
            "T: abcdefghijklmnopqrstuvwxyz\r\n"
            "\r\n");

        check(
            "T: 1\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(f.find("T"), "abcdefghijk\r\n lmnopqrst\r\n\tuvwxyz\r\n ");
            },
            "T: abcdefghijk   lmnopqrst  \tuvwxyz\r\n"
            "\r\n");

        check_error(
            "T: abc\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.set(f.find("T"), "\r\n");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_value);

                rv = f.set(f.find("T"), "abcdefghijk\r\nlmnopqrstuvwxyz");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_smuggle);
            });

        // set(field, string_view)

        check(
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.set(field::server, "x");
                BOOST_TEST(rv.has_value());
            },
            "Server: x\r\n"
            "\r\n");

        check(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
            "T: 1\r\n"
            "Server: x\r\n"
            "T: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "y");
            },
            "T: 1\r\n"
            "T: 2\r\n"
            "Server: y\r\n"
            "\r\n");

        check(
            "Server: x1\r\n"
            "Server: x2\r\n"
            "Server: x3\r\n"
            "T: t\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set(field::server, "y");
            },
            "T: t\r\n"
            "Server: y\r\n"
            "\r\n");

        check(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.set(field::server, "\r\n x\r\n yz \r\n \r\n\t");
                BOOST_TEST(rv.has_value());
            },
            "Server: x   yz\r\n"
            "\r\n");

        check_error(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.set(field::server, "\r\n x\r\nyz \r\n \r\n\t");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_smuggle);

                rv = f.set(field::server, "yz\r\n\x01\x02\x03");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_smuggle);
            });

        // set(string_view, string_view)

        check(
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "x");
            },
            "Server: x\r\n"
            "\r\n");

        check(
            "Server: x\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        check(
            "T: 1\r\n"
            "Server: xx\r\n"
            "T: 2\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "y");
            },
            "T: 1\r\n"
            "T: 2\r\n"
            "Server: y\r\n"
            "\r\n");

        check(
            "Server: x1\r\n"
            "Server: x2\r\n"
            "Server: x3\r\n"
            "T: t\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "y");
            },
            "T: t\r\n"
            "Server: y\r\n"
            "\r\n");

        check(
            "Connection: keep-alive\r\n"
            "Server: Boost\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Connection", "close");
            },
            "Server: Boost\r\n"
            "Connection: close\r\n"
            "\r\n");

        check(
            "Connection: keep-alive\r\n"
            "Server: Boost\r\n"
            "\r\n",
            [](fields_base& f)
            {
                f.set("Server", "\r\n hello    \r\n     world!!!!");
            },
            "Connection: keep-alive\r\n"
            "Server: hello           world!!!!\r\n"
            "\r\n");

        check_error(
            "\r\n",
            [](fields_base& f)
            {
                auto rv = f.set(" invalid string", "valid string");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_name);

                rv = f.set("invalid\r\n string", "valid string");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_name);

                rv = f.set("valid", "\r\ninvalid string");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_smuggle);

                rv = f.set("valid", "invalid\x01\x02\r\nstring");
                BOOST_TEST(rv.has_error());
                BOOST_TEST(rv.error() == error::bad_field_value);
            });
    }

    void
    testExpect()
    {
        // parse request
        {
            auto const check =
            []( metadata::expect_t md,
                core::string_view s)
            {
                request const req(s);
                BOOST_TEST_EQ(
                    req.metadata().expect.ec,
                    md.ec);
                BOOST_TEST_EQ(
                    req.metadata().expect.count,
                    md.count);
                BOOST_TEST_EQ(
                    req.metadata().expect.is_100_continue,
                    md.is_100_continue);
            };

            check(
                { {}, 0, false},
                "POST / HTTP/1.1\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { error::bad_expect, 1, false },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                { error::bad_expect, 2, false },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { error::bad_expect, 2, false },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");
        }

        // parse response
        {
            auto const check =
            [](core::string_view s)
            {
                response const res(s);
                BOOST_TEST_EQ(
                    res.metadata().expect.ec,
                    system::error_code());
                BOOST_TEST_EQ(
                    res.metadata().expect.count,
                    res.count(field::expect));
                BOOST_TEST_EQ(
                    res.metadata().expect.is_100_continue,
                    false);
            };

            check(
                "HTTP/1.1 200 OK\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");
        }

        // erase in response
        {
            response res(
                "HTTP/1.1 200 OK\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");
            auto it = res.find(field::expect);
            res.erase(it);
            BOOST_TEST(
                ! res.metadata().expect.ec.failed());
            BOOST_TEST_EQ(
                res.metadata().expect.count, 0);
            BOOST_TEST_EQ(
                res.metadata().expect.is_100_continue,
                false);
        }

        // erase, set in request
        {
            auto const check =
            []( metadata::expect_t md,
                void(*fn)(request&),
                core::string_view s)
            {
                request req(s);
                fn(req);
                BOOST_TEST_EQ(
                    req.metadata().expect.ec,
                    md.ec);
                BOOST_TEST_EQ(
                    req.metadata().expect.count,
                    md.count);
                BOOST_TEST_EQ(
                    req.metadata().expect.is_100_continue,
                    md.is_100_continue);
            };

            // erase

            check(
                { {}, 0, false },
                [](request& req)
                {
                    // erase one
                    auto it = req.find(
                        field::expect);
                    req.erase(it);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { {}, 0, false },
                [](request& req)
                {
                    // erase all
                    BOOST_TEST_EQ(req.erase(
                        field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { {}, 0, false },
                [](request& req)
                {
                    BOOST_TEST_EQ(req.erase(
                        field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    auto it = req.find(field::expect);
                    BOOST_TEST_NE(it, req.end());
                    req.erase(it);
                    BOOST_TEST_EQ(
                        req.count(field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { error::bad_expect, 1, false },
                [](request& req)
                {
                    auto it = req.find(field::expect);
                    BOOST_TEST_NE(it, req.end());
                    req.erase(it);
                    BOOST_TEST_EQ(
                        req.count(field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    auto it = req.find_last(
                        req.end(), field::expect);
                    BOOST_TEST_NE(it, req.end());
                    req.erase(it);
                    BOOST_TEST_EQ(
                        req.count(field::expect), 1);
                },
                "POST / HTTP/1.1\r\n"
                "Content-Length: 1234\r\n"
                "Expect: 100-continue\r\n"
                "Connection: close\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");

            // set

            check(
                { error::bad_expect, 1, false },
                [](request& req)
                {
                    req.set(
                        field::expect,
                        "100-continueish");
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continue\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    req.set(
                        field::expect,
                        "100-continue");
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 100-continueish\r\n"
                "\r\n");

            check(
                { {}, 1, true },
                [](request& req)
                {
                    req.set(
                        field::expect,
                        "100-continue");
                },
                "POST / HTTP/1.1\r\n"
                "Expect: 500-server-error\r\n"
                "Expect: 404-not-found\r\n"
                "\r\n");
        }
    }

    void
    run()
    {
        testCapacity();
        testAppend();
        testInsert();
        testErase();
        testSet();
        testExpect();

        test_suite::log <<
            "sizeof(detail::header) == " <<
            sizeof(detail::header) << "\n";
    }
};

TEST_SUITE(
    fields_base_test,
    "boost.http_proto.fields_base");

} // http_proto
} // boost

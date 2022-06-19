//
// Copyright (c) 2021 Vinnie Falco (vinnie dot falco at gmail dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/fields.hpp>

#include <boost/http_proto/field.hpp>
#include <boost/http_proto/fields_view.hpp>
#include <initializer_list>

#include "test_helpers.hpp"

#include <string>

namespace boost {
namespace http_proto {

struct fields_test
{
    void
    modify(
        string_view before,
        void (*pf)(fields&),
        string_view after)
    {
        fields f0 = make_fields(before);
        fields f1 = make_fields(after);
        fields f(f0);
        (*pf)(f);
        BOOST_TEST_EQ(f.string(),
            f1.string());
    }

    //--------------------------------------------

    void
    testSpecial()
    {
        string_view const cs1 =
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n";

        string_view const cs2 =
            "x: 1\r\n"
            "y: 2\r\n"
            "z: 3\r\n"
            "\r\n";

        // ~fields()
        // fields()
        {
            fields f;
            BOOST_TEST_EQ(
                f.string(), "\r\n");
            BOOST_TEST_EQ(
                f.string().data(),
                fields().string().data());
        }

        // fields(fields&&)
        {
            {
                fields f1 = make_fields(cs1);
                fields f2(std::move(f1));
                test_fields(f1, "\r\n");
                test_fields(f2, cs1);
            }
            {
                fields f1;
                fields f2(std::move(f1));
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.string().data(),
                    f2.string().data());
            }
        }

        // fields(fields const&)
        {
            {
                fields f1 = make_fields(cs1);
                fields f2(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.string().data(),
                    f2.string().data());
            }
            {
                fields f1;
                fields f2(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.string().data(),
                    f2.string().data());
            }
        }

        // fields(fields_view const&)
        {
            {
                fields f1 = make_fields(cs1);
                fields f2(static_cast<
                    fields_view>(f1));

                BOOST_TEST_EQ(
                    f2.string(), cs1);
                BOOST_TEST_NE(
                    f2.string().data(),
                    cs1.data());
                test_fields(f2, cs1);
            }

            // default buffer
            {
                fields_view fv;
                fields f(fv);
                BOOST_TEST_EQ(
                    f.string(), "\r\n");
                BOOST_TEST_EQ(
                    f.string().data(),
                    fv.string().data());
            }
        }

        // operator=(fields&&)
        {
            {
                fields f1 = make_fields(cs1);
                fields f2;
                f2 = std::move(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, cs1);
            }
            {
                fields f1 = make_fields(cs1);
                fields f2 = make_fields(cs2);
                f2 = std::move(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, cs1);
            }
            {
                fields f1;
                fields f2 = make_fields(cs1);
                f2 = std::move(f1);
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.string().data(),
                    f2.string().data());
            }
        }

        // operator=(fields const&)
        {
            {
                fields f1 = make_fields(cs1);
                fields f2;
                f2 = f1;
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.string().data(),
                    f2.string().data());
            }
            {
                fields f1 = make_fields(cs1);
                fields f2 = make_fields(
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "z: 3\r\n"
                    "\r\n");
                f2 = f1;
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.string().data(),
                    f2.string().data());
            }
            {
                fields f1;
                fields f2 = make_fields(cs1);
                f2 = f1;
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.string().data(),
                    f2.string().data());
            }
        }

        // operator=(fields_view)
        {
            {
                fields f1 =
                    make_fields(cs1);
                fields f2;
                f2 = static_cast<
                    fields_view>(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.string().data(),
                    f2.string().data());
            }
            {
                fields f1 = make_fields(cs1);
                fields f2 = make_fields(
                    "x: 1\r\n"
                    "y: 2\r\n"
                    "z: 3\r\n"
                    "\r\n");
                f2 = static_cast<
                    fields_view>(f1);
                test_fields(f1, cs1);
                test_fields(f2, cs1);
                BOOST_TEST_NE(
                    f1.string().data(),
                    f2.string().data());
            }
            {
                fields_view f1;
                fields f2 = make_fields(cs1);
                f2 = f1;
                test_fields(f1, "\r\n");
                test_fields(f2, "\r\n");
                BOOST_TEST_EQ(
                    f1.string().data(),
                    f2.string().data());
            }

            // existing capacity
            {
                fields f1 = make_fields(cs1);
                fields f2;
                f2.reserve(
                    2 * cs1.size() + 128);
                f2 = static_cast<
                    fields_view>(f1);
                test_fields(f2, cs1);
                BOOST_TEST(
                    f1.string().data() !=
                    f2.string().data());
            }
        }
    }

    void
    testObservers()
    {
        string_view const cs =
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n";

        // fields_view_base::string()
        {
            fields f1 = make_fields(cs);
            BOOST_TEST_EQ(f1.string(), cs);
        }

        // fields_base::capacity_in_bytes()
        {
            {
                fields f;
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
            }
            {
                fields f;
                f.reserve(100);
                BOOST_TEST_GE(
                    f.capacity_in_bytes(), 100);
            }
            {
                fields f;
                f.reserve(100);
                f.shrink_to_fit();
                BOOST_TEST_GT(
                    f.capacity_in_bytes(), 0);
            }
        }
    }

    void
    testModifiers()
    {
        string_view const cs =
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n";

        // clear()
        {
            {
                fields f;
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
                f.clear();
                test_fields(f, "\r\n");
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
            }
            {
                fields f = make_fields(cs);
                BOOST_TEST_GT(
                    f.capacity_in_bytes(), 0);
                f.clear();
                test_fields(f, "\r\n");
                BOOST_TEST_GT(
                    f.capacity_in_bytes(), 0);
            }
        }

        // reserve(std::size_t)
        {
            {
                fields f;
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), 0);
                f.reserve(100);
                BOOST_TEST_GE(
                    f.capacity_in_bytes(), 100);
                test_fields(f, "\r\n");
            }
            {
                fields f = make_fields(cs);
                auto const cap =
                    f.capacity_in_bytes();
                BOOST_TEST_GT(cap, 0);
                f.reserve(cap / 2);
                BOOST_TEST_EQ(
                    f.capacity_in_bytes(), cap);
                f.reserve(2 * cap);
                BOOST_TEST_GE(
                    f.capacity_in_bytes(), 2 * cap);
                test_fields(f, cs);
            }
        }

        // shrink_to_fit()
        {
            fields f;
            f.reserve(200);
            f.shrink_to_fit();
            auto const n =
                f.capacity_in_bytes();
            BOOST_TEST_LT(n, 200);
            f.shrink_to_fit();
            BOOST_TEST_EQ(
                f.capacity_in_bytes(), n);
        }

        // swap()
        {
        }
    }

    void
    testAppend()
    {
        // append(field, string_view)

        modify(
            "\r\n",
            [](fields& f)
            {
                f.append(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "Cookie: x\r\n"
            "\r\n",
            [](fields& f)
            {
                f.append(field::server, "y");
            },
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n");

        // append(string_view, string_view)

        modify(
            "\r\n",
            [](fields& f)
            {
                f.append("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "Cookie: x\r\n"
            "\r\n",
            [](fields& f)
            {
                f.append("Server", "y");
            },
            "Cookie: x\r\n"
            "Server: y\r\n"
            "\r\n");
    }

    void
    testErase()
    {
        // erase(iterator)
        {
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields& f)
                {
                    f.erase(f.find("Server"));
                },
                "\r\n");

            modify(
                "Cookie: x\r\n"
                "Server: y\r\n"
                "\r\n",
                [](fields& f)
                {
                    f.erase(f.find("Server"));
                },
                "Cookie: x\r\n"
                "\r\n");
        }

        // erase(field)
        {
            // no match
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::connection), 0);
                },
                "Server: y\r\n"
                "\r\n");

            // one match
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 1);
                },
                "\r\n");

            // different capitalization
            modify(
                "server: y\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 1);
                },
                "\r\n");

            // three matches, different capitalization
            modify(
                "Server: x\r\n"
                "server: y\r\n"
                "SERVER: z\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 3);
                },
                "\r\n");

            // three matches, three unmatched
            modify(
                "T: 1\r\n"
                "Server: x\r\n"
                "U: 2\r\n"
                "Server: y\r\n"
                "Server: z\r\n"
                "V: 3\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase(field::server), 3);
                },
                "T: 1\r\n"
                "U: 2\r\n"
                "V: 3\r\n"
                "\r\n");
        }

        // erase(string_view)
        {
            // one match, different case
            modify(
                "Server: y\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("server"), 1);
                },
                "\r\n");

            // one match, different case
            modify(
                "server: y\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("Server"), 1);
                },
                "\r\n");

            // three matches
            modify(
                "T: 1\r\n"
                "Server: x\r\n"
                "U: 2\r\n"
                "Server: y\r\n"
                "Server: z\r\n"
                "V: 3\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("Server"), 3);
                },
                "T: 1\r\n"
                "U: 2\r\n"
                "V: 3\r\n"
                "\r\n");

            // three matches, unknown id
            modify(
                "T: 1\r\n"
                "Server: Boost\r\n"
                "T: 2\r\n"
                "Connection: close\r\n"
                "T: 3\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("T"), 3);
                },
                "Server: Boost\r\n"
                "Connection: close\r\n"
                "\r\n");

            // no matches
            modify(
                "Connection: keep-alive\r\n"
                "Server: Boost\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("Accept"), 0);
                },
                "Connection: keep-alive\r\n"
                "Server: Boost\r\n"
                "\r\n");

            // unknown field name
            modify(
                "Connection: keep-alive\r\n"
                "X: 1\r\n"
                "Server: Boost\r\n"
                "X: 2\r\n"
                "\r\n",
                [](fields& f)
                {
                    BOOST_TEST_EQ(
                        f.erase("X"), 2);
                },
                "Connection: keep-alive\r\n"
                "Server: Boost\r\n"
                "\r\n");
        }
    }

    void
    testInsert()
    {
        // insert(iterator, field, string_view)

        modify(
            "T: 1\r\n"
            "\r\n",
            [](fields& f)
            {
                f.insert(f.find("T"),
                    field::server, "x");
            },
            "Server: x\r\n"
            "T: 1\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields& f)
            {
                f.insert(f.find("U"),
                    field::server, "x");
            },
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "\r\n");

        // insert(iterator, string_view, string_view)

        modify(
            "T: 1\r\n"
            "\r\n",
            [](fields& f)
            {
                f.insert(f.find("T"),
                    "Server", "x");
            },
            "Server: x\r\n"
            "T: 1\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "U: 2\r\n"
            "\r\n",
            [](fields& f)
            {
                f.insert(f.find("U"),
                    "Server", "x");
            },
            "T: 1\r\n"
            "Server: x\r\n"
            "U: 2\r\n"
            "\r\n");

        // self-intersect
        modify(
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n",
            [](fields& f)
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

        // self-intersect
        modify(
            "Connection: close\r\n"
            "Set-Cookie: 0\r\n"
            "User-Agent: boost\r\n"
            "Set-Cookie: 1\r\n"
            "\r\n",
            [](fields& f)
            {
                // reserve
                f.reserve(f.capacity_in_bytes() * 2);
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
    testSet()
    {
        // set(iterator, string_view)

        modify(
            "T: 1\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set(f.find("T"), "2");
            },
            "T: 2\r\n"
            "\r\n");

        // set(field, string_view)

        modify(
            "\r\n",
            [](fields& f)
            {
                f.set(field::server, "x");
            },
            "Server: x\r\n"
            "\r\n");

        modify(
            "Server: x\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set(field::server, "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "Server: x\r\n"
            "T: 2\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set(field::server, "y");
            },
            "T: 1\r\n"
            "T: 2\r\n"
            "Server: y\r\n"
            "\r\n");

        modify(
            "Server: x1\r\n"
            "Server: x2\r\n"
            "Server: x3\r\n"
            "T: t\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set(field::server, "y");
            },
            "T: t\r\n"
            "Server: y\r\n"
            "\r\n");

        // set(string_view, string_view)

        modify(
            "\r\n",
            [](fields& f)
            {
                f.set("Server", "x");
            },
            "Server: x\r\n"
            "\r\n");

        modify(
            "Server: x\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set("Server", "y");
            },
            "Server: y\r\n"
            "\r\n");

        modify(
            "T: 1\r\n"
            "Server: xx\r\n"
            "T: 2\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set("Server", "y");
            },
            "T: 1\r\n"
            "T: 2\r\n"
            "Server: y\r\n"
            "\r\n");

        modify(
            "Server: x1\r\n"
            "Server: x2\r\n"
            "Server: x3\r\n"
            "T: t\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set("Server", "y");
            },
            "T: t\r\n"
            "Server: y\r\n"
            "\r\n");

        modify(
            "Connection: keep-alive\r\n"
            "Server: Boost\r\n"
            "\r\n",
            [](fields& f)
            {
                f.set(f.find("Connection"), "close");
            },
            "Connection: close\r\n"
            "Server: Boost\r\n"
            "\r\n");
    }

    void
    testContentLength()
    {
#if 0
        fields f = make_fields(make_header(
            { "Content-Length: 0" }));
        BOOST_TEST(f.content_length().has_value);
        BOOST_TEST_EQ(f.content_length().value, 0);
#endif
    }

    void
    run()
    {
        testSpecial();
        testObservers();
        testModifiers();
        testAppend();
        testErase();
        testInsert();
        testSet();
        testContentLength();
    }
};

TEST_SUITE(
    fields_test,
    "boost.http_proto.fields");

} // http_proto
} // boost

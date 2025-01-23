//
// Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

// Test that header file is self-contained.
#include <boost/http_proto/parser.hpp>

#include <boost/http_proto/context.hpp>
#include <boost/http_proto/request_parser.hpp>
#include <boost/http_proto/response_parser.hpp>
#include <boost/http_proto/service/zlib_service.hpp>
#include <boost/buffers/buffer_copy.hpp>
#include <boost/buffers/buffer_size.hpp>
#include <boost/buffers/flat_buffer.hpp>
#include <boost/buffers/make_buffer.hpp>
#include <boost/buffers/string_buffer.hpp>
#include <boost/core/ignore_unused.hpp>
#include <vector>

#include "test_helpers.hpp"

#include <vector>

//------------------------------------------------
/*

Parser operation

    For caller provided objects the parser can copy
    its internal contents into the caller's object
    by calling a function. Or it can request a buffer
    from the caller's object into which the body
    contents are placed. In the simple case this
    means that enclosing socket reads can read
    directly into caller-provided buffers.

General Case
    parser pr;
    error_code ec;

    pr.start();                 // must call first
    auto mb = pr.prepare();     // returns the input buffer
    ...
    pr.commit( n );             // commit input buffer bytes
    pr.parse( ec );             // parse data
                                // (inspect ec for special codes)

Parser-provided string_view body (default)

    // nothing to do
    assert( pr.is_complete() );
    string_view s = pr.body();

Parser-provided buffer-at-time body

    parser::const_buffers_type part = pr.pull_some()

Caller-provided body buffers (dynamic buffer?)

    pr.set_body( bb );           //

Caller-provided sink

    pr.set_body( sk );

--------------------------------------------------

Caller wants to parse a message and have the body
stored in a std::string upon completion.
    1. Re-using an existing string, or
    2. Creating a new string

This all speaks to DynamicBuffer as the correct API
    * But is our DynamicBuffer owning or reference-like?
    * What triggers the final resize() on the std::string?
        - destructor
        - other member function

    parser pr;
    std::string s;
    ...
    pr.set_body( buffers::dynamic_for( s ) ); // reference semantics

    parser pr;
    buffers::flat_buffer fb;
    ...
    pr.set_body( fb ); // flat_buffer&
*/
//------------------------------------------------

namespace boost {
namespace http_proto {

struct parser_test
{
    template<class T>
    class opt
    {
        union aligned_storage {
            aligned_storage(){}
            ~aligned_storage(){}

            T v_;
        };

        aligned_storage s_;
        bool b_ = false;

    public:
        opt() = default;

        ~opt()
        {
            if(b_)
                get().~T();
        }

        template<class... Args>
        T&
        emplace(Args&&... args)
        {
            if(b_)
            {
                get().~T();
                b_ = false;
            }
            ::new(&get()) T(
                std::forward<Args>(args)...);
            b_ = true;
            return get();
        }

        T& get() noexcept
        {
            return s_.v_;
        }

        T const& get() const noexcept
        {
            return s_.v_;
        }

        T& operator*() noexcept
        {
            return get();
        }

        T const& operator*() const noexcept
        {
            return get();
        }
    };

    struct test_sink : sink
    {
        std::string s;
        std::size_t max_size_;

        explicit
        test_sink(
            std::size_t max_size =
                std::size_t(-1)) noexcept
            : max_size_(max_size)
        {
        }

        results
        on_write(
            buffers::const_buffer b,
            bool more) noexcept override
        {
            (void)more;
            results rv;
            auto const space =
                max_size_ - s.size();
            auto n = b.size();
            if( n > space)
                n = space;
            s.append(static_cast<
                char const*>(b.data()),
                    b.size());
            rv.bytes = n;
            if(n < b.size())
                rv.ec = error::buffer_overflow;
            return rv;
        }
    };

    //--------------------------------------------

    using pieces = std::vector<
        core::string_view>;

    context ctx_;
    core::string_view sh_;
    core::string_view sb_;
    request_parser req_pr_;
    response_parser res_pr_;
    parser* pr_ = nullptr;
    opt<context> ctx_opt_;
    opt<request_parser> req_pr_opt_;
    opt<response_parser> res_pr_opt_;
    pieces in_;

    parser_test()
        : ctx_()
        , req_pr_(
            [&]() -> context&
            {
                request_parser::config cfg;
                cfg.body_limit = 5;
                cfg.min_buffer = 3;
                install_parser_service(ctx_, cfg);
                return ctx_;
            }())
        , res_pr_(ctx_)
    {
        in_.reserve(5);
        req_pr_.reset();
        res_pr_.reset();
    }

    //-------------------------------------------

    static
    void
    read_some(
        parser& pr,
        pieces& in,
        system::error_code& ec)
    {
        pr.parse(ec);
        if(ec != condition::need_more_input)
            return;
        if(! in.empty())
        {
            core::string_view& s = in[0];
            auto const n =
                buffers::buffer_copy(
                pr.prepare(),
                buffers::make_buffer(
                    s.data(), s.size()));
            pr.commit(n);
            s.remove_prefix(n);
            if(s.empty())
                in.erase(in.begin());
        }
        else
        {
            pr.commit_eof();
        }
        pr.parse(ec);
    }

    static
    void
    read_header(
        parser& pr,
        pieces& in,
        system::error_code& ec)
    {
        do
        {
            read_some(pr, in, ec);
            if(ec == condition::need_more_input)
                continue;
            if(ec.failed())
                return;
        }
        while(! pr.got_header());
    }

    static
    void
    read(
        parser& pr,
        pieces& in,
        system::error_code& ec)
    {
        do
        {
            read_some(pr, in, ec);
            if(ec == condition::need_more_input)
                continue;
            if(ec.failed())
                return;
        }
        while(! pr.is_complete());
    }

    static
    void
    prep(
        parser& pr,
        core::string_view s)
    {
        pr.reset();
        pr.start();
        auto const n =
            buffers::buffer_copy(
            pr.prepare(),
            buffers::make_buffer(
                s.data(), s.size()));
        pr.commit(n);
        BOOST_TEST_EQ(n, s.size());
        system::error_code ec;
        pr.parse(ec);
        if( ec == condition::need_more_input)
            ec = {};
        BOOST_TEST(! ec.failed());
    }

    //--------------------------------------------

    request_parser&
    do_req(
        std::initializer_list<
            core::string_view> init)
    {
        in_ = init;
        BOOST_ASSERT(init.size() > 0);
        BOOST_ASSERT(! init.begin()->empty());
        request_parser::config cfg;
        ctx_opt_.emplace();
        install_parser_service(*ctx_opt_, cfg);
        req_pr_opt_.emplace(*ctx_opt_);
        return *req_pr_opt_;
    }

    //--------------------------------------------

    void
    testSpecial()
    {
        // ~parser

        {
            request_parser pr(ctx_);
        }

        {
            response_parser pr(ctx_);
        }

    }

    void
    testConfig()
    {
    #ifdef BOOST_HTTP_PROTO_HAS_ZLIB
        context ctx;
        zlib::install_service(ctx);

        request_parser::config_base cfg1;
        cfg1.apply_deflate_decoder = true;
        install_parser_service(ctx, cfg1);
    #endif
    }

    void
    testReset()
    {
    }

    void
    testStart()
    {
        context ctx;
        request_parser::config_base cfg;
        install_parser_service(ctx, cfg);

        {
            // missing reset
            request_parser pr(ctx);
            BOOST_TEST_THROWS(
                pr.start(),
                std::logic_error);
        }

        {
            // missing reset
            request_parser pr(ctx);
            pr.reset();
            pr.start();
            pr.commit_eof();
            BOOST_TEST_THROWS(
                pr.start(),
                std::logic_error);
        }

        {
            // start called twice
            request_parser pr(ctx);
            pr.reset();
            pr.start();
            BOOST_TEST_THROWS(
                pr.start(),
                std::logic_error);
        }

        {
            // incomplete message
            request_parser pr(ctx);
            prep(pr, "GET /");
            BOOST_TEST_THROWS(
                pr.start(),
                std::logic_error);
        }

        {
            // incomplete message
            request_parser pr(ctx);
            prep(pr,
                "POST / HTTP/1.1\r\n"
                "Content-Length: 5\r\n"
                "\r\n"
                "123");

            BOOST_TEST_THROWS(
                pr.start(),
                std::logic_error);
        }
    }

    void
    testPrepare()
    {
        {
            // missing reset
            request_parser pr(ctx_);
            BOOST_TEST_THROWS(
                pr.prepare(),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            pr.reset();
            BOOST_TEST_THROWS(
                pr.prepare(),
                std::logic_error);
        }

        auto const check_header = [](
            request_parser::config const& cfg,
            std::size_t n)
        {
            context ctx;
            install_parser_service(ctx, cfg);

            request_parser pr(ctx);
            pr.reset();
            pr.start();
            parser::mutable_buffers_type dest;
            BOOST_TEST_NO_THROW(
                dest = pr.prepare());
            BOOST_TEST_EQ(
                buffers::buffer_size(dest), n);
        };

        //
        // header
        //

        {
            // normal
            request_parser::config cfg;
            cfg.headers.max_size = 40;
            cfg.min_buffer = 3;
            check_header(cfg,
                cfg.headers.max_size +
                cfg.min_buffer);
        }

        {
            // max_prepare
            request_parser::config cfg;
            cfg.max_prepare = 2;
            check_header(cfg, cfg.max_prepare);
        }

        //
        // in_place body
        //

        auto const check_in_place = [](
            request_parser::config const& cfg,
            std::size_t n,
            core::string_view s)
        {
            context ctx;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            request_parser pr(ctx);
            pr.reset();
            pr.start();
            pieces in({ s });
            read_header(pr, in, ec);
            if( ! BOOST_TEST(! ec.failed()) ||
                ! BOOST_TEST(pr.got_header()) ||
                ! BOOST_TEST(! pr.is_complete()))
                return;
            pr.parse(ec);
            BOOST_TEST_EQ(ec, error::need_data);
            parser::mutable_buffers_type dest;
            BOOST_TEST_NO_THROW(
                dest = pr.prepare());
            BOOST_TEST_EQ(
                buffers::buffer_size(dest), n);
        };

        {
            // normal
            request_parser::config cfg;
            check_in_place(cfg, 12291,
                "POST / HTTP/1.1\r\n"
                "Content-Length: 3\r\n"
                "\r\n");
        }

        {
            // max_prepare
            request_parser::config cfg;
            cfg.max_prepare = 10;
            check_in_place(cfg, 10,
                "POST / HTTP/1.1\r\n"
                "Content-Length: 32\r\n"
                "\r\n");
        }

        //
        // dynamic body
        //

        constexpr std::size_t
            dynamic_max_size = 7;

        auto const check_dynamic = [&](
            request_parser::config const& cfg,
            std::size_t n,
            core::string_view s,
            system::error_code ex = error::need_data)
        {
            context ctx;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            request_parser req_pr(ctx);
            response_parser res_pr(ctx);
            BOOST_ASSERT(! s.empty());
            parser* pr;
            if(s.front() != 'H')
            {
                req_pr.reset();
                req_pr.start();
                pr = &req_pr;
            }
            else
            {
                res_pr.reset();
                res_pr.start();
                pr = &res_pr;
            }
            pieces in({ s });
            read_header(*pr, in, ec);
            if( ! BOOST_TEST(! ec.failed()) ||
                ! BOOST_TEST(pr->got_header()))
                return;

            // we use a heap-allocated std::string object because internally,
            // parsers store copies of the provided buffers (which have view
            // semantics)
            // placing the object on the heap ensures reliable
            // use-after-free errors are produced under sanitized builds
            std::unique_ptr<std::string> ptmp(new std::string());
            auto &tmp = *ptmp;
            buffers::string_buffer sb(
                &tmp, dynamic_max_size);
            pr->set_body(std::move(sb));
            pr->parse(ec);
            BOOST_TEST_EQ(ec, ex);
            if(! ec.failed())
            {
                parser::mutable_buffers_type dest;
                BOOST_TEST_NO_THROW(
                    dest = pr->prepare());
                BOOST_TEST_EQ(
                    buffers::buffer_size(dest), n);
            }

            // the parser must be manually reset() to clear its inner workspace
            // otherwise, ~workspace itself will wind up clearing the registered
            // buffers which winds up touching the long-dead `ptmp` used by the
            // `buffers::string_buffer`
            pr->reset();
        };

        {
            // Content-Length
            request_parser::config cfg;
            check_dynamic(cfg, 3,
                "POST / HTTP/1.1\r\n"
                "Content-Length: 3\r\n"
                "\r\n");
        }

        {
            // Content-Length, no overread
            request_parser::config cfg;
            cfg.max_prepare = 10;
            check_dynamic(cfg, 5,
                "POST / HTTP/1.1\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
        }

        {
            // Content-Length, max_prepare
            request_parser::config cfg;
            cfg.max_prepare = 3;
            check_dynamic(cfg, 3,
                "POST / HTTP/1.1\r\n"
                "Content-Length: 5\r\n"
                "\r\n");
        }

        {
            // to_eof
            request_parser::config cfg;
            check_dynamic(cfg,
                dynamic_max_size,
                "HTTP/1.1 200 OK\r\n"
                "\r\n");
        }

        {
            // to_eof, max_prepare
            request_parser::config cfg;
            cfg.max_prepare = 3;
            BOOST_TEST(cfg.max_prepare <
                dynamic_max_size);
            check_dynamic(cfg,
                cfg.max_prepare,
                "HTTP/1.1 200 OK\r\n"
                "\r\n");
        }

        {
            // to_eof, max_prepare
            request_parser::config cfg;
            BOOST_TEST(
                dynamic_max_size == 7);
            check_dynamic(cfg,
                1,
                "HTTP/1.1 200 OK\r\n"
                "\r\n"
                "1234567");
        }

        {
            // fill capacity first
            context ctx;
            request_parser::config cfg;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            response_parser pr(ctx);
            pr.reset();
            pr.start();
            pieces in({
                "HTTP/1.1 200 OK\r\n"
                "\r\n" });
            read_header(pr, in, ec);
            std::unique_ptr<std::string> ps(new std::string());
            auto &s = *ps;
            // requires small string optimization
            BOOST_TEST_GT(s.capacity(), 0);
            BOOST_TEST_LT(s.capacity(), 5000);
            pr.set_body(buffers::string_buffer(&s));
            pr.parse(ec);
            BOOST_TEST(ec == error::need_data);
            auto dest = pr.prepare();
            BOOST_TEST_LE(
                buffers::buffer_size(dest),
                s.capacity());
            pr.reset();
        }

        {
            // set_body, no room
            request_parser::config cfg;
            BOOST_TEST(
                dynamic_max_size == 7);
            check_dynamic(cfg,
                0,
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 10\r\n"
                "\r\n"
                "1234567890",
                error::buffer_overflow);
        }

        // prepare when complete
        {
            request_parser::config cfg;
            pieces in({
                "GET / HTTP/1.1\r\n\r\n"});
            context ctx;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            request_parser pr(ctx);
            pr.reset();
            pr.start();
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());
            parser::mutable_buffers_type dest;
            BOOST_TEST_THROWS(
                dest = pr.prepare(),
                std::logic_error);
        }
    }

    void
    testCommit()
    {
        {
            // missing reset
            request_parser pr(ctx_);
            BOOST_TEST_THROWS(
                pr.commit(0),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            pr.reset();
            BOOST_TEST_THROWS(
                pr.commit(0),
                std::logic_error);
        }

        //
        // header
        //

        {
            // commit too large
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            auto dest = pr.prepare();
            BOOST_TEST_THROWS(
                pr.commit(
                    buffers::buffer_size(dest) + 1),
                std::invalid_argument);
        }

        {
            // commit after EOF
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            pr.commit_eof();
            BOOST_TEST_THROWS(
                pr.commit(0),
                std::logic_error);
        }

        {
            // 0-byte commit
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            BOOST_TEST_NO_THROW(
                pr.commit(0));
        }

        {
            // 1-byte commit
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            auto dest = pr.prepare();
            BOOST_TEST_GE(
                buffers::buffer_size(dest), 1);
            BOOST_TEST_NO_THROW(
                pr.commit(1));
        }

        //
        // body
        //

        {
            // commit too large
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            system::error_code ec;
            pieces in = {
                "POST / HTTP/1.1\r\n"
                "Content-Length: 5\r\n"
                "\r\n" };
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            pr.parse(ec);
            auto dest = pr.prepare();
            BOOST_TEST_THROWS(
                pr.commit(
                    buffers::buffer_size(dest) + 1),
                std::invalid_argument);
        }

#if 0
        // VFALCO missing chunked implementation
        {
            // buffered payload
            context ctx;
            request_parser::config cfg;
            install_parser_service(ctx, cfg);
            request_parser pr(ctx_);

            check_body(pr,
                "POST / HTTP/1.1\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n");
        }
#endif

        //
        // in-place
        //

        auto const check_in_place = [](
            request_parser::config const& cfg,
            system::error_code ex,
            bool is_complete,
            pieces&& in)
        {
            context ctx;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            response_parser pr(ctx);
            pr.reset();
            pr.start();
            read_header(pr, in, ec);
            if(ex.failed() && ec == ex)
            {
                BOOST_TEST_EQ(ec, ex);
                return;
            }
            read_some(pr, in, ec);
            BOOST_TEST_EQ(ec, ex);
            BOOST_TEST_EQ(
                pr.is_complete(), is_complete);
        };

        // Content-Length, incomplete
        {
            request_parser::config cfg;
            check_in_place(cfg,
                error::need_data, false, {
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 3\r\n"
                "\r\n",
                "1" });
        }

        // Content-Length, complete
        {
            request_parser::config cfg;
            check_in_place(cfg,
                {}, true, {
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 3\r\n"
                "\r\n",
                "123" });
        }

        // to_eof
        {
            request_parser::config cfg;
            cfg.min_buffer = 3;
            check_in_place(cfg,
                error::need_data, false, {
                "HTTP/1.1 200 OK\r\n"
                "\r\n",
                "1" });
        }

        //
        // dynamic
        //

        // VFALCO Could do with some targeted
        // tests, right now this is covered
        // incidentally from other tests

        //
        // set_body
        //

        {
            // no-op
            context ctx;
            response_parser::config cfg;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            response_parser pr(ctx);
            pr.reset();
            pr.start();
            pieces in = {
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 3\r\n"
                "\r\n"
                "123" };
            read(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());
            std::unique_ptr<std::string> ps(new std::string());
            auto &s = *ps;
            pr.set_body(
                buffers::string_buffer(&s));
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            pr.reset();
        }

        {
            // commit too large
            context ctx;
            response_parser::config cfg;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            response_parser pr(ctx);
            pr.reset();
            pr.start();
            pieces in = {
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 3\r\n"
                "\r\n"
                "123" };
            read(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());

            std::unique_ptr<std::string> ps(new std::string());
            auto &s = *ps;
            pr.set_body(
                buffers::string_buffer(&s));
            BOOST_TEST_THROWS(
                pr.commit(1),
                std::logic_error);
            pr.reset();
        }

        //
        // complete
        //

        {
            // 0-byte commit
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            system::error_code ec;
            pieces in = {
                "GET / HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n" };
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(!pr.is_complete());
            pr.parse(ec);
            BOOST_TEST_EQ(
                ec, condition::need_more_input);
            auto dest = pr.prepare();
            ignore_unused(dest);
            BOOST_TEST_NO_THROW(pr.commit(0));
        }

        {
            // commit too large
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            system::error_code ec;
            pieces in = {
                "GET / HTTP/1.1\r\n"
                "Content-Length: 1\r\n"
                "\r\n" };
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(!pr.is_complete());
            pr.parse(ec);
            BOOST_TEST_EQ(
                ec, condition::need_more_input);
            auto dest = pr.prepare();
            ignore_unused(dest);
            BOOST_TEST_THROWS(
                pr.commit(
                    buffers::buffer_size(dest) + 1),
                std::invalid_argument);
        }
    }

    void
    testCommitEof()
    {
        {
            // missing reset
            request_parser pr(ctx_);
            BOOST_TEST_THROWS(
                pr.commit_eof(),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            pr.reset();
            BOOST_TEST_THROWS(
                pr.commit_eof(),
                std::logic_error);
        }

        {
            // empty stream
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            BOOST_TEST_NO_THROW(
                pr.commit_eof());
            // VFALCO What else?
        }

        {
            // body
            response_parser pr(ctx_);
            pr.reset();
            pr.start();
            system::error_code ec;
            pieces in = {
                "HTTP/1.1 200 OK\r\n"
                "\r\n" };
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(! pr.is_complete());
            pr.parse(ec);
            BOOST_TEST_EQ(
                ec, condition::need_more_input);
            BOOST_TEST_NO_THROW(
                pr.commit_eof());
        }

        {
            // set_body
            response_parser pr(ctx_);
            pr.reset();
            pr.start();
            system::error_code ec;
            pieces in = {
                "HTTP/1.1 200 OK\r\n"
                "\r\n" };
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(! pr.is_complete());
            std::unique_ptr<std::string> ps(new std::string());
            auto &s = *ps;
            pr.set_body(
                buffers::string_buffer(&s));
            pr.parse(ec);
            BOOST_TEST(ec == error::need_data);
            BOOST_TEST_NO_THROW(
                pr.commit_eof());
            pr.reset();
        }

        {
            // complete
            request_parser pr(ctx_);
            pr.reset();
            pr.start();
            system::error_code ec;
            pieces in = {
                "GET / HTTP/1.1\r\n"
                "\r\n" };
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());
            BOOST_TEST_THROWS(
                pr.commit_eof(),
                std::logic_error);
        }
    }

    void
    testParse()
    {
        {
            // missing reset
            request_parser pr(ctx_);
            system::error_code ec;
            BOOST_TEST_THROWS(
                pr.parse(ec),
                std::logic_error);
        }

        {
            // missing start
            request_parser pr(ctx_);
            pr.reset();
            system::error_code ec;
            BOOST_TEST_THROWS(
                pr.parse(ec),
                std::logic_error);
        }

        //
        // in_place
        //

        auto const check_in_place = [](
            response_parser::config const& cfg,
            bool some,
            system::error_code ex,
            bool is_complete,
            pieces&& in)
        {
            context ctx;
            install_parser_service(ctx, cfg);
            system::error_code ec;
            response_parser pr(ctx);
            pr.reset();
            pr.start();
            read_header(pr, in, ec);
            if(ex.failed() && ec == ex)
            {
                BOOST_TEST_EQ(ec, ex);
                return;
            }
            if(some)
                read_some(pr, in, ec);
            else
                read(pr, in, ec);
            BOOST_TEST_EQ(ec, ex);
            BOOST_TEST_EQ(
                pr.is_complete(), is_complete);
        };

        {
            // Content-Length, incomplete
            response_parser::config cfg;
            core::string_view const h =
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 5\r\n"
                "\r\n";
            cfg.headers.max_size = h.size();
            check_in_place(cfg, false,
                error::incomplete, false, {
                h,
                "123" });
        }

        {
            // Content-Length, in_place limit
            response_parser::config cfg;
            core::string_view const h =
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 10\r\n"
                "\r\n";
            cfg.headers.max_size = h.size();
            cfg.min_buffer = 3;
            check_in_place(cfg, true,
                error::in_place_overflow, false, {
                h, "1234567890" });
        }

        {
            // Content-Length, need data
            response_parser::config cfg;
            check_in_place(cfg, true,
                error::need_data, false, {
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 10\r\n"
                "\r\n",
                "123" });
        }

        {
            // Content-Length, complete
            response_parser::config cfg;
            check_in_place(cfg, false,
                {}, true, {
                "HTTP/1.1 200 OK\r\n"
                "Content-Length: 3\r\n"
                "\r\n",
                "123" });
        }

        {
            // to_eof, body too large
            response_parser::config cfg;
            cfg.body_limit = 3;
            cfg.min_buffer = 3;
            check_in_place(cfg, true,
                error::body_too_large, false, {
                "HTTP/1.1 200 OK\r\n"
                "\r\n",
                "12345" });
        }

        {
            // chunked, need data
            response_parser::config cfg;
            check_in_place(cfg, true,
                error::need_data, false, {
                "HTTP/1.1 200 OK\r\n"
                "Transfer-Encoding: chunked\r\n"
                "\r\n"});
        }

        {
            // to_eof, complete
            response_parser::config cfg;
            check_in_place(cfg, false,
                {}, true, {
                "HTTP/1.1 200 OK\r\n"
                "\r\n",
                "12345" });
        }

        //
        // dynamic
        //

    }

    //--------------------------------------------

    void
    start()
    {
        pr_->reset();
        if(pr_ == &req_pr_)
            req_pr_.start();
        else
            res_pr_.start();
    }

    void
    check_in_place(
        pieces& in,
        system::error_code ex = {})
    {
        system::error_code ec;

        start();
        read_header(*pr_, in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            pr_->reset();
            return;
        }
        if(pr_->is_complete())
        {
            BOOST_TEST_EQ(pr_->body(), sb_);
            // this should be a no-op
            read(*pr_, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST_EQ(pr_->body(), sb_);
            return;
        }
        BOOST_TEST(pr_->body().empty());
        read(*pr_, in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            pr_->reset();
            return;
        }
        if(! BOOST_TEST(pr_->is_complete()))
            return;
        BOOST_TEST_EQ(pr_->body(), sb_);
        // this should be a no-op
        read(*pr_, in, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(pr_->body(), sb_);
    }

    void
    check_dynamic(
        pieces& in,
        system::error_code ex = {})
    {
        char buf[5];
        system::error_code ec;

        start();
        read_header(*pr_, in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            pr_->reset();
            return;
        }
        buffers::flat_buffer fb(buf, sizeof(buf));
        pr_->set_body(std::ref(fb));
        BOOST_TEST(pr_->body().empty());
        if(! pr_->is_complete())
        {
            read(*pr_, in, ec);
            if(ec.failed())
            {
                BOOST_TEST_EQ(ec, ex);
                pr_->reset();
                return;
            }
            if(! BOOST_TEST(pr_->is_complete()))
                return;
        }
        BOOST_TEST_EQ(
            test_to_string(fb.data()), sb_);
        BOOST_TEST(pr_->body().empty());
        // this should be a no-op
        read(*pr_, in, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(
            test_to_string(fb.data()), sb_);
    }

    void
    check_sink(
        pieces& in,
        system::error_code ex = {})
    {
        system::error_code ec;

        start();
        read_header(*pr_, in, ec);
        if(ec.failed())
        {
            BOOST_TEST_EQ(ec, ex);
            pr_->reset();
            return;
        }
        auto& ts = pr_->set_body<test_sink>();
        BOOST_TEST(pr_->body().empty());
        if(! pr_->is_complete())
        {
            read(*pr_, in, ec);
            if(ec.failed())
            {
                BOOST_TEST_EQ(ec, ex);
                pr_->reset();
                return;
            }
            if(! BOOST_TEST(pr_->is_complete()))
                return;
        }
        BOOST_TEST_EQ(ts.s, sb_);
        BOOST_TEST(pr_->body().empty());
        // this should be a no-op
        read(*pr_, in, ec);
        BOOST_TEST(! ec.failed());
        BOOST_TEST_EQ(ts.s, sb_);
    }


    void
    check_req_1(
        pieces const& in0,
        system::error_code ex)
    {
        // in_place
        {
            auto in = in0;
            check_in_place(in, ex);
        }

        // dynamic
        {
            auto in = in0;
            check_dynamic(in, ex);
        }

        // sink
        {
            auto in = in0;
            check_sink(in, ex);
        }
    }

    void
    check_res_1(
        pieces const& in0,
        system::error_code ex)
    {
        // in_place
        {
            auto in = in0;
            check_in_place(in, ex);
        }

        // dynamic
        {
            auto in = in0;
            check_dynamic(in, ex);
        }

        // sink
        {
            auto in = in0;
            check_sink(in, ex);
        }
    }

    // void Fn( pieces& )
    template<class Fn>
    void
    grind(
        core::string_view sh,
        core::string_view sb,
        Fn const& fn)
    {
        std::string const s = [&]
        {
            std::string s;
            s.reserve(sh.size() + sb.size());
            s.append(sh.data(), sh.size());
            s.append(sb.data(), sb.size());
            return s;
        }();

        sh_ = sh;
        sb_ = sb;
        pieces in;
        in.reserve(3);
        core::string_view const sv(
            s.data(), s.size());

        // one piece
        in = pieces{ sv };
        fn(in);

        for(std::size_t i = 0;
            i <= s.size(); ++i)
        {
            // two pieces
            in = {
                sv.substr(0, i),
                sv.substr(i) };
            fn(in);

#if 0
            // VFALCO is this helpful?
            for(std::size_t j = i;
                j <= s.size(); ++j)
            {
                // three pieces
                in = {
                    sv.substr(0, i),
                    sv.substr(i, j - i),
                    sv.substr(j) };
                fn(in);
            }
#endif
        }
    }

    void
    check_req(
        core::string_view sh,
        core::string_view sb,
        system::error_code ex = {})
    {
        pr_ = &req_pr_;
        grind(sh, sb, [&](
            pieces const& in0)
            {
                check_req_1(in0, ex);
            });
    }

    void
    check_res(
        core::string_view sh,
        core::string_view sb,
        system::error_code ex = {})
    {
        pr_ = &res_pr_;
        grind(sh, sb, [&](
            pieces const& in0)
            {
                check_res_1(in0, ex);
            });
    }

    void
    should_pass(
        core::string_view sh,
        core::string_view sb)
    {
        BOOST_ASSERT(! sh.empty());
        if(sh[0] != 'H')
            check_req(sh, sb);
        else
            check_res(sh, sb);
    }

    void
    should_fail(
        system::error_code ex,
        core::string_view sh,
        core::string_view sb)
    {
        if(sh.empty())
        {
            BOOST_ASSERT(sb.empty());
            check_req(sh, sb, ex);
            check_res(sh, sb, ex);
            return;
        }
        if(sh[0] != 'H')
            check_req(sh, sb, ex);
        else
            check_res(sh, sb, ex);
    }

    void
    testParseHeader()
    {
        // clean stream close
        should_fail(
            error::end_of_stream,
            "",
            "");

        // partial message
        should_fail(
            error::incomplete,
            "GET",
            "");

        // VFALCO TODO map grammar error codes
        // invalid syntax
        //should_fail(
            //error::bad_method,
            //" B", "");

        // payload error
        should_fail(
            error::bad_payload,
            "GET / HTTP/1.1\r\n"
            "Content-Length: 1\r\n"
            "Content-Length: 2\r\n"
            "\r\n",
            "12");

        should_pass(
            "GET / HTTP/1.1\r\n"
            "\r\n",
            "");
    }

    void
    testParseRequest()
    {
        should_pass(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "\r\n",
            "");

        should_pass(
            "GET / HTTP/1.1\r\n"
            "User-Agent: test\r\n"
            "Content-Length: 3\r\n"
            "\r\n",
            "123");
    }

    void
    testParseResponse()
    {
        should_pass(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "");

        should_pass(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "Content-Length: 3\r\n"
            "\r\n",
            "123");

        should_pass(
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "123");

        should_fail(
            error::body_too_large,
            "HTTP/1.1 200 OK\r\n"
            "Server: test\r\n"
            "\r\n",
            "Hello");
    }

    void
    testChunkedInPlace()
    {
        context ctx;
        response_parser::config cfg;
        install_parser_service(ctx, cfg);

        response_parser pr(ctx);

        {
            // initial hello world parse for chunked
            // encoding

            pr.reset();
            pr.start();

            pieces in = {
                "HTTP/1.1 200 OK\r\n"
                "transfer-encoding: chunked\r\n"
                "\r\n"
                "d\r\nhello, world!\r\n"
                "0\r\n\r\n" };

            system::error_code ec;
            read(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());

            auto str = pr.body();
            BOOST_TEST_EQ(str, "hello, world!");
        }

        {
            // split chunk-header and chunk-body w/
            // closing chunk
            // must be valid
            // also test that the code accepts any arbitrary
            // amount of leading zeroes

            pr.reset();
            pr.start();

            pieces in1 = {
                "HTTP/1.1 200 OK\r\n"
                "transfer-encoding: chunked\r\n"
                "\r\n"
                "00000000000000000000000000000000000d\r\n" };

            pieces in2 = {
                "hello, world!\r\n"
                "0\r\n\r\n" };

            system::error_code ec;
            read_header(pr, in1, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            BOOST_TEST(! pr.is_complete());

            read(pr, in2, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());
            BOOST_TEST_EQ(pr.body(), "hello, world!");
        }

        {
            // chunk-size much larger, contains closing
            // chunk in its body
            // this must _not_ complete the parser, and we
            // should error out with needs-more

            pr.reset();
            pr.start();

            // the chunk body is an OCTET stream, thus it
            // can contain anything
            pieces in1 = {
                "HTTP/1.1 200 OK\r\n"
                "transfer-encoding: chunked\r\n"
                "\r\n"
                "1234\r\nhello, world!\r\n"
                "0\r\n\r\n" };

            system::error_code ec;
            read_header(pr, in1, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            BOOST_TEST(! pr.is_complete());

            pr.parse(ec);
            BOOST_TEST_EQ(
                ec, condition::need_more_input);
            BOOST_TEST(! pr.is_complete());
        }

        {
            // chunk-size is too small
            // must end with an unrecoverable parsing error

            pr.reset();
            pr.start();

            // the chunk body is an OCTET stream, thus it
            // can contain anything
            pieces in1 = {
                "HTTP/1.1 200 OK\r\n"
                "transfer-encoding: chunked\r\n"
                "\r\n"
                "03\r\nhello, world!\r\n"
                "0\r\n\r\n" };

            system::error_code ec;
            read_header(pr, in1, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            BOOST_TEST(! pr.is_complete());

            pr.parse(ec);
            BOOST_TEST_EQ(
                ec, condition::invalid_payload);
            BOOST_TEST(!pr.is_complete());
        }

        {
            // valid chunk, but split up oddly

            pr.reset();
            pr.start();
            pieces in = {
                "HTTP/1.1 200 OK\r\ntransfer-encoding: chunked\r\n\r\n",
                "d\r\nhello, ",
                "world!",
                "\r\n",
                "29\r\n",
                " and this is a much longer ",
                "string of text",
                "\r\n",
                "0\r\n\r\n"
            };

            system::error_code ec;
            read(pr, in, ec);
            BOOST_TEST(pr.is_complete());
            BOOST_TEST_EQ(
                pr.body(),
                "hello, world! and this is a much longer string of text");
        }

        {
            // make sure we're somewhat robust against
            // malformed input

            core::string_view headers =
                "HTTP/1.1 200 OK\r\n"
                "transfer-encoding: chunked\r\n"
                "\r\n";

            pieces samples = {
                "xxxasdfasdfasd", // invalid chunk header
                "1\rzxcv",        // invalid crlf on chunk-size close
                "1\r\nabcd",      // invalid chunk-body close
                "1\r\na\rqwer",   // invalid chunk-body close
                // similar but now for the last-chunk
                "0\rzxcv",
                "0\r\n\rqwer",
                "1\r\na\r\n0\r\n\rabcd",
                "fffffffffffffffff\r\n"
            };

            for( core::string_view bad_chunk : samples )
            {
                pr.reset();
                pr.start();

                pieces in = { headers, bad_chunk };

                system::error_code ec;
                read_header(pr, in, ec);
                BOOST_TEST(! ec.failed());
                BOOST_TEST(pr.got_header());

                pr.parse(ec);
                BOOST_TEST_EQ(
                    ec, condition::need_more_input);

                read_some(pr, in, ec);
                BOOST_TEST_EQ(
                    ec, condition::invalid_payload);
                BOOST_TEST(!pr.is_complete());
            }
        }

        {
            // grind it out, emulate light fuzzing

            core::string_view headers =
                "HTTP/1.1 200 OK\r\n"
                "transfer-encoding: chunked\r\n"
                "\r\n";

            core::string_view body =
                "d\r\nhello, world!\r\n"
                "29\r\n and this is a much longer string of text\r\n"
                "0;d;e=3;f=\"4\"\r\n" // chunk extension
                "Expires: never\r\n" // trailer header
                "MD5-Fingerprint: -\r\n" // trailer header
                "\r\n";

            for( std::size_t i = 0; i < body.size(); ++i )
            {
                pr.reset();
                pr.start();

                pieces in = {
                    headers,
                    body.substr(0, i),
                    body.substr(i) };

                system::error_code ec;
                read_header(pr, in, ec);
                BOOST_TEST(! ec.failed());
                BOOST_TEST(pr.got_header());

                pr.parse(ec);
                BOOST_TEST_EQ(
                    ec, condition::need_more_input);

                read(pr, in, ec);
                BOOST_TEST(pr.is_complete());
                BOOST_TEST_EQ(
                    pr.body(),
                    "hello, world! and this is a much longer string of text");
            }
        }
    }

    void
    testMultipleMessageInPlace()
    {
        request_parser::config cfg;
        context ctx;

        cfg.headers.max_size = 500;
        cfg.min_buffer = 500;

        install_parser_service(ctx, cfg);
        system::error_code ec;

        request_parser pr(ctx);

        auto make_header = [](std::size_t n) -> std::string
        {
            return
                "GET / HTTP/1.1\r\n"
                "content-length: " + std::to_string(n) + "\r\n"
                "\r\n";
        };

        pr.reset();

        for( std::size_t i = 0; i < 2000; i += 1 )
        {
            pr.start();

            auto octets = make_header(i);
            auto remaining = i;

            while( remaining > 100 )
            {
                octets += std::string(100, 'a');
                remaining -= 100;

                pr.commit(buffers::buffer_copy(
                    pr.prepare(),
                    buffers::const_buffer(
                        octets.data(), octets.size())));

                pr.parse(ec);
                if(! ec)
                    pr.parse(ec);
                BOOST_TEST_EQ(
                    ec, condition::need_more_input);
                BOOST_TEST(!pr.is_complete());

                pr.consume_body(
                    buffers::buffer_size(pr.pull_body())
                    - 1); // left one byte so the circular buffer doesn't reset

                octets.clear();
            }

            // finalize the first message
            octets += std::string(remaining, 'a');
            // append second message
            octets += make_header(i % 100);
            octets += std::string(i % 100, 'a');

            pr.commit(buffers::buffer_copy(
                pr.prepare(),
                buffers::const_buffer(
                    octets.data(), octets.size())));

            // first message
            if(! pr.got_header())
            {
                pr.parse(ec);
                BOOST_TEST(! ec.failed());
                BOOST_TEST(pr.got_header());
            }
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());

            // second message
            pr.start();
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());
        }
        
    }

    void
    testMultipleMessageInPlaceChunked()
    {
        core::string_view headers =
            "GET / HTTP/1.1\r\n"
            "transfer-encoding: chunked\r\n"
            "\r\n";

        auto to_hex = [](std::size_t n)
        {
            std::string header(18, '0');
            auto c = std::snprintf(&header[0], 18, "%zx", n);
            header.erase(c);
            return header;
        };

        auto make_chunk = [=](std::size_t n)
        {
            auto header = to_hex(n);
            header += "\r\n";
            header += std::string(n, 'a');
            header += "\r\n";
            return header;
        };

        request_parser::config cfg;
        context ctx;

        cfg.min_buffer = 500;
        cfg.headers.max_size = 500;

        install_parser_service(ctx, cfg);
        system::error_code ec;
        request_parser pr(ctx);
        pr.reset();

        for( size_t i = 0; i < 2000  ; i += 1 )
        {
            pr.start();

            std::string octets = headers;
            octets += to_hex(1);
            auto remaining = i;

            while( remaining > 100 )
            {
                octets += "\r\na\r\n";
                octets += make_chunk(100);
                octets += to_hex(1);
                remaining -= 100;

                pr.commit(buffers::buffer_copy(
                    pr.prepare(),
                    buffers::const_buffer(
                        octets.data(), octets.size())));

                pr.parse(ec);
                if(! ec)
                    pr.parse(ec);
                BOOST_TEST_EQ(
                    ec, condition::need_more_input);
                BOOST_TEST(!pr.is_complete());

                pr.consume_body(
                    buffers::buffer_size(pr.pull_body()));

                octets.clear();
            }

            // finalize the first message
            octets += "\r\na\r\n";
            octets += make_chunk(0);
            // append second message
            octets += headers;
            octets += make_chunk(i % 100 + 1);
            octets += make_chunk(0);

            pr.commit(buffers::buffer_copy(
                pr.prepare(),
                buffers::const_buffer(
                    octets.data(), octets.size())));

            // first message
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());

            // second message
            pr.start();
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            pr.parse(ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());
        }
    }

    //-------------------------------------------

    void
    testSetBodyLimit()
    {
        context ctx;
        response_parser::config cfg;
        cfg.body_limit = 7;
        install_parser_service(ctx, cfg);

        response_parser pr(ctx);

        // before reset
        BOOST_TEST_THROWS(
            pr.set_body_limit(3),
            std::logic_error)

        pr.reset();

        // before start
        BOOST_TEST_THROWS(
            pr.set_body_limit(3),
            std::logic_error)

        pr.start();
        pr.set_body_limit(3);

        {
            pieces in = {
            "HTTP/1.1 200 OK\r\n"
            "content-length: 7\r\n"
            "\r\n"
            "1234567" };
            system::error_code ec;
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            read(pr, in, ec);
            BOOST_TEST_EQ(
                ec, error::body_too_large);
        }

        pr.reset();
        pr.start();

        // body_limit reset to cfg value
        // after start
        {
            pieces in = {
            "HTTP/1.1 200 OK\r\n"
            "content-length: 7\r\n"
            "\r\n"
            "1234567" };
            system::error_code ec;
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            read(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.is_complete());

        }

        // after completion with body
        BOOST_TEST(! pr.body().empty());
        BOOST_TEST_THROWS(
            pr.set_body_limit(3),
            std::logic_error)

        pr.start();

        // after completion of a bodiless message
        {
            pieces in = {
            "HTTP/1.1 200 OK\r\n"
            "content-length: 0\r\n"
            "\r\n" };
            system::error_code ec;
            read_header(pr, in, ec);
            BOOST_TEST(! ec.failed());
            BOOST_TEST(pr.got_header());
            BOOST_TEST(pr.is_complete());
            BOOST_TEST(pr.body().empty());
            pr.set_body_limit(3);
        }
    }

    void
    run()
    {
#if 1
        testSpecial();
        testConfig();
        testReset();
        testStart();
        testPrepare();
        testCommit();
        testCommitEof();
        testParse();
        testChunkedInPlace();
        testMultipleMessageInPlace();
        testMultipleMessageInPlaceChunked();
        testSetBodyLimit();
#else
        // For profiling
        for(int i = 0; i < 10000; ++i )
#endif
        {
            testParseHeader();
            testParseRequest();
            testParseResponse();
        }
    }
};

TEST_SUITE(
    parser_test,
    "boost.http_proto.parser");

} // http_proto
} // boost

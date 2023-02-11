//
// Copyright (c) 2022 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/CPPAlliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILE_STDIO_HPP
#define BOOST_HTTP_PROTO_FILE_STDIO_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/error.hpp>
#include <boost/http_proto/file_base.hpp>
#include <cstdio>
#include <cstdint>

namespace boost {
namespace http_proto {

/** An implementation of File which uses cstdio.

    This class implements a file using the interfaces present
    in the C++ Standard Library, in `<stdio>`.
*/
class file_stdio
{
    std::FILE* f_ = nullptr;

public:
    /** The type of the underlying file handle.

        This is platform-specific.
    */
    using native_handle_type = std::FILE*;

    /** Destructor

        If the file is open it is first closed.
    */
    BOOST_HTTP_PROTO_DECL
    ~file_stdio();

    /** Constructor

        There is no open file initially.
    */
    file_stdio() = default;

    /** Constructor

        The moved-from object behaves as if default constructed.
    */
    BOOST_HTTP_PROTO_DECL
    file_stdio(
        file_stdio&& other) noexcept;

    /** Assignment

        The moved-from object behaves as if default constructed.
    */
    BOOST_HTTP_PROTO_DECL
    file_stdio&
    operator=(
        file_stdio&& other) noexcept;

    /// Returns the native handle associated with the file.
    std::FILE*
    native_handle() const
    {
        return f_;
    }

    /** Set the native handle associated with the file.

        If the file is open it is first closed.

        @param f The native file handle to assign.
    */
    BOOST_HTTP_PROTO_DECL
    void
    native_handle(std::FILE* f);

    /// Returns `true` if the file is open
    bool
    is_open() const
    {
        return f_ != nullptr;
    }

    /** Close the file if open

        @param ec Set to the error, if any occurred.
    */
    BOOST_HTTP_PROTO_DECL
    void
    close(error_code& ec);

    /** Open a file at the given path with the specified mode

        @param path The utf-8 encoded path to the file

        @param mode The file mode to use

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    void
    open(char const* path, file_mode mode, error_code& ec);

    /** Return the size of the open file

        @param ec Set to the error, if any occurred

        @return The size in bytes
    */
    BOOST_HTTP_PROTO_DECL
    std::uint64_t
    size(error_code& ec) const;

    /** Return the current position in the open file

        @param ec Set to the error, if any occurred

        @return The offset in bytes from the beginning of the file
    */
    BOOST_HTTP_PROTO_DECL
    std::uint64_t
    pos(error_code& ec) const;

    /** Adjust the current position in the open file

        @param offset The offset in bytes from the beginning of the file

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    void
    seek(std::uint64_t offset, error_code& ec);

    /** Read from the open file

        @param buffer The buffer for storing the result of the read

        @param n The number of bytes to read

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    read(void* buffer, std::size_t n, error_code& ec) const;

    /** Write to the open file

        @param buffer The buffer holding the data to write

        @param n The number of bytes to write

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    write(void const* buffer, std::size_t n, error_code& ec);
};

} // http_proto
} // boost

#endif

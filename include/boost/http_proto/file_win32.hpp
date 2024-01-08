//
// Copyright (c) 2022 Vinnie Falco (vinnie.falco@gmail.com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILE_WIN32_HPP
#define BOOST_HTTP_PROTO_FILE_WIN32_HPP

#include <boost/http_proto/detail/config.hpp>

#if ! defined(BOOST_HTTP_PROTO_USE_WIN32_FILE)
# ifdef _WIN32
#  define BOOST_HTTP_PROTO_USE_WIN32_FILE 1
# else
#  define BOOST_HTTP_PROTO_USE_WIN32_FILE 0
# endif
#endif

#if BOOST_HTTP_PROTO_USE_WIN32_FILE

#include <boost/http_proto/error.hpp>
#include <boost/http_proto/file_base.hpp>
#include <boost/winapi/basic_types.hpp>
#include <boost/winapi/handles.hpp>
#include <cstdio>
#include <cstdint>

namespace boost {
namespace http_proto {

/** An implementation of File for Win32.

    This class implements a <em>File</em> using Win32 native interfaces.
*/
class file_win32
{
    boost::winapi::HANDLE_ h_ =
        boost::winapi::INVALID_HANDLE_VALUE_;

public:
    /** The type of the underlying file handle.

        This is platform-specific.
    */
#if BOOST_HTTP_PROTO_DOXYGEN
    using native_handle_type = HANDLE;
#else
    using native_handle_type = boost::winapi::HANDLE_;
#endif

    /** Destructor

        If the file is open it is first closed.
    */
    BOOST_HTTP_PROTO_DECL
    ~file_win32();

    /** Constructor

        There is no open file initially.
    */
    file_win32() = default;

    /** Constructor

        The moved-from object behaves as if default constructed.
    */
    BOOST_HTTP_PROTO_DECL
    file_win32(
        file_win32&& other) noexcept;

    /** Assignment

        The moved-from object behaves as if default constructed.
    */
    BOOST_HTTP_PROTO_DECL
    file_win32&
    operator=(
        file_win32&& other) noexcept;

    /// Returns the native handle associated with the file.
    native_handle_type
    native_handle()
    {
        return h_;
    }

    /** Set the native handle associated with the file.

        If the file is open it is first closed.

        @param h The native file handle to assign.
    */
    BOOST_HTTP_PROTO_DECL
    void
    native_handle(native_handle_type h);

    /// Returns `true` if the file is open
    bool
    is_open() const
    {
        return h_ != boost::winapi::INVALID_HANDLE_VALUE_;
    }

    /** Close the file if open

        @param ec Set to the error, if any occurred.
    */
    BOOST_HTTP_PROTO_DECL
    void
    close(system::error_code& ec);

    /** Open a file at the given path with the specified mode

        @param path The utf-8 encoded path to the file

        @param mode The file mode to use

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    void
    open(char const* path, file_mode mode, system::error_code& ec);

    /** Return the size of the open file

        @param ec Set to the error, if any occurred

        @return The size in bytes
    */
    BOOST_HTTP_PROTO_DECL
    std::uint64_t
    size(system::error_code& ec) const;

    /** Return the current position in the open file

        @param ec Set to the error, if any occurred

        @return The offset in bytes from the beginning of the file
    */
    BOOST_HTTP_PROTO_DECL
    std::uint64_t
    pos(system::error_code& ec);

    /** Adjust the current position in the open file

        @param offset The offset in bytes from the beginning of the file

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    void
    seek(std::uint64_t offset, system::error_code& ec);

    /** Read from the open file

        @param buffer The buffer for storing the result of the read

        @param n The number of bytes to read

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    read(void* buffer, std::size_t n, system::error_code& ec);

    /** Write to the open file

        @param buffer The buffer holding the data to write

        @param n The number of bytes to write

        @param ec Set to the error, if any occurred
    */
    BOOST_HTTP_PROTO_DECL
    std::size_t
    write(void const* buffer, std::size_t n, system::error_code& ec);
};

} // http_proto
} // boost

#endif

#endif

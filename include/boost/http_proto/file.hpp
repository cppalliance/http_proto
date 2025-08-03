//
// Copyright (c) 2022 Vinnie Falco (vinnie.falco@gmail.com)
// Copyright (c) 2025 Mohammad Nejati
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//
// Official repository: https://github.com/cppalliance/http_proto
//

#ifndef BOOST_HTTP_PROTO_FILE_HPP
#define BOOST_HTTP_PROTO_FILE_HPP

#include <boost/http_proto/detail/config.hpp>
#include <boost/http_proto/detail/file_posix.hpp>
#include <boost/http_proto/detail/file_stdio.hpp>
#include <boost/http_proto/detail/file_win32.hpp>
#include <boost/http_proto/file_mode.hpp>

namespace boost {
namespace http_proto {

/** A file stream.

    This class is intended for use with
    @ref file_sink and @ref file_source.

    @par Example
    @code
    file f;
    system::error_code ec;

    f.open("example.zip", file_mode::write_new, ec);
    if(ec.failed())
        throw system::system_error(ec);
    parser.set_body<file_sink>(std::move(file));
    @endcode
*/
class file
{
#if BOOST_HTTP_PROTO_USE_WIN32_FILE
    using impl_type = detail::file_win32;
#elif BOOST_HTTP_PROTO_USE_POSIX_FILE
    using impl_type = detail::file_posix;
#else
    using impl_type = detail::file_stdio;
#endif

    impl_type impl_;

public:
    /** The type of the underlying file handle.

        This is platform-specific.
    */
    using native_handle_type =
        impl_type::native_handle_type;

    /** Constructor

        There is no open file initially.
    */
    file() = default;

    /** Constructor

        The moved-from object behaves as if default constructed.
    */
    file(file&& other) noexcept = default;

    /** Assignment

        The moved-from object behaves as if default constructed.
    */
    file&
    operator=(
        file&& other) noexcept = default;

    /** Returns the native handle associated with the file.
    */
    native_handle_type
    native_handle()
    {
        return impl_.native_handle();
    }

    /** Set the native handle associated with the file.

        If the file is open it is first closed.

        @param fd The native file handle to assign.
    */
    void
    native_handle(native_handle_type h)
    {
        impl_.native_handle(h);
    }

    /** Returns `true` if the file is open
    */
    bool
    is_open() const
    {
        return impl_.is_open();
    }

    /** Close the file if open

        @param ec Set to the error, if any occurred.
    */
    void
    close(system::error_code& ec)
    {
        impl_.close(ec);
    }

    /** Open a file at the given path with the specified mode

        @param path The utf-8 encoded path to the file

        @param mode The file mode to use

        @param ec Set to the error, if any occurred
    */
    void
    open(char const* path, file_mode mode, system::error_code& ec)
    {
        impl_.open(path, mode, ec);
    }

    /** Return the size of the open file

        @param ec Set to the error, if any occurred

        @return The size in bytes
    */
    std::uint64_t
    size(system::error_code& ec) const
    {
        return impl_.size(ec);
    }

    /** Return the current position in the open file

        @param ec Set to the error, if any occurred

        @return The offset in bytes from the beginning of the file
    */
    std::uint64_t
    pos(system::error_code& ec) const
    {
        return impl_.pos(ec);
    }

    /** Adjust the current position in the open file

        @param offset The offset in bytes from the beginning of the file

        @param ec Set to the error, if any occurred
    */
    void
    seek(std::uint64_t offset, system::error_code& ec)
    {
        return impl_.seek(offset, ec);
    }

    /** Read from the open file

        @param buffer The buffer for storing the result of the read

        @param n The number of bytes to read

        @param ec Set to the error, if any occurred
    */
    std::size_t
    read(void* buffer, std::size_t n, system::error_code& ec)
    {
        return impl_.read(buffer, n, ec);
    }

    /** Write to the open file

        @param buffer The buffer holding the data to write

        @param n The number of bytes to write

        @param ec Set to the error, if any occurred
    */
    std::size_t
    write(void const* buffer, std::size_t n, system::error_code& ec)
    {
        return impl_.write(buffer, n, ec);
    }

    /** Destructor

        If the file is open it is first closed.
    */
    ~file() = default;
};

} // http_proto
} // boost

#endif

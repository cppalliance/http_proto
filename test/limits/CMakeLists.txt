#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/CPPAlliance/http_proto
#

set(TEST_MAIN ../../../url/extra/test_main.cpp)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES limits.cpp Jamfile)
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../../../url/extra PREFIX "_extra" FILES ${TEST_MAIN})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/../../src PREFIX "_extra" FILES ../../src/src.cpp)
add_executable(boost_http_proto_limits limits.cpp Jamfile ${TEST_MAIN} ../../src/src.cpp)
target_include_directories(boost_http_proto_limits PRIVATE ../../include ../../../url/extra ../../..)
target_compile_definitions(boost_http_proto_limits PRIVATE
    BOOST_HTTP_PROTO_MAX_HEADER=20
    BOOST_HTTP_PROTO_NO_LIB=1
    BOOST_HTTP_PROTO_STATIC_LINK
)
target_link_libraries(boost_http_proto_limits PRIVATE
    Boost::align
    Boost::assert
    Boost::buffers
    Boost::config
    Boost::container_hash
    Boost::system
    Boost::throw_exception
    Boost::url
    Boost::utility)

target_link_libraries(boost_http_proto_limits INTERFACE Boost::http_proto)
if (ZLIB_FOUND)
    target_link_libraries(boost_http_proto_limits INTERFACE ZLIB::ZLIB)
endif()
add_test(NAME boost_http_proto_limits COMMAND boost_http_proto_limits)

#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/cppalliance/http_proto
#

set(EXTRAFILES
    ../../../url/extra/test_main.cpp
    ../../../url/extra/test_suite.hpp
    ./test_helpers.hpp
    ./test_helpers.cpp
    )

file(GLOB_RECURSE PFILES CONFIGURE_DEPENDS *.cpp *.hpp)
list(APPEND PFILES
    CMakeLists.txt
    Jamfile
    )

set(UNIT_TEST_LINK_LIBRARIES boost_http_proto)
if (ZLIB_FOUND)
    set(UNIT_TEST_LINK_LIBRARIES ${UNIT_TEST_LINK_LIBRARIES} boost_http_proto_zlib)
endif()

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group("_extra" FILES ${EXTRAFILES})
add_executable(boost_http_proto_tests ${PFILES} ${EXTRAFILES})
target_include_directories(boost_http_proto_tests PRIVATE . ../../../url/extra)
target_link_libraries(
    boost_http_proto_tests PRIVATE
    ${UNIT_TEST_LINK_LIBRARIES}
    Boost::filesystem)
if (ZLIB_FOUND)
    target_link_libraries(boost_http_proto_tests PRIVATE ZLIB::ZLIB)
endif()
add_test(NAME boost_http_proto_tests COMMAND boost_http_proto_tests)

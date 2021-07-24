#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/http_proto
#


set(TFILES
    ./include/main.cpp
    ./include/test_suite.hpp
    )

set(PFILES
    CMakeLists.txt
    basic_parser.cpp
    context.cpp
    decoder.cpp
    error.cpp
    field.cpp
    fields_view.cpp
    method.cpp
    request_parser.cpp
    request_view.cpp
    response_parser.cpp
    status.cpp
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "include" FILES ${TFILES})
add_executable(http_proto_tests ${PFILES} ${TFILES})
target_include_directories(http_proto_tests PRIVATE . ./include)
target_link_libraries(http_proto_tests PRIVATE Boost::http_proto)

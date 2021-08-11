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
    connection.cpp
    context.cpp
    ctype.cpp
    decoder.cpp
    deflate_service.cpp
    encoder.cpp
    error.cpp
    field.cpp
    fields_view.cpp
    method.cpp
    request_parser.cpp
    request_view.cpp
    response_parser.cpp
    response_view.cpp
    rfc.cpp
    status.cpp
    string_view.cpp
    trivial_optional.cpp
    bnf/any_element.cpp
    bnf/any_list.cpp
    bnf/header_fields.cpp
    bnf/number.cpp
    bnf/optional_list.cpp
    bnf/range.cpp
    bnf/request_line.cpp
    bnf/required_list.cpp
    bnf/token.cpp
    bnf/token_list.cpp
    bnf/transfer_encoding_list.cpp
    bnf/transfer_param_list.cpp
    bnf/type_traits.cpp
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "include" FILES ${TFILES})
add_executable(http_proto_tests ${PFILES} ${TFILES})
target_include_directories(http_proto_tests PRIVATE . ./include)
target_link_libraries(http_proto_tests PRIVATE Boost::http_proto)

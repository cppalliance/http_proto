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
    arrow_proxy.cpp
    basic_header.cpp
    buffer.cpp
    context.cpp
    error.cpp
    field.cpp
    headers.cpp
    headers_view.cpp
    method.cpp
    parser.cpp
    request.cpp
    request_parser.cpp
    request_view.cpp
    response.cpp
    response_parser.cpp
    response_view.cpp
    serializer.cpp
    status.cpp
    string_view.cpp
    trivial_optional.cpp
    version.cpp
    bnf/algorithm.cpp
    bnf/char_set.cpp
    bnf/chunk_ext.cpp
    bnf/chunk_part.cpp
    bnf/connection.cpp
    bnf/ctype.cpp
    bnf/header_fields.cpp
    bnf/list.cpp
    bnf/number.cpp
    bnf/quoted_string.cpp
    bnf/range.cpp
    bnf/request_line.cpp
    bnf/sequence.cpp
    bnf/token.cpp
    bnf/transfer_encoding.cpp
    bnf/transfer_param_list.cpp
    bnf/type_traits.cpp
    codec/codecs.cpp
    codec/decoder.cpp
    codec/deflate_service.cpp
    codec/encoder.cpp
    mime/mime.cpp
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include PREFIX "include" FILES ${TFILES})
add_executable(http_proto_tests ${PFILES} ${TFILES})
target_include_directories(http_proto_tests PRIVATE . ./include)
target_link_libraries(http_proto_tests PRIVATE Boost::http_proto)

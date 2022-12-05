#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/http_proto
#

set(EXTRAFILES
    ../../../url/extra/test_main.cpp
    ../../../url/extra/test_suite.hpp
    ./test_helpers.hpp
    ./test_helpers.cpp
    )

set(PFILES
    CMakeLists.txt
    Jamfile
    buffer.cpp
    context.cpp
    error.cpp
    error_types.cpp
    field.cpp
    fields.cpp
    fields_base.cpp
    fields_view.cpp
    fields_view_base.cpp
    file.cpp
    file_base.cpp
    file_body.cpp
    file_posix.cpp
    file_stdio.cpp
    file_win32.cpp
    http_proto.cpp
    message_base.cpp
    message_view_base.cpp
    metadata.cpp
    method.cpp
    parser.cpp
    request.cpp
    request_parser.cpp
    request_view.cpp
    response.cpp
    response_parser.cpp
    response_view.cpp
    serializer.cpp
    source.cpp
    status.cpp
    string_body.cpp
    string_view.cpp
    version.cpp
    codec/codecs.cpp
    codec/decoder.cpp
    codec/deflate_service.cpp
    codec/encoder.cpp
    mime/mime_types.cpp
    #rfc/chunk_ext_rule.cpp
    rfc/combine_field_values.cpp
    rfc/list_rule.cpp
    rfc/quoted_token_rule.cpp
    rfc/token_rule.cpp
    rfc/transfer_encoding_rule.cpp
    rfc/detail/rules.cpp
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group("_extra" FILES ${EXTRAFILES})
add_executable(boost_http_proto_tests ${PFILES} ${EXTRAFILES})
target_include_directories(boost_http_proto_tests PRIVATE . ../../../url/extra)
target_link_libraries(boost_http_proto_tests PRIVATE Boost::http_proto Boost::filesystem)

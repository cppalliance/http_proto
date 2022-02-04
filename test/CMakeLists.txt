#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/vinniefalco/http_proto
#


set(EXTRAFILES
    ../../url/extra/test_main.cpp
    ../../url/extra/include/test_suite.hpp
    ./test_helpers.hpp
    )

set(PFILES
    CMakeLists.txt
    Jamfile
    arrow_proxy.cpp
    basic_header.cpp
    buffer.cpp
    context.cpp
    error.cpp
    field.cpp
    fields.cpp
    fields_view.cpp
    http_proto.cpp
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
    test_helpers.cpp
    version.cpp
    bnf/algorithm.cpp
    bnf/char_set.cpp
    bnf/chunk_part.cpp
    bnf/connection.cpp
    bnf/ctype.cpp
    bnf/list.cpp
    bnf/number.cpp
    bnf/range.cpp
    bnf/sequence.cpp
    bnf/test_rule.hpp
    bnf/token.cpp
    bnf/transfer_encoding.cpp
    bnf/transfer_param_list.cpp
    bnf/type_traits.cpp
    codec/codecs.cpp
    codec/decoder.cpp
    codec/deflate_service.cpp
    codec/encoder.cpp
    mime/mime_types.cpp
    rfc/charsets.cpp
    rfc/chunk_ext_rule.cpp
    rfc/crlf_rule.cpp
    rfc/field_rule.cpp
    rfc/list_rule.cpp
    rfc/method_rule.cpp
    rfc/ows_rule.cpp
    rfc/quoted_string_rule.cpp
    rfc/reason_phrase_rule.cpp
    rfc/request_line_rule.cpp
    rfc/request_target_rule.cpp
    rfc/status_code_rule.cpp
    rfc/status_line_rule.cpp
    rfc/token_rule.cpp
    rfc/version_rule.cpp
    )

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES ${PFILES})
source_group("_extra" FILES ${EXTRAFILES})
add_executable(boost_http_proto_tests ${PFILES} ${EXTRAFILES})
target_include_directories(boost_http_proto_tests PRIVATE . ../../url/extra/include)
target_link_libraries(boost_http_proto_tests PRIVATE Boost::http_proto)

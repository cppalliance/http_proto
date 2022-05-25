#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/CPPAlliance/http_proto
#

cmake_minimum_required(VERSION 3.8...3.16)

set(BOOST_HTTP_PROTO_VERSION 1)
if(BOOST_SUPERPROJECT_VERSION)
    set(BOOST_HTTP_PROTO_VERSION ${BOOST_SUPERPROJECT_VERSION})
endif()

project(boost_http_proto VERSION "${BOOST_HTTP_PROTO_VERSION}" LANGUAGES CXX)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_CURRENT_SOURCE_DIR)
    include(CTest)
    option(BOOST_HTTP_PROTO_INSTALL "Install boost::http_proto files" ON)
    option(BOOST_HTTP_PROTO_BUILD_TESTS "Build boost::http_proto tests" ${BUILD_TESTING})
    set(BOOST_HTTP_PROTO_IS_ROOT ON)
else()
    set(BOOST_HTTP_PROTO_BUILD_TESTS ${BUILD_TESTING})
    set(BOOST_HTTP_PROTO_IS_ROOT OFF)
endif()

include(GNUInstallDirs)
if(BOOST_HTTP_PROTO_IS_ROOT)
    set(BOOST_INCLUDE_LIBRARIES http_proto url)
    set(BOOST_EXCLUDE_LIBRARIES http_proto)
    set(CMAKE_FOLDER _deps)
    add_subdirectory(../.. _deps/boost EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
endif()

function(boost_http_proto_setup_properties target)
    target_compile_features(${target} PUBLIC cxx_constexpr)
    target_compile_definitions(${target} PUBLIC BOOST_HTTP_PROTO_NO_LIB=1)

    if(BOOST_SUPERPROJECT_VERSION)
        target_include_directories(${target} PUBLIC "${PROJECT_SOURCE_DIR}/include")
    else()
        target_include_directories(${target}
            PUBLIC
                "$<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>"
                "$<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>"
        )
    endif()

    target_link_libraries(${target}
        PUBLIC
            Boost::align
            Boost::assert
            Boost::config
            Boost::container
            Boost::exception
            Boost::optional
            Boost::system
            Boost::throw_exception
            Boost::url
            Boost::utility
    )
endfunction()

file(GLOB_RECURSE BOOST_HTTP_PROTO_HEADERS CONFIGURE_DEPENDS
    include/boost/*.hpp
    include/boost/*.ipp
    include/boost/*.natvis
)

set(BOOST_HTTP_PROTO_SOURCES src/src.cpp)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/boost PREFIX "" FILES ${BOOST_HTTP_PROTO_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "" FILES ${BOOST_HTTP_PROTO_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES Jamfile)

add_library(boost_http_proto ${BOOST_HTTP_PROTO_HEADERS} ${BOOST_HTTP_PROTO_SOURCES})
add_library(Boost::http_proto ALIAS boost_http_proto)
boost_http_proto_setup_properties(boost_http_proto)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_http_proto PUBLIC BOOST_HTTP_PROTO_DYN_LINK=1)
else()
    target_compile_definitions(boost_http_proto PUBLIC BOOST_HTTP_PROTO_STATIC_LINK=1)
endif()

if(BOOST_HTTP_PROTO_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
    install(TARGETS boost_http_proto
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    )
endif()

if(BOOST_HTTP_PROTO_BUILD_TESTS)
    add_subdirectory(test)
endif()

#
# Copyright (c) 2019 Vinnie Falco (vinnie.falco@gmail.com)
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
# Official repository: https://github.com/cppalliance/http_proto
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
    set(BOOST_HTTP_PROTO_BUILD_TESTS OFF CACHE BOOL "")
    set(BOOST_HTTP_PROTO_IS_ROOT OFF)
endif()

include(GNUInstallDirs)
if(BOOST_HTTP_PROTO_IS_ROOT)
    set(BOOST_INCLUDE_LIBRARIES http_proto buffers filesystem url)
    set(BOOST_EXCLUDE_LIBRARIES http_proto)
    set(CMAKE_FOLDER Dependencies)
    add_subdirectory(../.. Dependencies/boost EXCLUDE_FROM_ALL)
    unset(CMAKE_FOLDER)
endif()

find_package(ZLIB)

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
            Boost::buffers
            Boost::config
            Boost::container_hash
            Boost::system
            Boost::throw_exception
            Boost::url
            Boost::utility
            Boost::winapi
    )
    if (ZLIB_FOUND)
        target_compile_definitions(${target} PUBLIC BOOST_HTTP_PROTO_HAS_ZLIB)
    endif()
endfunction()

file(GLOB_RECURSE BOOST_HTTP_PROTO_HEADERS CONFIGURE_DEPENDS
    include/boost/*.hpp
    include/boost/*.ipp
    include/boost/*.natvis
)
file(GLOB_RECURSE BOOST_HTTP_PROTO_SOURCES CONFIGURE_DEPENDS src/*.cpp src/*.hpp)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/boost PREFIX "" FILES ${BOOST_HTTP_PROTO_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src PREFIX "http_proto" FILES ${BOOST_HTTP_PROTO_SOURCES})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} PREFIX "" FILES Jamfile)

add_library(boost_http_proto ${BOOST_HTTP_PROTO_HEADERS} ${BOOST_HTTP_PROTO_SOURCES} build/Jamfile)
add_library(Boost::http_proto ALIAS boost_http_proto)
boost_http_proto_setup_properties(boost_http_proto)

target_compile_definitions(boost_http_proto
  PUBLIC
  BOOST_HTTP_PROTO_NO_LIB
)

if(BUILD_SHARED_LIBS)
    target_compile_definitions(boost_http_proto PUBLIC BOOST_HTTP_PROTO_DYN_LINK=1)
else()
    target_compile_definitions(boost_http_proto PUBLIC BOOST_HTTP_PROTO_STATIC_LINK=1)
endif()
target_compile_definitions(boost_http_proto PRIVATE BOOST_HTTP_PROTO_SOURCE)

if(BOOST_HTTP_PROTO_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
    install(TARGETS boost_http_proto
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
    )
endif()

if (ZLIB_FOUND)
    file(GLOB_RECURSE BOOST_HTTP_PROTO_ZLIB_SOURCES CONFIGURE_DEPENDS src_zlib/*.cpp)

    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/include/boost PREFIX "" FILES ${BOOST_HTTP_PROTO_HEADERS})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/src_zlib PREFIX "http_proto" FILES ${BOOST_HTTP_PROTO_ZLIB_SOURCES})
    source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/build PREFIX "" FILES build/Jamfile)

    add_library(boost_http_proto_zlib ${BOOST_HTTP_PROTO_HEADERS} ${BOOST_HTTP_PROTO_ZLIB_SOURCES} build/Jamfile)
    add_library(Boost::http_proto_zlib ALIAS boost_http_proto_zlib)

    target_link_libraries(boost_http_proto_zlib PUBLIC boost_http_proto)
    target_link_libraries(boost_http_proto_zlib PUBLIC ZLIB::ZLIB)
    target_compile_definitions(boost_http_proto_zlib PUBLIC BOOST_HTTP_PROTO_HAS_ZLIB)
    target_compile_definitions(boost_http_proto_zlib PRIVATE BOOST_HTTP_PROTO_ZLIB_SOURCE)

    if(BOOST_HTTP_PROTO_INSTALL AND NOT BOOST_SUPERPROJECT_VERSION)
        install(TARGETS boost_http_proto_zlib
            RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
            LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
            ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        )
    endif()
endif()

if(BOOST_HTTP_PROTO_BUILD_TESTS)
    add_subdirectory(test)
endif()

#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  ХХ ХХХ 2026
 Website   :  na
 Copyright :  Damir Bakiev 2016-2026
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 4.0 FATAL_ERROR)

# set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
# set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

project(GERBER_X3 VERSION 0.9.9)

set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_UNITY_BUILD ON)
if(CMAKE_SYSTEM_NAME STREQUAL Linux)
    set(CMAKE_LINKER_TYPE MOLD)
endif()
set(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)

option(GERBER_NATIVE_MARCH "Optimize for host CPU architecture (disables portability)" ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
    add_compile_definitions(DEBUG=1)
endif()

# SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
# SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

add_compile_definitions(
    QT_MESSAGELOGCONTEXT
    QT_NO_CAST_TO_ASCII
    QT_NO_CAST_FROM_ASCII
    QT_NO_URL_CAST_FROM_STRING
    QT_NO_CAST_FROM_BYTEARRAY
    QT_USE_QSTRINGBUILDER
    QT_USE_FAST_OPERATOR_PLUS
    QT_DISABLE_DEPRECATED_BEFORE=0x070000
    saturate_cast=saturating_cast
    USINGZ=1)

set(COMMON_COMPILE_OPTIONS
    -fPIC
    -Wall
    -Wextra
    -Wimplicit-fallthrough
    -Wpedantic
    # -Og instead of the default -O0 in Debug: _FORTIFY_SOURCE (pulled in by
    # -fhardened) requires optimizations, otherwise GCC emits -Whardened.
    $<$<CONFIG:Debug>:-Og>)

if(GERBER_NATIVE_MARCH)
    list(APPEND COMMON_COMPILE_OPTIONS -march=native)
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    add_compile_options(${COMMON_COMPILE_OPTIONS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    add_compile_options(
        ${COMMON_COMPILE_OPTIONS}
        -fhardened
        -freflection
        -fstack-protector-strong
        # -fvtable-verify=preinit
        -fstack-clash-protection
        # -fcf-protection=full
    )
    # Security hardening link flags (RELRO + BIND_NOW)
    add_link_options(-Wl,-z,relro -Wl,-z,now)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
endif()

# FIND BOOST — version check is handled by find_package directly
find_package(Boost 1.85 REQUIRED)
message(STATUS "==> Boost ${Boost_VERSION} found")

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

set(CLIPPER2_UTILS OFF)
set(CLIPPER2_EXAMPLES OFF)
set(CLIPPER2_TESTS OFF)
set(CLIPPER2_USINGZ "ONLY")

FetchContent_Declare(
    ctre
    GIT_REPOSITORY "https://github.com/hanickadot/compile-time-regular-expressions.git"
    GIT_TAG main
    GIT_SHALLOW ON)
FetchContent_Declare(
    clipper2
    GIT_REPOSITORY "https://github.com/XRay3D/Clipper2.git"
    GIT_TAG main
    GIT_SHALLOW ON
    SOURCE_SUBDIR CPP)
FetchContent_MakeAvailable(ctre clipper2)

find_package(
    Qt6
    COMPONENTS Widgets LinguistTools Qml
    REQUIRED)

include(${CMAKE_SOURCE_DIR}/PATHS.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

include_directories(ggeasy)

# STATIC LIBS
add_subdirectory(static_libs)

# PLUGINS
add_subdirectory(plugins)

# MAIN APP
add_subdirectory(ggeasy)

# add_subdirectory(ToolTest)

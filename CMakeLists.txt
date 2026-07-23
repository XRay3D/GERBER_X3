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
cmake_minimum_required(VERSION 4.00 FATAL_ERROR)

# set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
# set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

project(GERBER_X3 VERSION 0.9.9)

# set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD 26)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_SCAN_FOR_MODULES OFF)

set(CMAKE_UNITY_BUILD ON)
set(CMAKE_LINKER_TYPE MOLD)

set_source_files_properties(PROPERTIES SKIP_UNITY_BUILD_INCLUSION ON)

if(CMAKE_BUILD_TYPE STREQUAL Debug)
  add_definitions(-DDEBUG=1)
endif()

# SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
# SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

set_property(GLOBAL PROPERTY CXX_STANDARD 26)
set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_definitions(
  -DQT_MESSAGELOGCONTEXT
  -DQT_NO_CAST_TO_ASCII
  -DQT_NO_CAST_FROM_ASCII
  -DQT_NO_URL_CAST_FROM_STRING
  -DQT_NO_CAST_FROM_BYTEARRAY
  -DQT_USE_QSTRINGBUILDER
  # * DQT_NO_KEYWORDS QT_NO_SIGNALS_SLOTS_KEYWORDS
  -DQT_USE_FAST_OPERATOR_PLUS
  -DQT_DISABLE_DEPRECATED_BEFORE=0x070000
  -Dsaturate_cast=saturating_cast)

set(COMMON_COMPILE_OPTIONS
    -fPIC
    # * ffast - math
    -march=native
    # * xc ++ - system - header iostream
    -Wall
    # * Wconversion - Werror
    -Wextra
    -Wimplicit-fallthrough
    -Wpedantic
    # -Og instead of the default -O0 in Debug: _FORTIFY_SOURCE (pulled in by
    # -fhardened) requires optimizations, otherwise GCC emits -Whardened.
    $<$<CONFIG:Debug>:-Og>)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(${COMMON_COMPILE_OPTIONS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  # add_definitions(-D_FORTIFY_SOURCE=3 -D_GLIBCXX_ASSERTIONS)
  add_compile_options(
    ${COMMON_COMPILE_OPTIONS}
    # -O1
    -fhardened
    -freflection
    -fstack-protector-strong
    # -fvtable-verify=preinit#std
    -fstack-clash-protection
    # -fcf-protection=full
  )
  link_libraries(stdc++exp -Wl,-z,relro -Wl,-z,now)
  add_link_options(-fuse-ld=lld)
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

endif()

# FIND BOOST
find_package(Boost)
if(Boost_FOUND)
  if(Boost_VERSION VERSION_LESS 1.85)
    message(
      FATAL_ERROR
        "==> !! Required Boost version is greater than or equal to 1.85.0! Current ${Boost_VERSION}."
    )
  endif()
  include_directories(${Boost_INCLUDE_DIR})
  message(==> "Boost ${Boost_VERSION} found!!!")
else()
  message(FATAL_ERROR "Boost not found!!!")
endif()

include(FetchContent)
set(FETCHCONTENT_BASE_DIR ${CMAKE_CURRENT_LIST_DIR}/third_party)

set(CLIPPER2_UTILS OFF)
set(CLIPPER2_EXAMPLES OFF)
set(CLIPPER2_TESTS OFF)
set(CLIPPER2_USINGZ "ONLY")
add_definitions(-DUSINGZ=1)
# add_subdirectory(third_party/clipper2-src/CPP)

# FetchContent_Declare( pfr # GIT_REPOSITORY https: //
# github.com/boostorg/pfr.git GIT_REPOSITORY
# https://github.com/apolukhin/pfr_non_boost.git GIT_TAG master GIT_SHALLOW ON)
FetchContent_Declare(
  ctre
  GIT_REPOSITORY
    "https://github.com/hanickadot/compile-time-regular-expressions.git"
  GIT_TAG main
  GIT_SHALLOW ON)
FetchContent_Declare(
  clipper2
  GIT_REPOSITORY "https://github.com/XRay3D/Clipper2.git"
  GIT_TAG main
  GIT_SHALLOW ON
  SOURCE_SUBDIR CPP)
FetchContent_MakeAvailable(#[[pfr]] ctre clipper2)

#[[FIND CGAL
find_package(CGAL)
if(CGAL_FOUND)
#if (CGAL_VERSION_MINOR LESS 84) message(FATAL_ERROR "==> !! Required CGAL
#version is greater than or equal to 1.84.0 !Current ${CGAL_VERSION }." )
#endif()
  include_directories(${CGAL_INCLUDE_DIR})
  message(==> "CGAL ${CGAL_VERSION} found!!!")
else()
  message(FATAL_ERROR "CGAL not found!!!")
endif()

#FIND EIGEN3
find_package(EIGEN3)
if(EIGEN3_FOUND)
#if (eigen_VERSION_MINOR LESS 84) message(FATAL_ERROR "==> !! Required eigen
#version is greater than or equal to 1.84.0 !Current ${eigen_VERSION }." )
#endif()
  include_directories(${EIGEN3_INCLUDE_DIR})
  message(==> "EIGEN3 ${EIGEN3_VERSION} found!!!")
else()
  message(FATAL_ERROR "EIGEN3 not found!!!")
endif()
]]

include_directories(ggeasy)

# CXX_MODULES FILES static_libs / common / std.cppm)

# target_compile_definitions(Clipper2Z USINGZ=1) get_target_property(defs
# Clipper2Z COMPILE_DEFINITIONS) message(FATAL_ERROR ${defs}) list(FILTER defs
# EXCLUDE REGEX "_FORTIFY_SOURCE") # Отфильтровываем нужное
# set_target_properties(Clipper2Z PROPERTIES COMPILE_DEFINITIONS ${defs})

# add_subdirectory(third_party / clipper2 / CPP)

# set(CTRE_MODULE ON) add_subdirectory(third_party / ctre)

find_package(
  Qt6
  COMPONENTS Widgets LinguistTools Qml
  REQUIRED)

# qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ru en)

include(${CMAKE_SOURCE_DIR}/PATHS.cmake)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

# STATIC LIBS
add_subdirectory(static_libs)

# PLUGINS
add_subdirectory(plugins)

# MAIN APP
add_subdirectory(ggeasy)

# add_subdirectory(ToolTest)

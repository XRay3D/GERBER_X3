#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  ХХ ХХХ 2025
 Website   :  na
 Copyright :  Damir Bakiev 2016-2025
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

# set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
# set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

project(GERBER_X3 VERSION 0.9.1)

# set(CMAKE_CXX_MODULE_STD ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
# SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

set_property(GLOBAL PROPERTY CXX_STANDARD 23)
set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_definitions(
  -DQT_MESSAGELOGCONTEXT # qDebug()
  -DQT_DISABLE_DEPRECATED_BEFORE=0x060400
  -DQT_NO_CAST_TO_ASCII=1 # ERORR QByteArray &appendToByteArray(QByteArray &a,
  # const QStringBuilder < A, B>& b, QChar)
)

# add_compile_options( #
# $<IF:$OR<$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>, -fPIC -Wpedantic #
# -Wall -Wextra -Wconversion -Werror> #
# $<IF:$<AND:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:GNU>>, -fPIC #
# -Wpedantic -Wall -Wextra -Wconversion -Werror>
# $<$<AND:$<CXX_COMPILER_ID:GNU>,$<CXX_COMPILER_ID:Clang>>:-fPIC -Wpedantic
# -Wall -Wextra -Wconversion -Werror> #
# $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Debug>>:/Od> #
# $<$<AND:$<CXX_COMPILER_ID:MSVC>,$<CONFIG:Release>>:/O2>) ) #
# target_compile_options(my_target PRIVATE $ < $ < AND : $ < CXX_COMPILER_ID : #
# GNU>, $ < CONFIG : Debug>> : -Og> $ < $ < AND : $ < CXX_COMPILER_ID : GNU>, $
# # < CONFIG : Release>> : -O2> $ < $ < AND : $ < CXX_COMPILER_ID : MSVC>, $ < #
# CONFIG : Debug>> : / Od> $ < $ < AND : $ < CXX_COMPILER_ID : MSVC>, $ < CONFIG
# # : Release>> : / O2>)

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

# add_library(Qt INTERFACE) target_precompile_headers(Qt INTERFACE qt_pch.h)

include_directories(
  ggeasy
  plugins/file
  plugins/gcode
  plugins/shape
  static_libs/clipper
  static_libs/common
  static_libs/filetree
  static_libs/gi
  static_libs/graphicsview
  static_libs/tooldatabase
  third_party/clipper2/CPP/Clipper2Lib/include
  third_party/ctre/single-header)

# add_library(std MODULE) target_sources(std PUBLIC FILE_SET all_my_modules TYPE
# CXX_MODULES FILES static_libs/common/std.cppm)

set(CLIPPER2_UTILS OFF)
set(CLIPPER2_EXAMPLES OFF)
set(CLIPPER2_TESTS OFF)
set(CLIPPER2_USINGZ "ONLY")
add_definitions(-DUSINGZ=1)

add_subdirectory(third_party/clipper2/CPP)

set(CTRE_MODULE ON)
add_subdirectory(third_party/ctre)

set(COMMON_COMPILE_OPTIONS
    -fPIC
    # -ffast-math
    -march=native
    # -xc++-system-header iostream
    -Wall
    # -Wconversion
    -Werror
    -Wextra
    -Wimplicit-fallthrough
    -Wpedantic)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
  add_compile_options(${COMMON_COMPILE_OPTIONS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(${COMMON_COMPILE_OPTIONS})
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")

endif()

# add_library(ctre) # third_party/ctre/ctre.cppm) target_sources(ctre PUBLIC
# FILE_SET cxx_modules TYPE CXX_MODULES FILES third_party/ctre/ctre.cppm)
# target_compile_options(Clipper2Z PUBLIC - fPIC)

find_package(QT NAMES Qt6 REQUIRED COMPONENTS Core) # Qt6

find_package(
  Qt6
  COMPONENTS Widgets LinguistTools
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

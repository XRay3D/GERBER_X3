#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  03 October 2024
 Website   :  na
 Copyright :  Damir Bakiev 2016-2024
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 3.24)

# set_property(GLOBAL PROPERTY RULE_LAUNCH_COMPILE "${CMAKE_COMMAND} -E time")
# set_property(GLOBAL PROPERTY RULE_LAUNCH_LINK "${CMAKE_COMMAND} -E time")

project(GERBER_X3 VERSION 0.9.0)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

# SET(CMAKE_FIND_PACKAGE_SORT_ORDER NATURAL)
# SET(CMAKE_FIND_PACKAGE_SORT_DIRECTION DEC)

set_property(GLOBAL PROPERTY CXX_STANDARD 20)
set_property(GLOBAL PROPERTY INTERPROCEDURAL_OPTIMIZATION TRUE)

add_definitions(
  -DQT_MESSAGELOGCONTEXT # qDebug()
  -DQT_DISABLE_DEPRECATED_BEFORE=0x060000
  -DQT_NO_CAST_TO_ASCII=1 # ERORR QByteArray &appendToByteArray(QByteArray &a,
                          # const QStringBuilder<A, B> &b, QChar)
)

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
  add_compile_options(
    -Winvalid-pch -pthread
    # -fmodules-ts
  )
  link_directories(tbb/lib)
  # Link against the dependency of Intel TBB (for parallel C++17 algorithms)
  # target_link_libraries(${PROJECT_NAME} tbb)

  # set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FORMAT "gcc")
  # set(CMAKE_EXPERIMENTAL_CXX_MODULE_MAP_FLAG "${compiler_flags_for_module_map}
  # -fmodule-mapper=<MODULE_MAP_FILE>") target_compile_options(example PRIVATE
  # -fmodules-ts) add_compile_options(-H) add_compile_options(-ftime-report)
  # -pthread -ltbbD
elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Intel")

elseif(CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  # add_definitions(-D_USE_MATH_DEFINES=1)
endif()

# FIND BOOST
find_package(Boost)
if(Boost_FOUND)
  if(Boost_VERSION_MINOR LESS 84)
    message(
      FATAL_ERROR
        "==> !! Required Boost version is greater than or equal to 1.84.0! Current ${Boost_VERSION}."
    )
  endif()
  include_directories(${Boost_INCLUDE_DIR})
  message(==> "Boost ${Boost_VERSION} found!!!")
else()
  message(FATAL_ERROR "Boost not found!!!")
endif()

set(INC_DIRS
    third_party/ctre/single-header/
    third_party/clipper2/CPP/Clipper2Lib/include/
    ggeasy
    plugins/file
    plugins/gcode
    plugins/shape
    static_libs/clipper
    static_libs/common
    static_libs/filetree
    static_libs/gi
    static_libs/graphicsview
    static_libs/tooldatabase)

# add_library(Qt INTERFACE) target_precompile_headers(Qt INTERFACE qt_pch.h)

set(CLIPPER2_UTILS OFF)
set(CLIPPER2_EXAMPLES OFF)
set(CLIPPER2_TESTS OFF)
set(CLIPPER2_USINGZ OFF)

add_subdirectory(third_party/clipper2/CPP)

include(${CMAKE_SOURCE_DIR}/PATHS.cmake)
include_directories(${INC_DIRS})

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

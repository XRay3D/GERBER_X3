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

project(common LANGUAGES CXX)

find_package(
  Qt6
  COMPONENTS Qml
  REQUIRED)

# set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL ON) # fix Qml

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(common STATIC ${HEADERS} ${SOURCES})
# target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

if(${CMAKE_HOST_SYSTEM_NAME} STREQUAL Linux)
  set(EXECUTION tbb)
else()
  set(EXECUTION tbb12)
endif()

target_link_libraries(
  common
  PRIVATE filetree shape gi graphicsview Qt6::Widgets
  PUBLIC #[[tbb12]] ${EXECUTION} Qt6::Qml)

# add_subdirectory(doublespinbox)

# add_module_library(QT qt.cpp)

# target_compile_definitions(common PRIVATE COMMON_LIBRARY)

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
cmake_minimum_required(VERSION 3.20)

project(cgal LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Widgets
  REQUIRED)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(cgal STATIC ${HEADERS} ${SOURCES})

target_link_libraries(cgal PRIVATE graphicsview
                                         Qt${QT_VERSION_MAJOR}::Widgets)

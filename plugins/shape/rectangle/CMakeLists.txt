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

project(shrectangle LANGUAGES CXX)

set(PLUG_NAME "Rectangle")
set(PLUG_INFO "Rectangle")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "1")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_SOURCE_DIR}/description.json NEWLINE_STYLE UNIX)

file(GLOB PROJECT_SOURCES *.json *.cpp *.h)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE clipper
          common
          filetree
          gi
          graphicsview
          shape
          tooldatabase
          Qt${QT_VERSION_MAJOR}::Widgets)

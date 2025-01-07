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

project(gcthermal LANGUAGES CXX)

set(PLUG_NAME "Thermal")
set(PLUG_INFO "Thermal")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "0")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_SOURCE_DIR}/description.json NEWLINE_STYLE UNIX)

file(GLOB HEADERS *.h)
file(GLOB JSONS *.json)
file(GLOB SOURCES *.cpp)
file(GLOB UIS *.ui)

include_directories(${CMAKE_STATIC_LIBS_DIR})

set(PROJECT_SOURCES ${HEADERS} ${JSONS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE clipper
          gcode
          common
          filetree
          gi
          graphicsview
          tooldatabase
          Qt${QT_VERSION_MAJOR}::Widgets)

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
cmake_minimum_required(VERSION 4.0)

project(file_dxf LANGUAGES CXX)

set(PLUG_NAME "DXF")
set(PLUG_INFO "Opening DXF Files")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "2")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/description.json NEWLINE_STYLE UNIX)

file(GLOB_RECURSE HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS *.cpp)
file(GLOB_RECURSE UIS CONFIGURE_DEPENDS *.ui)
file(GLOB JSONS CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/*.json)

set(PROJECT_SOURCES ${HEADERS} ${JSONS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE geometry
            common
            filetree
            gi
            graphicsview
            tooldatabase
            file_plugins
            Qt6::Widgets)

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UIS})

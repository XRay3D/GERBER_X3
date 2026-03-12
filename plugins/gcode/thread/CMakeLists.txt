#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  03 October 2022
 Website   :  na
 Copyright :  Damir Bakiev 2016-2022
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 4.00)

project(thread LANGUAGES CXX)



find_package(Qt6 COMPONENTS  Widgets REQUIRED)

file(GLOB HEADERS *.h)
file(GLOB JSONS *.json)
file(GLOB SOURCES *.cpp)
file(GLOB UIS *.ui)


set(PROJECT_SOURCES
    ${HEADERS}
    ${JSONS}
    ${SOURCES}
    ${UIS}
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE
    clipper
    common
    filetree
    gcode
    gi
    graphicsview
    tooldatabase
    Qt6::Widgets
)

#add_library(gc_profile STATIC
#    profile.cpp
#    profile.h
#)

#target_link_libraries(gc_profile PRIVATE
#    clipper
#    gcode
#    common
#    filetree
#    Qt6::Core
#    Qt6::Gui
#    Qt6::Widgets
#)

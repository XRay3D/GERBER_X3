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
cmake_minimum_required(VERSION 3.20)

project(thread LANGUAGES CXX)



find_package(Qt${QT_VERSION_MAJOR} COMPONENTS  Widgets REQUIRED)

file(GLOB HEADERS *.h)
file(GLOB JSONS *.json)
file(GLOB SOURCES *.cpp)
file(GLOB UIS *.ui)

include_directories(${CMAKE_STATIC_LIBS_DIR})

set(PROJECT_SOURCES
    ${HEADERS}
    ${JSONS}
    ${SOURCES}
    ${UIS}
)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

#target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE
    clipper
    common
    filetree
    gcode
    gi
    graphicsview
    tooldatabase
    Qt${QT_VERSION_MAJOR}::Widgets
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
#    Qt${QT_VERSION_MAJOR}::Core
#    Qt${QT_VERSION_MAJOR}::Gui
#    Qt${QT_VERSION_MAJOR}::Widgets
#)

cmake_minimum_required(VERSION 3.14)

project(rectangle LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_PLUGINS_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_PLUGINS_DIR})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_PLUGINS_DIR})


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

include_directories(${CMAKE_STATIC_LIBS_DIR})

add_library(rectangle SHARED
    rectangle.json
    shrectangle.cpp
    shrectangle.h
    ${SOURCE}
    ${HEADERS}
    )


target_link_libraries(rectangle PRIVATE
#    clipper
#    filetree
#    gi
#    graphicsview
#    project
#    settings
#    tooldatabase
    )

target_link_libraries(rectangle PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

target_compile_definitions(rectangle PRIVATE RECTANGLE_LIBRARY)

cmake_minimum_required(VERSION 3.14)

project(text LANGUAGES CXX)

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

add_library(text SHARED
    text.json
    shtext.cpp
    shtext.h
    shtextdialog.cpp
    shtextdialog.h
    shtextdialog.ui
    ${SOURCE}
    ${HEADERS}
    )


target_link_libraries(text PRIVATE
#    clipper
#    filetree
#    gi
#    graphicsview
#    project
#    settings
#    tooldatabase
    )

target_link_libraries(text PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

target_compile_definitions(text PRIVATE TEXT_LIBRARY)

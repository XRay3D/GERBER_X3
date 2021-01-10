cmake_minimum_required(VERSION 3.14)

project(shape LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets OpenGL REQUIRED)

add_library(shape STATIC
    shape.cpp
    shape.h
    shcreator.cpp
    shcreator.h
    shhandler.cpp
    shhandler.h
    shheaders.h
    shnode.cpp
    shnode.h
    )

target_link_libraries(shape PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

target_compile_definitions(shape PRIVATE SHAPE_LIBRARY)

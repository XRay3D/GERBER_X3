cmake_minimum_required(VERSION 3.14)

project(clipper LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui REQUIRED)

add_library(clipper STATIC
    clipper.hpp
    #    cpline.h
    myclipper.h
    clipper.cpp
    #    cpline.cpp
    myclipper.cpp
    )

target_link_libraries(clipper PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(clipper PRIVATE Qt${QT_VERSION_MAJOR}::Gui)

target_compile_definitions(clipper PRIVATE CLIPPER_LIBRARY)

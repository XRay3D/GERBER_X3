cmake_minimum_required(VERSION 3.14)

project(common LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

add_library(common STATIC
    app.h
    colorselector.h
    datastream.h
    depthform.h
    doublespinbox.h
    leakdetector.h
    settings.h
    colorselector.cpp
    depthform.cpp
    doublespinbox.cpp
    settings.cpp
    project.h
    project.cpp
    colorselector.ui
    )

target_link_libraries(common PRIVATE
    filetree
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

target_compile_definitions(common PRIVATE COMMON_LIBRARY)

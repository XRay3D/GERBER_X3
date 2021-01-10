cmake_minimum_required(VERSION 3.14)

project(thermal LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

add_library(thermal STATIC
    thermalform.ui
    thermal.h
    thermaldelegate.h
    thermalform.h
    thermalmodel.h
    thermalnode.h
    thvars.h
    thermaldelegate.cpp
    thermalform.cpp
    thermalmodel.cpp
    thermalnode.cpp
    )

target_link_libraries(thermal PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(thermal PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(thermal PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_compile_definitions(thermal PRIVATE THERMAL_LIBRARY)

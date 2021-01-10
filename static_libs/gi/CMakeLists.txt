cmake_minimum_required(VERSION 3.14)

project(gi LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

add_library(gi STATIC
    bridgeitem.h
    componentitem.h
    datapathitem.h
    datasoliditem.h
    drillitem.h
    drillpreviewgi.h
    erroritem.h
    gcpathitem.h
    graphicsitem.h
    itemgroup.h
    thermalpreviewitem.h
    bridgeitem.cpp
    componentitem.cpp
    datapathitem.cpp
    datasoliditem.cpp
    drillitem.cpp
    drillpreviewgi.cpp
    erroritem.cpp
    gcpathitem.cpp
    graphicsitem.cpp
    itemgroup.cpp
    thermalpreviewitem.cpp
)

target_link_libraries(gi PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(gi PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(gi PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_compile_definitions(gi PRIVATE GI_LIBRARY)

cmake_minimum_required(VERSION 3.5)

project(ToolTest VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES #[[Qt6]] Qt5 REQUIRED COMPONENTS Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets)

file(GLOB PROJECT_SOURCES
    *.cpp
    *.h
    *.ui
    ../static_libs/tooldatabase/*.cpp
    ../static_libs/tooldatabase/*.h
    ../static_libs/tooldatabase/*.ui

    ../static_libs/common/dou*
    ../static_libs/common/dou*
)

#include_directories(
#    ../static_libs/tooldatabase
#)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(ToolTest
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
    )
else()
    add_executable(ToolTest
        ${PROJECT_SOURCES}
    )
endif()

target_link_libraries(ToolTest PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
)

set_target_properties(ToolTest PROPERTIES
    MACOSX_BUNDLE_GUI_IDENTIFIER my.example.com
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

install(TARGETS ToolTest
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(ToolTest)
endif()

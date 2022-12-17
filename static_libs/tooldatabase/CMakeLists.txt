#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  03 October 2022                                                 *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License   :                                                                  *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#********************************************************************************/
cmake_minimum_required(VERSION 3.20)


project(tooldatabase LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Gui REQUIRED)

#set(TS_FILES tooldatabase_af_NA.ts)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

add_library(tooldatabase STATIC
    ${HEADERS}
    ${SOURCES}
    ${UIS}
)

target_link_libraries(tooldatabase PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(tooldatabase PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(tooldatabase PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_compile_definitions(tooldatabase PRIVATE TOOLDATABASE_LIBRARY)

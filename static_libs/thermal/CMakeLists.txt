#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  11 November 2021                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
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

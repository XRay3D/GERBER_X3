#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  14 January 2021                                                 *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.14)

project(tooldatabase LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
##set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)


find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets Gui REQUIRED)

#set(TS_FILES tooldatabase_af_NA.ts)

add_library(tooldatabase STATIC
    tooldatabase.ui
    tooleditdialog.ui
    tooleditform.ui
    tool.h
    tooldatabase.h
    tooleditdialog.h
    tooleditform.h
    toolitem.h
    toolmodel.h
    tooltreeview.h
    tool.cpp
    tooldatabase.cpp
    tooleditdialog.cpp
    tooleditform.cpp
    toolitem.cpp
    toolmodel.cpp
    tooltreeview.cpp
    toolselectorform.h
    toolselectorform.cpp
    )

target_link_libraries(tooldatabase PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(tooldatabase PRIVATE Qt${QT_VERSION_MAJOR}::Gui)
target_link_libraries(tooldatabase PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)

target_compile_definitions(tooldatabase PRIVATE TOOLDATABASE_LIBRARY)

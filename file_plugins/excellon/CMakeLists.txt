#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.16)

project(excellon LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
#set(CMAKE_CXX_STANDARD 17)
#set(CMAKE_CXX_STANDARD_REQUIRED ON)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PLUGINS_DIR})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_DIR})
#set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})

find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

include_directories(${CMAKE_STATIC_LIBS_DIR})

add_library(excellon SHARED
    drillitem.cpp
    drillitem.h
    excellon.h
    excellondialog.cpp
    excellondialog.h
    excellondialog.ui
    exfile.cpp
    exfile.h
    exformatstate.cpp
    exh.cpp
    exh.h
    exnode.cpp
    exnode.h
    exparser.cpp
    exparser.h
    explugin.cpp
    explugin.h
    exsettingstab.cpp
    exsettingstab.h
    extypes.h
    excellon.json
    ${SOURCE}
    ${HEADERS}
    )


target_link_libraries(excellon PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
    tooldatabase
    )

target_link_libraries(excellon PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
    )

#install(TARGETS ${PROJECT_NAME} DESTINATION ${PLUGINS_DIR})

#target_compile_definitions(excellon PRIVATE EXCELLON_LIBRARY)

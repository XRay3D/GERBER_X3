#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "GGEasy")

project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# QtCreator supports the following variables for Android, which are identical to qmake Android variables.
# Check http://doc.qt.io/qt-5/deployment-android.html for more information.
# They need to be set before the find_package(Qt5 ...) call.

#if(ANDROID)
# set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
# if (ANDROID_ABI STREQUAL "armeabi-v7a")
# set(ANDROID_EXTRA_LIBS
# ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libcrypto.so
# ${CMAKE_CURRENT_SOURCE_DIR}/path/to/libssl.so)
# endif()
#endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(SUFFIX "_clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(SUFFIX "_gnu")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(SUFFIX "_intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SUFFIX "_msvc")
endif()

#if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
#if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
if(CMAKE_CL_64)
    set(SUFFIX "${SUFFIX}_x64")
else()
    set(SUFFIX "${SUFFIX}_x32")
endif()

if(CMAKE_BUILD_TYPE MATCHES Debug)
    set(SUFFIX "${SUFFIX}_d")
elseif(CMAKE_BUILD_TYPE MATCHES Release)
    set(SUFFIX "${SUFFIX}_r")
elseif(CMAKE_BUILD_TYPE MATCHES MinSizeRel)
    set(SUFFIX "${SUFFIX}_rm")
elseif(CMAKE_BUILD_TYPE MATCHES RelWithDebInfo)
    set(SUFFIX "${SUFFIX}_rd")
endif()

string(TIMESTAMP DT "%d.%m.%Y - %H:%M:%S")# [<format string>] [UTC])
add_definitions(-DBUILD_DATE="${DT}")

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_GX3_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_GX3_DIR}/bin)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin")

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Concurrent
    Core
    Gui
    LinguistTools
    OpenGL
    PrintSupport
    Widgets
    REQUIRED)
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    find_package(Qt${QT_VERSION_MAJOR} COMPONENTS OpenGLWidgets REQUIRED)
endif()

set(TS_FILES
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}_en.ts"
    "${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}_ru.ts"
    )

#include_directories(${CMAKE_STATIC_LIBS_DIR})

include_directories (
    ../static_libs/clipper
    ../static_libs/common
    ../static_libs/filetree
    ../static_libs/gerber
    ../static_libs/gi
    ../static_libs/graphicsview
    ../static_libs/shape
    ../static_libs/thermal
    ../static_libs/tooldatabase
    )

link_directories(../libs${SUFFIX})

add_subdirectory(forms)
add_subdirectory(forms/drillform)
add_subdirectory(forms/formsutil)

add_subdirectory(gcode)
add_subdirectory(gcode/voroni)

add_subdirectory(interfaces)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations ${TS_FILES})
    qt6_add_resources(RESOURCES res/resources.qrc)
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations ${TS_FILES})
    qt5_add_resources(RESOURCES res/resources.qrc)
endif()

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/myapp.rc")

set(SOURCE
    ${SOURCE}
    aboutform.cpp
    main.cpp
    mainwindow.cpp
    plugindialog.cpp
    point.cpp
    recent.cpp
    settingsdialog.cpp
    )

set(HEADERS
    ${HEADERS}
    aboutform.h
    mainwindow.h
    mvector.h
    openingdialog.h
    plugindialog.h
    point.h
    recent.h
    settingsdialog.h
    splashscreen.h
    version.h
    )

set(UI
    ${UI}
    aboutform.ui
    colorsetup.ui
    mainwindow.ui
    settingsdialog.ui
    )


add_executable(${PROJECT_NAME}
    ${TS_FILES}
    ${SOURCE}
    ${HEADERS}
    ${UI}
    ${RESOURCES}
    ${APP_ICON_RESOURCE_WINDOWS}
    )

target_link_libraries(${PROJECT_NAME} PRIVATE
    clipper
    filetree
    gi
    graphicsview
    common
    thermal
    tooldatabase
    shape
    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Widgets
    )
if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}${SUFFIX}")

#add_custom_command (
#    OUTPUT ${TS_FILES}
#    COMMAND lupdate -silent -I ./includes ./sources -ts ./project.ts ${CMAKE_CURRENT_BINARY_DIR}/project.ts
#    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
#    )

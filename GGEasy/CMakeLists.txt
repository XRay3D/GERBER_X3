#/*******************************************************************************
#*   *
#* Author : Damir Bakiev  *
#* Version : na  *
#* Date : 14 January 2021  *
#* Website : na  *
#* Copyright : Damir Bakiev 2016-2021  *
#*   *
#* License:   *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt  *
#*   *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.5)

set(PROJECT_NAME "GGEasy")

project(${PROJECT_NAME} LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

#set(CMAKE_CXX_STANDARD 17)
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


#if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
#if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(SUFFIX "_clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(SUFFIX "_gnu")
    if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
        set(SUFFIX "${SUFFIX}_x64")
    else()
        set(SUFFIX "${SUFFIX}_x32")
    endif()
    include_directories (
        /usr/lib/x86_64-linux-gnu/
        )
    link_directories(
        /usr/lib/x86_64-linux-gnu/
        )
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(SUFFIX "_intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SUFFIX "_msvc")
    if(CMAKE_CL_64)
        set(SUFFIX "${SUFFIX}_x64")
    else()
        set(SUFFIX "${SUFFIX}_x32")
    endif()
    include_directories (
#        C:/dev/CGAL-5.1.2/auxiliary/gmp/include
#        C:/dev/CGAL-5.1.2/include
        C:/local/boost_1_76_0_x64
        )
    link_directories(
#        C:/dev/CGAL-5.1.2/auxiliary/gmp/lib
        )
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
add_definitions(
    -DBUILD_DATE="${DT}"
    -D_USE_CGAL_
    -D_USE_BOOST_
    )

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

include_directories (
    ${INCLUDE_DIRECTORIES}
    ../shape_plugins/shape
    ../static_libs/clipper
    ../static_libs/common
    ../static_libs/filetree
    ../static_libs/gerber
    ../static_libs/gi
    ../static_libs/graphicsview
    ../static_libs/thermal
    ../static_libs/tooldatabase
#    C:/dev/CGAL-5.1.2/auxiliary/gmp/include
#    C:/dev/CGAL-5.1.2/include
#    C:/dev/boost_1_75_0
    )

link_directories(
    ${LINK_DIRECTORIES}
    ../libs${SUFFIX}
#    C:/dev/CGAL-5.1.2/auxiliary/gmp/lib
    )

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
    style.cpp
    )

set(HEADERS
    ${HEADERS}
    aboutform.h
    mainwindow.h
    messageoutput.h
    openingdialog.h
    plugindialog.h
    point.h
    recent.h
    settingsdialog.h
    splashscreen.h
    style.h
    version.h
    )

set(UI
    ${UI}
    aboutform.ui
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
    ${MAGIC_GET}
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

#if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
#    target_link_libraries(${PROJECT_NAME} PRIVATE
#        gmp#CGAL
#        mpfr#CGAL
#        )
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
#    target_link_libraries(${PROJECT_NAME} PRIVATE
#        libgmp-10#CGAL
#        libmpfr-4#CGAL
#        )
#endif()

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "${PROJECT_NAME}${SUFFIX}")

if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set_target_properties(${PROJECT_NAME} PROPERTIES
        LINK_FLAGS_DEBUG "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELWITHDEBINFO "/SUBSYSTEM:CONSOLE"
        LINK_FLAGS_RELEASE "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        LINK_FLAGS_MINSIZEREL "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup"
        )
endif()

############################
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Concurrent> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Core> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Gui> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::OpenGL> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::PrintSupport> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::Widgets> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#    )
#if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
#    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
#        COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:Qt${QT_VERSION_MAJOR}::OpenGLWidgets> $<TARGET_FILE_DIR:${PROJECT_NAME}>
#        )
#endif()


#include(../PVS-Studio.cmake)
#pvs_studio_add_target(TARGET example1.analyze ALL
#                      OUTPUT FORMAT errorfile
#                      ANALYZE ${PROJECT_NAME}
#                      MODE GA:1,2 OP
#                      LOG target.err)


#if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
# set(SUFFIX "_clang")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
# set(SUFFIX "_gnu")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
# set(SUFFIX "_intel")
#elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
# set(SUFFIX "_msvc")
#endif()

##if("${CMAKE_GENERATOR}" MATCHES "(Win64|IA64)")
##if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
#if(CMAKE_CL_64)
# set(SUFFIX "${SUFFIX}_x64")
#else()
# set(SUFFIX "${SUFFIX}_x32")
#endif()
#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
# "C:/Qt/5.12.10/mingw73_64/plugins/platforms"
# "$<TARGET_FILE_DIR::${PROJECT_NAME}>/platforms"
# )

#add_custom_command (
# OUTPUT ${TS_FILES}
# COMMAND lupdate -silent -I ./includes ./sources -ts ./project.ts ${CMAKE_CURRENT_BINARY_DIR}/project.ts
# WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# )

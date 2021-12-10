#/*******************************************************************************
#*   *
#* Author : Damir Bakiev  *
#* Version : na  *
#* Date : 11 November 2021 *
#* Website : na  *
#* Copyright : Damir Bakiev 2016-2021  *
#*   *
#* License:   *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt  *
#*   *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.5)

#set(PROJECT_NAME "GGEasy")

#project(${PROJECT_NAME} LANGUAGES CXX)
project(GGEasy)



set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(PREBUILT_MODULE_PATH ${CMAKE_BINARY_DIR}/modules)

#function(add_module name)
#    file(MAKE_DIRECTORY ${PREBUILT_MODULE_PATH})
#    add_custom_target(${name}.pcm
#            COMMAND
#                ${CMAKE_CXX_COMPILER}
#                -std=c++20
#                -stdlib=libc++
#                -fmodules
#                -c
#                ${CMAKE_CURRENT_SOURCE_DIR}/${ARGN}
#                -Xclang -emit-module-interface
#                -o ${PREBUILT_MODULE_PATH}/${name}.pcm

#            )
#endfunction()

#add_compile_options(-fmodules)
#add_compile_options(-stdlib=libc++)
#add_compile_options(-fbuiltin-module-map)
#add_compile_options(-fimplicit-module-maps)
#add_compile_options(-fprebuilt-module-path=${PREBUILT_MODULE_PATH})

#add_module(module module.h)
##add_dependencies(${PROJECT_NAME} module.pcm)
##add_library(${PROJECT_NAME} module.pcm)

#add_compile_options(-fmodules)
#add_compile_options(-stdlib=libc++)
#add_compile_options(-fbuiltin-module-map)
#add_compile_options(-fimplicit-module-maps)
#add_compile_options(-fprebuilt-module-path=${PREBUILT_MODULE_PATH})


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

if(CMAKE_CL_64)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin_x64")
else()
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin_x32")
endif()

include (${PROJECT_SOURCE_DIR}/../SUFFIX.cmake)

string(TIMESTAMP DT "%d.%m.%Y - %H:%M:%S")# [<format string>] [UTC])
add_definitions(
    -DBUILD_DATE="${DT}"
    #    -D_USE_CGAL_
    -D_USE_BOOST_
    )

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_GX3_DIR}/bin)
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_GX3_DIR}/bin)

find_package(Boost)
if (Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIR})
    message(${Boost_INCLUDE_DIR})
    add_definitions( "-D_USE_BOOST_" )
else()
    message("Boost not found!!!" FATAL_ERROR)
endif()

find_package(QT NAMES Qt6 Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS
    Concurrent
    Core
    Gui
    LinguistTools
    OpenGL
    PrintSupport
    Widgets
    REQUIRED
    )

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


set(TARGETS
    ${PROJECT_NAME}
    )

foreach(TNAME ${TARGETS})
    target_compile_features(${TNAME} PUBLIC  cxx_std_20)
    #  target_include_directories(${TNAME} PUBLIC  ${CMAKE_CURRENT_BINARY_DIR})
    #  target_include_directories(${TNAME} PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

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

#add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD COMMAND ${CMAKE_COMMAND} -E copy_directory
# "C:/Qt/5.12.10/mingw73_64/plugins/platforms"
# "$<TARGET_FILE_DIR::${PROJECT_NAME}>/platforms"
# )

#add_custom_command (
# OUTPUT ${TS_FILES}
# COMMAND lupdate -silent -I ./includes ./sources -ts ./project.ts ${CMAKE_CURRENT_BINARY_DIR}/project.ts
# WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
# )

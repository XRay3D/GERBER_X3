#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  11 November 2021                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.16)

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

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
    set(SUFFIX "${SUFFIX}_x64")
    set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin_x64")
else()
    set(SUFFIX "${SUFFIX}_x32")
    set(OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../bin_x32")
endif()

if (CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(SUFFIX "_clang")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(SUFFIX "_gnu")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "Intel")
    set(SUFFIX "_intel")
elseif (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
    set(SUFFIX "_msvc")
endif()

include (${PROJECT_SOURCE_DIR}/../SUFFIX.cmake)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

string(TIMESTAMP DT "%d.%m.%Y - %H:%M:%S")# [<format string>] [UTC])
add_definitions(-DBUILD_DATE="${DT}")

find_package(Boost)
if(Boost_FOUND)
    include_directories(${INCLUDE_DIRECTORIES} ${Boost_INCLUDE_DIRS})
    add_definitions(-D_USE_BOOST_)
endif()

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
    #    Concurrent
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
    forms/drillform/drillform.cpp
    forms/drillform/drillmodel.cpp
    forms/formsutil/errordialog.cpp
    forms/formsutil/formsutil.cpp
    forms/gcodepropertiesform.cpp
    forms/hatchingform.cpp
    forms/pocketoffsetform.cpp
    forms/pocketrasterform.cpp
    forms/profileform.cpp
    forms/toolname.cpp
    forms/voronoiform.cpp
    gcode/gccreator.cpp
    gcode/gcdrillitem.cpp
    gcode/gcfile.cpp
    gcode/gch.cpp
    gcode/gchatching.cpp
    gcode/gcnode.cpp
    gcode/gcplugin.cpp
    gcode/gcpocketoffset.cpp
    gcode/gcpocketraster.cpp
    gcode/gcprofile.cpp
    gcode/gcthermal.cpp
    gcode/gcutils.cpp
    gcode/voroni/gcvoronoi.cpp
    gcode/voroni/gcvoronoiboost.cpp
    gcode/voroni/gcvoronoicgal.cpp
    gcode/voroni/gcvoronoijc.cpp
    gcode/voroni/jc_voronoi.cpp
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
    forms/drillform/drillform.h
    forms/drillform/drillmodel.h
    forms/formsutil/errordialog.h
    forms/formsutil/formsutil.h
    forms/gcodepropertiesform.h
    forms/hatchingform.h
    forms/pocketoffsetform.h
    forms/pocketrasterform.h
    forms/profileform.h
    forms/toolname.h
    forms/voronoiform.h
    gcode/gccreator.h
    gcode/gcdrillitem.h
    gcode/gcfile.h
    gcode/gch.h
    gcode/gchatching.h
    gcode/gcnode.h
    gcode/gcode.h
    gcode/gcplugin.h
    gcode/gcpocketoffset.h
    gcode/gcpocketraster.h
    gcode/gcprofile.h
    gcode/gcthermal.h
    gcode/gctypes.h
    gcode/gcutils.h
    gcode/voroni/gcvoronoi.h
    gcode/voroni/gcvoronoiboost.h
    gcode/voroni/gcvoronoicgal.h
    gcode/voroni/gcvoronoijc.h
    gcode/voroni/jc_voronoi.h
    gcode/voroni/voronoi_visual_utils.h
    interfaces/file.h
    interfaces/pluginfile.h
    interfaces/plugintypes.h
    interfaces/shapepluginin.h
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
    forms/drillform/drillform.ui
    forms/gcodepropertiesform.ui
    forms/hatchingform.ui
    forms/pocketoffsetform.ui
    forms/pocketrasterform.ui
    forms/profileform.ui
    forms/voronoiform.ui
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
    common
    filetree
    gi
    graphicsview
    shape
    thermal
    tooldatabase
    #    Qt${QT_VERSION_MAJOR}::Concurrent
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::OpenGL
    Qt${QT_VERSION_MAJOR}::PrintSupport
    Qt${QT_VERSION_MAJOR}::Widgets
    )

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE setupapi)
endif()

# windeployqt
install(TARGETS ${PROJECT_NAME} DESTINATION ${OUTPUT_DIRECTORY})
set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
get_target_property(_QMAKE_EXECUTABLE Qt5::qmake IMPORTED_LOCATION)
get_filename_component(_QT_BIN_DIR "${_QMAKE_EXECUTABLE}" DIRECTORY)
find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_QT_BIN_DIR}")
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_QT_BIN_DIR}" "${WINDEPLOYQT_EXECUTABLE}"
            --verbose 0
            --no-compiler-runtime
            \"$<TARGET_FILE:${PROJECT_NAME}>\"
    COMMENT "Deploying Qt..."
)
#execute_process(
#    COMMAND "${CMAKE_COMMAND}" -E
#        env PATH="${_qt_bin_dir}" "${WINDEPLOYQT_EXECUTABLE}"
#            --dry-run
#            --no-compiler-runtime
#            --list mapping
#            \"$<TARGET_FILE:myapp>\"
#    OUTPUT_VARIABLE _output
#    OUTPUT_STRIP_TRAILING_WHITESPACE
#)

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
#if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
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

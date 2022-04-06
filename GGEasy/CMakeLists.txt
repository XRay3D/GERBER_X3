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

include (${CMAKE_SOURCE_DIR}/SUFFIX.cmake)

#set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
#set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
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

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations ${TS_FILES})
    qt6_add_resources(RESOURCES res/resources.qrc)
else()
    qt5_create_translation(QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/translations ${TS_FILES})
    qt5_add_resources(RESOURCES res/resources.qrc)
endif()


# WINDOWS APP ICON
set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_SOURCE_DIR}/myapp.rc")

file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

add_executable(${PROJECT_NAME}
    ${TS_FILES}
    ${SOURCES}
    ${HEADERS}
    ${UIS}
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
#    thermal
    tooldatabase
    gcode
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

install(TARGETS ${PROJECT_NAME} DESTINATION ${OUTPUT_DIRECTORY})

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE setupapi)
    # windeployqt
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    get_target_property(_QMAKE_EXECUTABLE Qt${QT_VERSION_MAJOR}::qmake IMPORTED_LOCATION)
    get_filename_component(_QT_BIN_DIR "${_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_QT_BIN_DIR}")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E
        env PATH="${_QT_BIN_DIR}" "${WINDEPLOYQT_EXECUTABLE}"
        --verbose 1
        --no-compiler-runtime
        \"$<TARGET_FILE:${PROJECT_NAME}>\"
        COMMENT "Deploying Qt..."
        )
elseif (UNIX)#LINUX
    #    cqtdeployer -bin myApp -qmake path/to/Qt/5.x.x/build/bin/qmake -qmlDir path/to/my/qml/files/dir qif
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

set(TARGETS ${PROJECT_NAME})

foreach(TNAME ${TARGETS})
    target_compile_features(${TNAME} PUBLIC  cxx_std_20)
    #  target_include_directories(${TNAME} PUBLIC  ${CMAKE_CURRENT_BINARY_DIR})
    #  target_include_directories(${TNAME} PUBLIC  ${CMAKE_CURRENT_SOURCE_DIR})
endforeach()

# COPY TRANSLATIONS
file(GLOB_RECURSE QM_FILES ${CMAKE_CURRENT_SOURCE_DIR}/*.qm)
foreach(QM_FILE ${QM_FILES})
    get_filename_component(QM_NAME ${QM_FILE} NAME)
    configure_file(${QM_FILE} "${OUTPUT_DIRECTORY}/translations/${QM_NAME}" COPYONLY)
#    file(COPY_FILE ${QM_FILE} "${OUTPUT_DIRECTORY}/translations/${QM_NAME}")
endforeach()

#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  March 25, 2023                                                  *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2023                                          *
#* License   :                                                                  *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#********************************************************************************/
cmake_minimum_required(VERSION 3.20)


project(common LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Gui Widgets LinguistTools)

file(GLOB_RECURSE HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_library(common STATIC
    ${HEADERS}
    ${SOURCES}
)

target_link_libraries(common PRIVATE
    filetree
    shape
    gi
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)


#add_module_library(QT qt.cpp)

#target_compile_definitions(common PRIVATE COMMON_LIBRARY)

set(TS_FILES
    ${PROJECT_NAME}_ru.ts
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES ${PROJECT_SOURCES} OPTIONS -silent OPTIONS -source-language en_US)
    qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES OPTIONS -silent)
else()
    qt5_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
    qt5_add_translation(${QM_FILES} ${TS_FILES})
endif()

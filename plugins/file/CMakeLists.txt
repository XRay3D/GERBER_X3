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


project(file_plugins)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES #[[Qt6]] Qt5 COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets LinguistTools REQUIRED)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

set(PROJECT_SOURCES
    ${HEADERS}
    ${SOURCES}
    ${UIS}
)

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

#target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

link_directories(
    ${CMAKE_SOURCE_DIR}/libs${PATHS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

set(TS_FILES
    ${PROJECT_NAME}_ru.ts
    ${PROJECT_NAME}_en.ts
)

set_source_files_properties(${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6) # Android
    qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
    qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES ${PROJECT_SOURCES})
    # qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES)
else()
    qt5_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
    qt5_add_translation(${QM_FILES} ${TS_FILES})
endif()


add_subdirectory(gerber)
add_subdirectory(dxf)
add_subdirectory(excellon)
#add_subdirectory(TopoR)

#add_subdirectory(file)
#add_subdirectory(hpgl)



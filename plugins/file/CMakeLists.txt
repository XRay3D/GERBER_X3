#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  ХХ ХХХ 2026
 Website   :  na
 Copyright :  Damir Bakiev 2016-2026
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 4.00)

project(file_plugins)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Widgets #[[tbb]] common)

set(TS_FILES ${PROJECT_NAME}_ru.ts ${PROJECT_NAME}_en.ts)

set_source_files_properties(
  ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES ${PROJECT_SOURCES})
# qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE
# QM_FILES)

foreach(SUBDIRECTORY ${FILE_PLUGINS})
  add_subdirectory(${SUBDIRECTORY})
endforeach()

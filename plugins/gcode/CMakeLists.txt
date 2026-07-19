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

project(gcode)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
file(GLOB UIS *.ui)
file(GLOB JS_SCRIPTS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.js")

qt_add_resources(QRC_SOURCES gc_scripts.qrc)

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} ${UIS} ${QRC_SOURCES})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC geometry ctre Qt6::Qml)

# Copy JS scripts to output directory so users can view and edit them
add_custom_target(
  ${PROJECT_NAME}_scripts ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SCRIPTS_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${JS_SCRIPTS} "${SCRIPTS_DIR}/"
  DEPENDS ${JS_SCRIPTS}
  COMMENT "Copying GCode scripts to ${SCRIPTS_DIR}/")
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_scripts)

set(TS_FILES ${PROJECT_NAME}_ru.ts ${PROJECT_NAME}_en.ts)

set_source_files_properties(
  ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES ${PROJECT_SOURCES})
# qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE
# QM_FILES)

foreach(SUBDIRECTORY ${GCODE_PLUGINS})
  add_subdirectory(${SUBDIRECTORY})
endforeach()

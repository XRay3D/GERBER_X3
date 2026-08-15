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
cmake_minimum_required(VERSION 4.0)

project(shscript LANGUAGES CXX)

set(PLUG_NAME "Script Shape")
set(PLUG_INFO "Shape built by a JavaScript script")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "1")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/description.json NEWLINE_STYLE UNIX)

file(GLOB HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
file(GLOB JSONS CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/*.json)
file(GLOB JS_SCRIPTS CONFIGURE_DEPENDS scripts/*.js)

qt_add_resources(QRC_SOURCES shscripts.qrc)

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} ${JSONS} ${QRC_SOURCES} ${JS_SCRIPTS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets Qt6::Qml)

target_link_ggcore(${PROJECT_NAME})

# Скрипты рядом с приложением -- пользователь их правит, плагин их перечитывает.
add_custom_target(
  ${PROJECT_NAME}_scripts ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SCRIPTS_DIR}/shapes"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${JS_SCRIPTS} "${SCRIPTS_DIR}/shapes/"
  DEPENDS ${JS_SCRIPTS}
  COMMENT "Copying shape scripts to ${SCRIPTS_DIR}/shapes/")
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_scripts)

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES})

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

project(gcode)

find_package(
  Qt6
  COMPONENTS OpenGL OpenGLWidgets
  REQUIRED)

file(GLOB HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
file(GLOB UIS CONFIGURE_DEPENDS *.ui)
file(GLOB JS_SCRIPTS CONFIGURE_DEPENDS
     "${CMAKE_CURRENT_SOURCE_DIR}/scripts/*.js")

qt_add_resources(QRC_SOURCES gc_scripts.qrc)

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} ${UIS} ${QRC_SOURCES})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

# common и file_plugins -- явно: раньше их каталоги прилетали сюда через
# кольцо geometry -> common -> filetree -> file_plugins, а Geo это кольцо
# не замыкает (он линкует только Qt6::Gui).
target_link_libraries(
  ${PROJECT_NAME} PUBLIC geometry common file_plugins ctre Qt6::Qml
                         Qt6::OpenGL Qt6::OpenGLWidgets)

# Copy JS scripts to output directory so users can view and edit them
add_custom_target(
  ${PROJECT_NAME}_scripts ALL
  COMMAND ${CMAKE_COMMAND} -E make_directory "${SCRIPTS_DIR}"
  COMMAND ${CMAKE_COMMAND} -E copy_if_different ${JS_SCRIPTS} "${SCRIPTS_DIR}/"
  DEPENDS ${JS_SCRIPTS}
  COMMENT "Copying GCode scripts to ${SCRIPTS_DIR}/")
add_dependencies(${PROJECT_NAME} ${PROJECT_NAME}_scripts)

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UIS})

foreach(SUBDIRECTORY ${GCODE_PLUGINS})
  add_subdirectory(${SUBDIRECTORY})
  if(TARGET gc_${SUBDIRECTORY})
    target_link_libraries(gc_${SUBDIRECTORY} PUBLIC gcode file_plugins)
  else()
    target_link_libraries(gc${SUBDIRECTORY} PUBLIC gcode file_plugins)
  endif()
endforeach()

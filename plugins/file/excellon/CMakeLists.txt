#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  03 October 2024
 Website   :  na
 Copyright :  Damir Bakiev 2016-2024
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 3.20)

project(file_excellon LANGUAGES CXX)

set(PLUG_NAME "Excellon")
set(PLUG_INFO "Opening drill files like Excellon")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "3")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_SOURCE_DIR}/description.json NEWLINE_STYLE UNIX)

file(GLOB HEADERS *.h)
file(GLOB JSONS *.json)
file(GLOB SOURCES *.cpp)
file(GLOB UIS *.ui)

include_directories(${CMAKE_STATIC_LIBS_DIR})

set(PROJECT_SOURCES ${HEADERS} ${JSONS} ${SOURCES} ${SOURCE} ${UIS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

# target_compile_options( ${PROJECT_NAME} PRIVATE -fmodules-ts
# -fmodule-mapper=${CMAKE_BINARY_DIR}/CMakeFiles/ctre.dir/third_party/ctre/ctre.cppm.o.modmap
# )

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE clipper
          common
          # ctre::ctre
          file_plugins
          filetree
          gcode
          gi
          graphicsview
          tooldatabase
          Qt6::Widgets)

set(TS_FILES ${PROJECT_NAME}_ru.ts ${PROJECT_NAME}_en.ts)

set_source_files_properties(
  ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

if(6 GREATER_EQUAL 6) # Android
  qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
  qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES
                 ${PROJECT_SOURCES})
  # qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES}
  # QM_FILES_OUTPUT_VARIABLE QM_FILES)
else()
  qt5_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
  qt5_add_translation(${QM_FILES} ${TS_FILES})
endif()

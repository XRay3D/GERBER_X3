set(DIR_NAME
    "Qt6.${QT_VERSION_MINOR}_${CMAKE_CXX_COMPILER_ID}_${CMAKE_BUILD_TYPE}")

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(DIR_NAME "${DIR_NAME}_x64")
else()
  set(DIR_NAME "${DIR_NAME}_x32")
endif()

set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${DIR_NAME}/bin")
set(PLUGINS_DIR "${OUTPUT_DIRECTORY}/plugins")
set(SCRIPTS_DIR "${OUTPUT_DIRECTORY}/scripts")
set(STATIC_LIBS_DIR "${CMAKE_SOURCE_DIR}/bin/${DIR_NAME}/static_libs")

message(STATUS "${PROJECT_NAME}")
message(STATUS "CMAKE_SOURCE_DIR >> ${CMAKE_SOURCE_DIR}")
message(STATUS "STATIC_LIBS_DIR  >> ${STATIC_LIBS_DIR}")
message(STATUS "PLUGINS_DIR      >> ${PLUGINS_DIR}")
message(STATUS "DIR_NAME         >> ${DIR_NAME}")

link_directories(${STATIC_LIBS_DIR})

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_DIR})

# Helper: set up lupdate + lrelease for a target. Usage:
# target_setup_translations(<target> [source1 source2 ...]) TS files
# <target>_ru.ts and <target>_en.ts must exist in CMAKE_CURRENT_SOURCE_DIR.
function(target_setup_translations TARGET)
  set(TS_FILES ${TARGET}_ru.ts ${TARGET}_en.ts)
  set_source_files_properties(
    ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")
  qt_add_lupdate(${TARGET} TS_FILES ${TS_FILES} SOURCES ${ARGN})
  qt_add_lrelease(${TARGET} TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE
                  QM_FILES)
endfunction()

# Shared precompiled header (Qt + common STL) compiled once for the whole
# project. Every other target reuses this binary via REUSE_FROM instead of
# reparsing <QtCore>/<QtGui>/<QtWidgets> for itself.
add_library(pch_qt OBJECT ${CMAKE_SOURCE_DIR}/pch_stub.cpp)
target_link_libraries(pch_qt PRIVATE Qt6::Widgets Qt6::Qml)
target_precompile_headers(pch_qt PRIVATE ${CMAKE_SOURCE_DIR}/qt_pch.h)

# Helper: opt a target into the shared PCH above.
# Usage: target_enable_pch(<target>)
function(target_enable_pch TARGET)
    target_precompile_headers(${TARGET} REUSE_FROM pch_qt)
endfunction()

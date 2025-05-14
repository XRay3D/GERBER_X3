cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

set(DIR_NAME
    "Qt6.${QT_VERSION_MINOR}_${CMAKE_CXX_COMPILER_ID}_${CMAKE_BUILD_TYPE}")

if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
  set(DIR_NAME "${DIR_NAME}_x64")
else()
  set(DIR_NAME "${DIR_NAME}_x32")
endif()

set(OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin/${DIR_NAME}/bin")

set(PLUGINS_DIR "${OUTPUT_DIRECTORY}/plugins")

set(STATIC_LIBS_DIR "${CMAKE_SOURCE_DIR}/bin/${DIR_NAME}/static_libs")

message(${PROJECT_NAME})
message("CMAKE_SOURCE_DIR >> ${CMAKE_SOURCE_DIR}")
message("STATIC_LIBS_DIR  >> ${STATIC_LIBS_DIR}")
message("PLUGINS_DIR      >> ${PLUGINS_DIR}")
message("DIR_NAME         >> ${DIR_NAME}")

link_directories(STATIC_LIBS_DIR)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${STATIC_LIBS_DIR})
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_MODULE_PATH ${CMAKE_ARCHIVE_OUTPUT_DIRECTORY})

# option(UPDATE_TRANSLATION_SOURCES "Updates *.ts files" OFF) if
# (UPDATE_TRANSLATION_SOURCES) qt5_create_translation(TRANSLATION_QM ${sources}
# ${TRANSLATION_FILES}) else(UPDATE_TRANSLATION_SOURCES)
# qt5_add_translation(TRANSLATION_QM ${TRANSLATION_FILES})
# endif(UPDATE_TRANSLATION_SOURCES) add_custom_target(translations DEPENDS
# ${TRANSLATION_QM})

function(add_translation TARGET)
  if(6 GREATER_EQUAL 6)
    set(TRANSLATIONS ${TARGET}_ru.ts ${TARGET}_en.ts)
    # qt_add_translations(TARGETS ${TARGET} TS_FILES ${TARGET}_ru.ts TS_FILES
    # ${TARGET}_en.ts)

    qt_create_translation(QM_FILES ${TRANSLATIONS})

    # qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})
    # qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
    # qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES
    # ${PROJECT_SOURCES}) qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES}
    # QM_FILES_OUTPUT_VARIABLE QM_FILES)
  else()
    # add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
    qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
  endif()
  message(==> ${QM_FILES})
  add_custom_target(${TARGET}_translations DEPENDS ${QM_FILES})
  add_dependencies(${TARGET} ${TARGET}_translations)
  set_source_files_properties(
    ${QM_FILES} PROPERTIES OUTPUT_LOCATION ${OUTPUT_DIRECTORY}/translations)

endfunction()

# set(QT_TRANSLATION_DIR "${Qt6_DIR}/../../../translations") file(GLOB
# QT_TRANSLATIONS ${QT_TRANSLATION_DIR}/*.qm)
# qt_add_resources(ex-texteditor-cmake "qt-translations" PREFIX "/translations"
# BASE ${QT_TRANSLATION_DIR} FILES ${QT_TRANSLATIONS})

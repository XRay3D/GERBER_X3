#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  ХХ ХХХ 2025
 Website   :  na
 Copyright :  Damir Bakiev 2016-2025
 License   :
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 3.30.0 FATAL_ERROR)

project(
  GGEasy
  VERSION 0.9.0.0
  LANGUAGES C CXX)

# if(ANDROID) set(ANDROID_PACKAGE_SOURCE_DIR
# "${CMAKE_CURRENT_SOURCE_DIR}/android") if(ANDROID_ABI STREQUAL "armeabi-v7a")
# set(ANDROID_EXTRA_LIBS ${CMAKE_CURRENT_SOURCE_DIR} / path / to / libcrypto.so
# ${CMAKE_CURRENT_SOURCE_DIR } / path / to / libssl.so) endif() endif()

# set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
# set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

string(TIMESTAMP DT "%d.%m.%Y - %H:%M:%S") # [<format string>] [UTC])

add_definitions(-DBUILD_DATE="${DT}" -DGIT_REF_NAME="${GIT_REF_NAME}"
                -DGIT_SHA="${GIT_SHA}")

find_package(Qt6 REQUIRED COMPONENTS Core Gui LinguistTools OpenGL PrintSupport
                                     Widgets)

set(APP_ICON_RESOURCE_WINDOWS "myapp.rc") # WINDOWS APP ICON

file(GLOB HEADERS *.h)
file(GLOB QRC_FILES res/*.qrc)
file(GLOB SOURCES *.cpp)
file(GLOB UIS *.ui)

set(TS_FILES ${PROJECT_NAME}_en.ts ${PROJECT_NAME}_ru.ts)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  find_package(
    Qt6
    COMPONENTS OpenGLWidgets
    REQUIRED)
endif()

# if(${QT_VERSION_MAJOR} GREATER_EQUAL 6) qt6_add_resources(RESOURCES
# ${QRC_FILES}) else() set(Qt5Core_RCC_EXECUTABLE /usr/share/qt5/bin/rcc)
# qt5_add_resources(RESOURCES ${QRC_FILES}) endif()

set(PROJECT_SOURCES ${APP_ICON_RESOURCE_WINDOWS} ${HEADERS} ${QRC_FILES}
                    ${RESOURCES} ${SOURCES} ${UIS})

set_source_files_properties(
  ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})
  # qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
  # qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES
  # ${PROJECT_SOURCES}) qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES}
  # QM_FILES_OUTPUT_VARIABLE QM_FILES)
else()
  add_executable(${PROJECT_NAME} ${PROJECT_SOURCES})
  # qt5_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
  # qt5_add_translation(${QM_FILES} ${TS_FILES})
endif()

# add_translation(${PROJECT_NAME})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE clipper
          common
          filetree
          gcode
          gi
          graphicsview
          shape
          tooldatabase
          Qt6::PrintSupport
          Qt6::Widgets)

if(EXISTS XrStyle)
  target_link_libraries(${PROJECT_NAME} PRIVATE XrStyle)
endif()

#[[
if(CMAKE_CXX_COMPILER_VERSION VERSION_GREATER_EQUAL 14)
  target_link_libraries(${PROJECT_NAME} PRIVATE stdc++exp)
else()
  target_link_libraries(${PROJECT_NAME} PRIVATE stdc++_libbacktrace)
endif()
]]

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::OpenGLWidgets)
endif()

set_target_properties(
  ${PROJECT_NAME}
  PROPERTIES MACOSX_BUNDLE_GUI_IDENTIFIER xr.ggeasy.com
             MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
             MACOSX_BUNDLE_SHORT_VERSION_STRING
             ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
             MACOSX_BUNDLE TRUE
             WIN32_EXECUTABLE TRUE)

install(
  TARGETS ${PROJECT_NAME}
  BUNDLE DESTINATION .
  LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
  qt_finalize_executable(${PROJECT_NAME})
endif()

if(WIN32_)
  target_link_libraries(${PROJECT_NAME} PRIVATE setupapi)
  # windeployqt
  set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
  get_target_property(_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
  get_filename_component(_QT_BIN_DIR "${_QMAKE_EXECUTABLE}" DIRECTORY)
  find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_QT_BIN_DIR}")
  add_custom_command(
    TARGET ${PROJECT_NAME}
    POST_BUILD
    COMMAND
      "${CMAKE_COMMAND}" -E env PATH="${_QT_BIN_DIR}"
      "${WINDEPLOYQT_EXECUTABLE}" --verbose 1 --no-compiler-runtime
      \"$<TARGET_FILE:${PROJECT_NAME}>\"
    COMMENT "Deploying Qt...")
elseif(UNIX) # LINUX
  find_program(CQT_EXE cqtdeployer)

  if(NOT EXISTS ${CQT_EXE})
    message(
      "the cqtdeployer not exits please install the cqtdeployer and run cmake again!"
    )
    return()
  endif()

  add_custom_target(
    deploy
    COMMAND
      cqtdeployer clear -bin "${OUTPUT_DIRECTORY}/${PROJECT_NAME}" -extraData
      "${OUTPUT_DIRECTORY}/plugins" -extraDataOut /bin -qmake
      "${QT_QMAKE_EXECUTABLE}" -libDir "${PROJECT_SOURCE_DIR}" -qmlDir
      "${PROJECT_SOURCE_DIR}" -recursiveDepth 5 -targetDir
      "${OUTPUT_DIRECTORY}/../cqt-deploy"
    COMMENT "Deploy app with cqtdeployer"
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

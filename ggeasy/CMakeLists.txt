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

project(
    GGEasy
    VERSION 0.9.0.0
    LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

string(TIMESTAMP DT "%d.%m.%Y - %H:%M:%S")

add_compile_definitions(
    BUILD_DATE="${DT}"
    GIT_REF_NAME="${GIT_REF_NAME}"
    GIT_SHA="${GIT_SHA}")

set(SOFTDEVELOPER X-Ray)
# Write to build dir so the source tree stays clean
configure_file(
    versioninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc
    NEWLINE_STYLE WIN32)

find_package(
    Qt6 REQUIRED
    COMPONENTS Core
               Gui
               LinguistTools
               OpenGL
               OpenGLWidgets
               PrintSupport
               Widgets)

set(APP_ICON_RESOURCE_WINDOWS "${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc")

file(GLOB HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB QRC_FILES CONFIGURE_DEPENDS res/*.qrc)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
file(GLOB UIS CONFIGURE_DEPENDS *.ui)

set(TS_FILES
    translations/${PROJECT_NAME}_ru.ts
    translations/${PROJECT_NAME}_en.ts)

set(PROJECT_SOURCES ${APP_ICON_RESOURCE_WINDOWS} ${HEADERS} ${QRC_FILES}
                    ${RESOURCES} ${SOURCES} ${UIS})

set_source_files_properties(${TS_FILES} PROPERTIES
    OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})

qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES ${HEADERS} ${SOURCES} ${UIS})
qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE geometry
            common
            filetree
            gcode
            gi
            graphicsview
            shape
            tooldatabase
            stdc++exp
            Qt6::PrintSupport
            Qt6::Widgets
            Qt6::OpenGLWidgets)

if(TARGET XrStyle)
    target_link_libraries(${PROJECT_NAME} PRIVATE XrStyle)
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

qt_finalize_executable(${PROJECT_NAME})

if(WIN32) # windeployqt
    target_link_libraries(${PROJECT_NAME} PRIVATE setupapi)
    set(CMAKE_INSTALL_UCRT_LIBRARIES TRUE)
    get_target_property(_QMAKE_EXECUTABLE Qt6::qmake IMPORTED_LOCATION)
    get_filename_component(_QT_BIN_DIR "${_QMAKE_EXECUTABLE}" DIRECTORY)
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_QT_BIN_DIR}")
    if(EXISTS "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/Qt6Core.dll")
        return()
    endif()
    add_custom_command(
        TARGET ${PROJECT_NAME}
        POST_BUILD
        COMMAND
            "${CMAKE_COMMAND}" -E env PATH="${_QT_BIN_DIR}"
            "${WINDEPLOYQT_EXECUTABLE}" --verbose 1
            \"$<TARGET_FILE:${PROJECT_NAME}>\"
        COMMENT "Deploying Qt...")
elseif(UNIX) # LINUX
    find_program(CQT_EXE cqtdeployer)
    if(NOT CQT_EXE)
        message(STATUS
            "cqtdeployer not found — install it and re-run cmake for the deploy target.")
        return()
    endif()
    add_custom_target(
        deploy
        COMMAND
            cqtdeployer clear
            -bin "${OUTPUT_DIRECTORY}/${PROJECT_NAME}"
            -extraData "${OUTPUT_DIRECTORY}/plugins"
            -extraDataOut /bin
            -qmake "${QT_QMAKE_EXECUTABLE}"
            -libDir "${PROJECT_SOURCE_DIR}"
            -qmlDir "${PROJECT_SOURCE_DIR}"
            -recursiveDepth 5
            -targetDir "${OUTPUT_DIRECTORY}/../cqt-deploy"
        COMMENT "Deploy app with cqtdeployer"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

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
    VERSION ${GERBER_X3_VERSION}
    LANGUAGES C CXX)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OUTPUT_DIRECTORY})

string(TIMESTAMP DT "%d.%m.%Y - %H:%M:%S")

add_compile_definitions(
    BUILD_DATE="${DT}"
    GIT_REF_NAME="${GIT_REF_NAME}"
    GIT_SHA="${GIT_SHA}")

set(SOFTDEVELOPER X-Ray)
configure_file(version.h.in ${CMAKE_CURRENT_BINARY_DIR}/version.h)
# Write to build dir so the source tree stays clean
configure_file(
    versioninfo.rc.in ${CMAKE_CURRENT_BINARY_DIR}/versioninfo.rc
    NEWLINE_STYLE WIN32)
# Манифест приложения (asInvoker), вшивается ресурсом из versioninfo.rc.
# Лежит рядом с versioninfo.rc, поэтому CMAKE_CURRENT_BINARY_DIR ниже добавлен
# в include-пути таргета: windres ищет файлы ресурсов именно по ним.
configure_file(
    ${PROJECT_NAME}.manifest.in ${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.manifest
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
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/translations/${PROJECT_NAME}_zh.ts")
  list(APPEND TS_FILES translations/${PROJECT_NAME}_zh.ts)
endif()

set(PROJECT_SOURCES ${APP_ICON_RESOURCE_WINDOWS} ${HEADERS} ${QRC_FILES}
                    ${RESOURCES} ${SOURCES} ${UIS})

set_source_files_properties(${TS_FILES} PROPERTIES
    OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

qt_add_executable(${PROJECT_NAME} MANUAL_FINALIZATION ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES ${HEADERS} ${SOURCES} ${UIS})
qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES} QM_FILES_OUTPUT_VARIABLE QM_FILES)

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE ggcore
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
    # Голый бинарь, НЕ обёртка CQtDeployer.sh: обёртка экспортирует QTDIR и
    # QT_PLUGIN_PATH на каталоги самого CQtDeployer, и анализ Qt деплоящегося
    # приложения ломается ("Major version: 0" — в дистрибутив не попадает ни
    # одна Qt-библиотека). Бинарю нужен только LD_LIBRARY_PATH на его lib/ —
    # задаётся в COMMAND ниже.
    find_program(CQT_EXE NAMES cqtdeployer CQtDeployer
                 HINTS "$ENV{HOME}/CQtDeployer/1.6/bin")
    if(NOT CQT_EXE)
        message(STATUS
            "cqtdeployer not found — install it and re-run cmake for the deploy target.")
        return()
    endif()

    # Qt-интроспекция старого CQtDeployer с qmake 6.10 не работает ("Major
    # version: 0"), поэтому каталоги задаются явно: -libDir на выходной каталог
    # (там libggcore.so) и системный lib64, -extraPlugin на платформенные
    # плагины Qt (offscreen/xcb) -- иначе дистрибутив без них не стартует.
    execute_process(
        COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
        OUTPUT_VARIABLE QT_PLUGINS_DIR OUTPUT_STRIP_TRAILING_WHITESPACE)

    # CQtDeployer кладёт в lib/ СИСТЕМНЫЙ libstdc++ независимо от порядка
    # -libDir, а собранное gcc-16 требует GLIBCXX_3.4.35+ -- дистрибутив не
    # стартует. Поверх кладётся рантайм самого компилятора (CXX_RUNTIME_DIR
    # вычислен в корневом CMakeLists).
    set(DEPLOY_FIX_RUNTIME)
    if(CXX_RUNTIME_DIR)
        set(DEPLOY_FIX_RUNTIME
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${CXX_RUNTIME_DIR}/libstdc++.so.6"
                    "${CXX_RUNTIME_DIR}/libgcc_s.so.1"
                    "${OUTPUT_DIRECTORY}/../cqt-deploy/lib/")
    endif()

    add_custom_target(
        deploy
        COMMAND
            ${CMAKE_COMMAND} -E env
            "LD_LIBRARY_PATH=$<PATH:GET_PARENT_PATH,$<PATH:GET_PARENT_PATH,${CQT_EXE}>>/lib"
            ${CQT_EXE} clear
            -bin "${OUTPUT_DIRECTORY}/${PROJECT_NAME}"
            -extraData "${OUTPUT_DIRECTORY}/plugins"
            -extraDataOut /bin
            -qmake "${QT_QMAKE_EXECUTABLE}"
            -libDir "${OUTPUT_DIRECTORY},${CXX_RUNTIME_DIR},/usr/lib64"
            -extraPlugin "${QT_PLUGINS_DIR}/platforms"
            # GL-библиотеки -- драйверные, их таскать нельзя: забандленный
            # libOpenGL падал на _glapi_tls_Current, на целевой машине они
            # берутся из системы.
            -ignore "libOpenGL,libGLX,libGLdispatch,libEGL,libGL,libdrm"
            -qmlDir "${PROJECT_SOURCE_DIR}"
            -recursiveDepth 5
            -targetDir "${OUTPUT_DIRECTORY}/../cqt-deploy"
        ${DEPLOY_FIX_RUNTIME}
        COMMENT "Deploy app with cqtdeployer"
        WORKING_DIRECTORY ${PROJECT_SOURCE_DIR})
endif()

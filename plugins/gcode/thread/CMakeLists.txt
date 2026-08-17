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

project(gcthread LANGUAGES CXX)

set(PLUG_NAME "Thread")
set(PLUG_INFO "Thread")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "0")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/description.json NEWLINE_STYLE UNIX)

file(GLOB HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
file(GLOB UIS CONFIGURE_DEPENDS *.ui)
file(GLOB JSONS CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/*.json)

set(PROJECT_SOURCES ${HEADERS} ${JSONS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

# Ничего из статлиб ядра тут линковать НЕЛЬЗЯ: они целиком лежат в
# libggcore, который прилетает через target_link_ggcore(gcthread) в
# plugins/gcode/CMakeLists.txt. Заголовки gcode приезжают оттуда же --
# ggcore отдаёт их потребителям через $<COMPILE_ONLY:gcode>. Прямой
# `PRIVATE gcode` тянул бы в .so ещё и libgcode.a с транзитивными
# libGeo.a/libcommon.a: на ELF это молча дублирует код, а mingw-ld падает
# на "multiple definition" -- символы уже экспортированы из libggcore.dll.

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UIS})

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
cmake_minimum_required(VERSION 4.0)

project(file_TopoR LANGUAGES CXX)

set(PLUG_NAME "TopoR")
set(PLUG_INFO
    "Opening TopoR files, with support for all kinds of aperture macros and components.")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "1")
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/description.json NEWLINE_STYLE UNIX)

file(GLOB HEADERS CONFIGURE_DEPENDS *.h tables/*.h)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp tables/*.cpp)
file(GLOB UIS CONFIGURE_DEPENDS *.ui)
file(GLOB JSONS CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/*.json)

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} ${JSONS} ${UIS})

set(LXML_USE_MODULES OFF)
add_subdirectory(littlexml)
# lxmlser.hpp зовёт двухаргументный meta::annotations_of(info, type) под
# именем annotations_of_with_type только когда определён GR (иначе -- под тем
# же именем, что и штатный одноаргументный std::meta::annotations_of, и не
# компилируется на GCC 16, где эти две функции разные, не перегрузки).
target_compile_definitions(lxml INTERFACE GR=1)
add_subdirectory(TopoR)
# TopoR/TopoR/CMakeLists.txt (порт схемы TopoR PCB) не знает о littlexml --
# подключаем здесь, а не переписывая чужой файл схемы.
target_link_libraries(TopoR PUBLIC lxml::lxml)
# Схема 1:1 переносит C#-соглашение "Header Header;" (поле совпадает по имени
# со своим типом) -- в GCC это жёсткая ошибка (-Wchanges-meaning), не
# подавляемая ничем, кроме этого флага; переименовывать сотни полей во всех
# файлах схемы вместо этого не стали.
target_compile_options(TopoR PRIVATE -Wno-changes-meaning)

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/tables
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets TopoR lxml::lxml)

# Заголовки схемы TopoR PCB парсятся заново и в этой цели (topor_parser.cpp,
# topor_file.h её включают) -- то же исключение, что у TopoR/CMakeLists.txt.
target_compile_options(${PROJECT_NAME} PRIVATE -Wno-changes-meaning)

target_link_ggcore(${PROJECT_NAME})

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UIS})

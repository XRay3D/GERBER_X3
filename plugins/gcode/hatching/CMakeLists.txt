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

project(gchatching LANGUAGES CXX)

set(PLUG_NAME "Crosshatch")
set(PLUG_INFO "Crosshatch")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "0")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/description.json NEWLINE_STYLE UNIX)

add_library(gc_hatching STATIC hatching.cpp hatching.h)
# Clipper2 (geometry) плагину больше не нужен: карман и гребёнка считаются в
# точном домене, резка змейки -- Geo::clipOpen, маршрут -- gc_pathutils.
target_link_libraries(gc_hatching PUBLIC gcode)

file(GLOB HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
file(GLOB UIS CONFIGURE_DEPENDS *.ui)
file(GLOB JSONS CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/*.json)

# Якорь обязателен: без него «hatching.h» как regex (точка -- любой символ)
# зацепил бы и hatching_form.h через wildcard.
list(FILTER HEADERS EXCLUDE REGEX "/hatching\\.h$")
list(FILTER SOURCES EXCLUDE REGEX "/hatching\\.cpp$")

set(PROJECT_SOURCES ${HEADERS} ${JSONS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE gc_hatching)

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UIS})

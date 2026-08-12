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

project(gcthermal LANGUAGES CXX)

set(PLUG_NAME "Thermal")
set(PLUG_INFO "Thermal")
set(PLUG_AUTHOR "X-Ray aka Bakiev Damir")
set(PLUG_LOCAL_VERSION "0")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/../../description.json.in
               ${CMAKE_CURRENT_BINARY_DIR}/description.json NEWLINE_STYLE UNIX)

add_library(gc_thermal STATIC thermal.cpp thermal.h)
# Clipper2 (geometry) плагину больше не нужен: кольца и рамка считаются в
# точном домене, резка колец -- Geo::clipOpen, маршрут -- gc_pathutils.
target_link_libraries(gc_thermal PUBLIC gcode)

file(GLOB HEADERS CONFIGURE_DEPENDS *.h)
file(GLOB SOURCES CONFIGURE_DEPENDS *.cpp)
file(GLOB UIS CONFIGURE_DEPENDS *.ui)
file(GLOB JSONS CONFIGURE_DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/*.json)

# Якорь обязателен: без него «thermal.h» как regex (точка -- любой символ)
# выел бы и thermal_pch.h.
list(FILTER HEADERS EXCLUDE REGEX "/thermal\\.h$")
list(FILTER SOURCES EXCLUDE REGEX "/thermal\\.cpp$")

set(PROJECT_SOURCES ${HEADERS} ${JSONS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} SHARED ${PROJECT_SOURCES})

target_enable_pch(${PROJECT_NAME})

target_include_directories(${PROJECT_NAME}
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
    PRIVATE ${CMAKE_CURRENT_BINARY_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE gc_thermal)

target_setup_translations(${PROJECT_NAME} ${HEADERS} ${SOURCES} ${UIS})

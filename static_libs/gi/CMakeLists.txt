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

project(gi LANGUAGES CXX)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

target_link_libraries(${PROJECT_NAME} PRIVATE gcode clipper
                                              Qt${QT_VERSION_MAJOR}::Widgets)

# target_compile_definitions(gi PRIVATE GI_LIBRARY)

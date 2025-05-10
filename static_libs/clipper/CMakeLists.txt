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

project(clipper LANGUAGES CXX)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(clipper STATIC ${HEADERS} ${SOURCES})
# add_library(clipper) target_sources(clipper PUBLIC FILE_SET CXX_MODULES FILES
# ${SOURCES})

target_link_libraries(clipper PRIVATE Clipper2Z Qt6::Widgets)

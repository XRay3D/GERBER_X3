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
cmake_minimum_required(VERSION 4.00)

project(shape)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
# file(GLOB UIS *.ui)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} ${UIS})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PUBLIC Qt6::Widgets gi)

foreach(SUBDIRECTORY ${SHAPE_PLUGINS})
  add_subdirectory(${SUBDIRECTORY})
endforeach()

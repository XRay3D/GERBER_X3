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

project(graphicsview LANGUAGES CXX)

find_package(
  Qt6
  COMPONENTS OpenGL
  REQUIRED)

if(6 GREATER_EQUAL 6)
  find_package(
    Qt6
    COMPONENTS OpenGLWidgets
    REQUIRED)
endif()

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(graphicsview STATIC ${HEADERS} ${SOURCES})

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

target_link_libraries(
  graphicsview PRIVATE common Qt6::Widgets
                       Qt6::OpenGL)

if(6 GREATER_EQUAL 6)
  target_link_libraries(graphicsview
                        PRIVATE Qt6::OpenGLWidgets)
endif()

# target_compile_definitions(graphicsview PRIVATE GRAPHICSVIEW_LIBRARY)

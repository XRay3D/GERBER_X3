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
cmake_minimum_required(VERSION 3.20)

project(graphicsview LANGUAGES CXX)

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Widgets OpenGL
  REQUIRED)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  find_package(
    Qt${QT_VERSION_MAJOR}
    COMPONENTS OpenGLWidgets
    REQUIRED)
endif()

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(graphicsview STATIC ${HEADERS} ${SOURCES})

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

target_link_libraries(
  graphicsview PRIVATE common Qt${QT_VERSION_MAJOR}::Widgets
                       Qt${QT_VERSION_MAJOR}::OpenGL)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
  target_link_libraries(graphicsview
                        PRIVATE Qt${QT_VERSION_MAJOR}::OpenGLWidgets)
endif()

# target_compile_definitions(graphicsview PRIVATE GRAPHICSVIEW_LIBRARY)

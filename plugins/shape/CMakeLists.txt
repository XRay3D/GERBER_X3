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

project(shape)

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
# file(GLOB UIS *.ui)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES} # ${UIS}
)

link_directories(${CMAKE_SOURCE_DIR}/libs${PATHS})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE Qt${QT_VERSION_MAJOR}::Widgets
  PUBLIC tbb12)

if(Enable_Circle)
  add_subdirectory(circle)
endif()

if(Enable_Circlearc)
  add_subdirectory(circlearc)
endif()

if(Enable_Polyline)
  add_subdirectory(polyline)
endif()

if(Enable_Rectangle)
  add_subdirectory(rectangle)
endif()

if(Enable_Text)
  add_subdirectory(text)
endif()

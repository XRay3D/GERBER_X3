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

project(common LANGUAGES CXX)

find_package(
  Qt6
  COMPONENTS Qml
  REQUIRED)

# set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL ON) # fix Qml

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC filetree
         shape
         gi
         graphicsview
         tooldatabase
         Qt6::Widgets
         ${EXECUTION}
         Qt6::Qml
         $<$<PLATFORM_ID:Linux>:
         # qcustomplot-qt6
         tbb
         >
         $<$<PLATFORM_ID:Windows>:
         # tbb12.dll QCustomPlot
         >)

# add_subdirectory(doublespinbox)

# add_module_library(QT qt.cpp)

# target_compile_definitions(common PRIVATE COMMON_LIBRARY)

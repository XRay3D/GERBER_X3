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

project(common LANGUAGES CXX)

find_package(
  Qt${QT_VERSION_MAJOR}
  COMPONENTS Qml
  REQUIRED)

# set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL ON) # fix Qml

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(common STATIC ${HEADERS} ${SOURCES})

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

target_link_libraries(
  common PRIVATE filetree shape gi graphicsview Qt${QT_VERSION_MAJOR}::Qml
                 Qt${QT_VERSION_MAJOR}::Widgets)

# add_subdirectory(doublespinbox)

# add_module_library(QT qt.cpp)

# target_compile_definitions(common PRIVATE COMMON_LIBRARY)

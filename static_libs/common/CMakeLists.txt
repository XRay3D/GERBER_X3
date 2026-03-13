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

project(common LANGUAGES CXX)

find_package(
  Qt6
  COMPONENTS Qml
  REQUIRED)

find_package(
  ICU REQUIRED
  COMPONENTS uc i18n
  REQUIRED)

# set(CMAKE_FIND_PACKAGE_TARGETS_GLOBAL ON) # fix Qml

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC ICU::i18n
         ICU::uc
         Qt6::Qml
         Qt6::Widgets
         filetree
         gi
         graphicsview
         shape
         tbb
         tooldatabase)

# add_subdirectory(doublespinbox)

# add_module_library(QT qt.cpp)

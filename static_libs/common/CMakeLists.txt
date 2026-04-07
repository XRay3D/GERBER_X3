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

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)

add_library(${PROJECT_NAME} STATIC ${HEADERS} ${SOURCES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(
  ${PROJECT_NAME}
  PUBLIC ICU::i18n
         ICU::uc
         filetree
         gi
         shape
         $<$<PLATFORM_ID:Linux>:
         tbb
         >
         $<$<PLATFORM_ID:Windows>:
         tbb12.dll
         >
         tooldatabase
         Qt6::Qml
         Qt6::Widgets)

# add_subdirectory(doublespinbox)

# add_module_library(QT qt.cpp)

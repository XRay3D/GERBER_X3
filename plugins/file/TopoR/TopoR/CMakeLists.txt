#[[#############################################################################
# Author    :  Damir Bakiev                                                    #
# Version   :  na                                                              #
# Date      :  06.02.2025                                                      #
# Website   :  na                                                              #
# Copyright :  Damir Bakiev 2016-2025                                          #
# License   :                                                                  #
# Use, modification & distribution is subject to Boost Software License Ver 1. #                                                                               #
# http://www.boost.org/LICENSE_1_0.txt                                         #
##############################################################################]]

cmake_minimum_required(VERSION 3.20)

project(TopoR LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

find_package(QT NAMES Qt6 Qt5 Widgets)
find_package(
  Qt6
  COMPONENTS Widgets
  REQUIRED)

add_library(${PROJECT_NAME} STATIC)

file(GLOB SRC ${CMAKE_CURRENT_LIST_DIR}/*.cpp)
file(GLOB HEADER ${CMAKE_CURRENT_LIST_DIR}/*.h)

target_sources(${PROJECT_NAME} PRIVATE ${SRC} #[[PUBLIC]] ${HEADER})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_LIST_DIR})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

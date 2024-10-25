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

project(shape)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

find_package(QT NAMES Qt5 #[[Qt6]] COMPONENTS Core Gui Widgets REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Gui Widgets REQUIRED)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
#file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

add_library(${PROJECT_NAME} STATIC
    ${HEADERS}
    ${SOURCES}
    #    ${UIS}
)

link_directories(
    ${CMAKE_SOURCE_DIR}/libs${PATHS}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Gui
    Qt${QT_VERSION_MAJOR}::Widgets
)

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


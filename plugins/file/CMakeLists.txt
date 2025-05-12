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

project(file_plugins)

file(GLOB HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)
file(GLOB UIS ${CMAKE_CURRENT_SOURCE_DIR}/*.ui)

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

link_directories(${CMAKE_SOURCE_DIR}/libs${PATHS})

target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Widgets)

set(TS_FILES ${PROJECT_NAME}_ru.ts ${PROJECT_NAME}_en.ts)

set_source_files_properties(
  ${TS_FILES} PROPERTIES OUTPUT_LOCATION "${OUTPUT_DIRECTORY}/translations")

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6) # Android
  qt_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
  qt_add_lupdate(${PROJECT_NAME} TS_FILES ${TS_FILES} SOURCES
                 ${PROJECT_SOURCES})
  # qt_add_lrelease(${PROJECT_NAME} TS_FILES ${TS_FILES}
  # QM_FILES_OUTPUT_VARIABLE QM_FILES)
else()
  qt5_create_translation(QM_FILES ${PROJECT_SOURCES} ${TS_FILES})
  qt5_add_translation(${QM_FILES} ${TS_FILES})
endif()

if(Enable_Gerber)
  add_subdirectory(gerber)
endif()

if(Enable_Dxf)
  add_subdirectory(dxf)
endif()

if(Enable_Excellon)
  add_subdirectory(excellon)
endif()

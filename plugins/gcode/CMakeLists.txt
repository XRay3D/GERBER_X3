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

project(gcode)

# include_directories()

file(GLOB HEADERS *.h)
file(GLOB SOURCES *.cpp)
file(GLOB UIS *.ui)

set(PROJECT_SOURCES ${HEADERS} ${SOURCES} ${UIS})

add_library(${PROJECT_NAME} STATIC ${PROJECT_SOURCES})

# target_precompile_headers(${PROJECT_NAME} PUBLIC ${HEADERS})

link_directories(${CMAKE_SOURCE_DIR}/libs${PATHS})

target_link_libraries(
  ${PROJECT_NAME}
  PRIVATE clipper file_plugins gi Qt6::Widgets
  PUBLIC #[[tbb12]] tbb)

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

if(Enable_Pocketoffset)
  add_subdirectory(pocketoffset)
endif()

if(Enable_Profile)
  add_subdirectory(profile)
endif()

if(Enable_Drill)
  add_subdirectory(drill)
endif()

if(Enable_Hatching)
  add_subdirectory(hatching)
endif()

if(Enable_Pocketraster)
  add_subdirectory(pocketraster)
endif()

if(Enable_Voroni)
  add_subdirectory(voroni)
endif()

if(Enable_Thermal)
  add_subdirectory(thermal)
endif()

if(Enable_Thread)
  # add_subdirectory(thread)
endif()

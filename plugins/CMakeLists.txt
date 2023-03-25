#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  March 25, 2023                                                  *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2023                                          *
#* License   :                                                                  *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#********************************************************************************/
cmake_minimum_required(VERSION 3.20)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PLUGINS_DIR})
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${STATIC_LIBS_DIR})
if (CMAKE_CXX_COMPILER_ID STREQUAL "GNU")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PLUGINS_DIR})
else()
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${STATIC_LIBS_DIR})
endif()

# plugins shape
add_subdirectory(shape)

# plugins file
add_subdirectory(file)

# plugins gcode
add_subdirectory(gcode)

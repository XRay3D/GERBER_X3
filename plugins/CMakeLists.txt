#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  03 October 2022                                                 *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
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

set(Enable OFF)

# plugins shape
set(Enable_Circle       OFF) #[["Enable_Circle"]]
set(Enable_Circlearc    OFF) #[["Enable_Circlearc"]]
set(Enable_Polyline     OFF) #[["Enable_Polyline"]]
set(Enable_Rectangle    OFF) #[["Enable_Rectangle"]]
set(Enable_Text         OFF) #[["Enable_Text"]]
add_subdirectory(shape)

# plugins file
set(Enable_Gerber       ON)  #[["Enable_Gerber"]]
set(Enable_Dxf          OFF) #[["Enable_Dxf"]]
set(Enable_Excellon     OFF) #[["Enable_Excellon"]]
add_subdirectory(file)

# plugins gcode
set(Enable_Drill        OFF) #[["Enable_Drill"]]
set(Enable_Hatching     OFF) #[["Enable_Hatching"]]
set(Enable_Pocketoffset OFF) #[["Enable_Pocketoffset"]]
set(Enable_Pocketraster OFF) #[["Enable_Pocketraster"]]
set(Enable_Profile      OFF) #[["Enable_Profile"]]
set(Enable_Thermal      OFF) #[["Enable_Thermal"]]
set(Enable_Voroni       OFF) #[["Enable_Voroni"]]
add_subdirectory(gcode)







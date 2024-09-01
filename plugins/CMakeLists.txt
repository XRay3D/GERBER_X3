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

set(Enable OFF)

# plugins shape
set(Enable_Circle       Enable)
set(Enable_Circlearc    Enable)
set(Enable_Polyline     Enable)
set(Enable_Rectangle    Enable)
set(Enable_Text         Enable)
add_subdirectory(shape)

# plugins file
set(Enable_Gerber       Enable)
set(Enable_Dxf          Enable)
set(Enable_Excellon     Enable)
add_subdirectory(file)

# plugins gcode
set(Enable_Pocketoffset ON)
set(Enable_Profile      ON)
set(Enable_Drill        ON)
set(Enable_Hatching     Enable)
set(Enable_Pocketraster Enable)
set(Enable_Thermal      Enable)
set(Enable_Voroni       Enable)
add_subdirectory(gcode)







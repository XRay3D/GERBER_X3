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

# plugins shape
set(Enable_Circle ON)
set(Enable_Circlearc ON)
set(Enable_Polyline ON)
set(Enable_Rectangle ON)
set(Enable_Text ON)
add_subdirectory(shape)

#[[
# plugins file
set(Enable_Gerber ON)
set(Enable_Dxf ON)
set(Enable_Excellon ON)
]]
add_subdirectory(file)

# plugins gcode
set(Enable_Pocketoffset ON)
set(Enable_Profile ON)
#[[
set(Enable_Drill ON)
set(Enable_Hatching ON)
set(Enable_Pocketraster ON)
set(Enable_Thermal ON)
set(Enable_Voroni ON)
]]
add_subdirectory(gcode)

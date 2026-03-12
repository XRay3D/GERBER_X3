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

set(SHAPE_PLUGINS circle circlearc polyline rectangle text)
add_subdirectory(shape)

set(FILE_PLUGINS gerber dxf excellon)
add_subdirectory(file)

set(GCODE_PLUGINS
    pocketoffset
    profile
    drill
    hatching
    pocketraster
    voroni
    thermal)
add_subdirectory(gcode)

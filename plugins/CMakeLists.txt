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
cmake_minimum_required(VERSION 4.0)

# set(SHAPE_PLUGINS circle circlearc polyline rectangle text)
add_subdirectory(shape)

set(FILE_PLUGINS # gerber  excellon svg
    dxf)
add_subdirectory(file)

set(GCODE_PLUGINS profile pocketoffset
                  # drill hatching pocketraster voroni thermal thread
)
add_subdirectory(gcode)

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

# static lib
#
# geo -- точная геометрия на CGAL. static_libs/geometry (старая, на Clipper2)
# снесена; алиас оставлен -- на имя "geometry" всё ещё линкуются некоторые
# CMakeLists.txt (в т.ч. у отключённых plugins/file/gerber2 и
# plugins/gcode/thread).
add_subdirectory(geo)
add_library(geometry ALIAS Geo)

add_subdirectory(filetree)
add_subdirectory(gi)
add_subdirectory(graphicsview)
add_subdirectory(common)
add_subdirectory(tooldatabase)

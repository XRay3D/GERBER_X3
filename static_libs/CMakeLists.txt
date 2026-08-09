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
# geo -- точная геометрия на CGAL, новая. geometry -- старая, на Clipper2:
# myclipper.h/geometry.h/circle.h/span.h ещё держат на себе полпроекта, так
# что до конца переезда живут обе, и geometry зависит от Geo, а не наоборот.
add_subdirectory(geo)
add_library(geometry ALIAS Geo)

add_subdirectory(filetree)
add_subdirectory(gi)
add_subdirectory(graphicsview)
add_subdirectory(common)
add_subdirectory(tooldatabase)

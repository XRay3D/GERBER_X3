#[[
 Author    :  Damir Bakiev
 Version   :  na
 Date      :  01 February 2020
 Website   :  na
 Copyright :  Damir Bakiev 2016-2022
 License:
 Use, modification & distribution is subject to Boost Software License Ver 1.
 http://www.boost.org/LICENSE_1_0.txt
]]
cmake_minimum_required(VERSION 3.16)

project(hpgl LANGUAGES CXX)




find_package(Qt${QT_VERSION_MAJOR} COMPONENTS   Widgets REQUIRED)

#add_subdirectory(entities)
#add_subdirectory(section)
#add_subdirectory(tables)

add_library(hpgl SHARED
    hpgl.json
    hpgl_file.cpp
    hpgl_file.h
    hpgl_node.cpp
    hpgl_node.h
    hpgl_parser.cpp
    hpgl_parser.h
    hpgl_plugin.cpp
    hpgl_plugin.h
    hpgl_sourcedialog.cpp
    hpgl_sourcedialog.h
    hpgl_types.cpp
    hpgl_types.h
    ${HEADERS}
    ${SOURCE}
)

link_directories(../../libs${SUFFIX})
target_link_libraries(hpgl PRIVATE
    clipper
    common
    filetree
    gi
    graphicsview
    tooldatabase
    Qt${QT_VERSION_MAJOR}::Widgets
)

#target_compile_definitions(hpgl PRIVATE HPGL_LIBRARY)

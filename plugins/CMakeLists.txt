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

#set(SHAPE_PLUGINS circle circlearc polyline rectangle text)
add_subdirectory(shape)

set(FILE_PLUGINS gerber dxf excellon #gerber2 svg
)
add_subdirectory(file)

#Переведены на Geo(точная геометрия на CGAL) и собираются всегда.
set(GCODE_PLUGINS profile pocketoffset drill thermal hatching pocketraster)

#Ещё на Clipper2 и на прежнем виде GCode::Creator : их геометрия(диаграмма
#Вороного, резьба) в точный домен так просто не переносится, поэтому они
#отложены до отдельного захода.Код лежит в дереве нетронутым; чтобы он не гнил
#незаметно, список печатается при конфигурации.
set(GCODE_PLUGINS_LEGACY voroni thread)
option(GGEASY_GCODE_LEGACY "Собирать gcode-плагины, ещё не переведённые на Geo"
       OFF)
if(GGEASY_GCODE_LEGACY)
  list(APPEND GCODE_PLUGINS ${GCODE_PLUGINS_LEGACY})
else()
  message(STATUS "gcode: отложены до перехода на Geo: ${GCODE_PLUGINS_LEGACY}"
                 " (включить: -DGGEASY_GCODE_LEGACY=ON)")
endif()

add_subdirectory(gcode)

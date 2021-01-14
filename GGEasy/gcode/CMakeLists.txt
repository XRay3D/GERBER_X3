#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  14 January 2021                                                 *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2021                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.14)

set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/gccreator.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcdrillitem.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcfile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcnode.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcplugin.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcpocketoffset.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcpocketraster.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcprofile.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcthermal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcutils.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoi.cpp
    PARENT_SCOPE
    )

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/gccreator.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcdrillitem.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcfile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcnode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcode.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcplugin.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcpocketoffset.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcpocketraster.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcprofile.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcthermal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gctypes.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcutils.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoi.h
    PARENT_SCOPE
    )




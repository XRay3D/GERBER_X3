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
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoi.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoiboost.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoicgal.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoijc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/jc_voronoi.cpp
    PARENT_SCOPE
    )

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoiboost.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoicgal.h
    ${CMAKE_CURRENT_SOURCE_DIR}/gcvoronoijc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/jc_voronoi.h
    ${CMAKE_CURRENT_SOURCE_DIR}/voronoi_visual_utils.hpp
    PARENT_SCOPE
    )

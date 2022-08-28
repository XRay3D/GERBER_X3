#/*******************************************************************************
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2022                                          *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*******************************************************************************/
cmake_minimum_required(VERSION 3.16)

set(SOURCE
    ${SOURCE}
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_arc.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_attdef.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_attrib.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_circle.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_dummy.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ellipse.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_entity.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_graphicobject.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_hatch.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_insert.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_line.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_lwpolyline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mtext.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_point.cpp#
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_polyline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_seqend.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_solid.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_spline.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_text.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_vec2.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_vertex.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_attrib.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_body.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_dimension.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_helix.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_image.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_leader.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_light.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mesh.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mleader.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mleaderstyle.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mline.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ole2frame.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_oleframe.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ray.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_region.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_section.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_shape.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_sun.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_surface.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_table.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_tolerance.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_trace.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_underlay.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_viewport.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_wipeout.cpp
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_xline.cpp
    PARENT_SCOPE
)

set(HEADERS
    ${HEADERS}
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_allentities.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_arc.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_attdef.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_attrib.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_circle.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_dummy.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ellipse.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_entity.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_graphicobject.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_hatch.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_insert.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_line.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_lwpolyline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mtext.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_point.h#
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_polyline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_seqend.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_solid.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_spline.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_text.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_vec2.h
    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_vertex.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_attrib.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_body.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_dimension.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_helix.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_image.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_leader.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_light.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mesh.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mleader.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mleaderstyle.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_mline.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ole2frame.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_oleframe.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_ray.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_region.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_section.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_shape.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_sun.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_surface.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_table.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_tolerance.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_trace.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_underlay.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_viewport.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_wipeout.h
    #    ${CMAKE_CURRENT_SOURCE_DIR}/dxf_xline.h
    PARENT_SCOPE
)

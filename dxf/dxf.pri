# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD

HEADERS += \
    $$PWD/dxf_block.h \
    $$PWD/dxf_codedata.h \
    $$PWD/dxf_file.h \
    $$PWD/dxf_node.h \
    $$PWD/dxf_parser.h \
    $$PWD/dxf_types.h \
    $$PWD/entities/dxf_arc.h \
    $$PWD/entities/dxf_attdef.h \
    $$PWD/entities/dxf_circle.h \
    $$PWD/entities/dxf_dummy.h \
    $$PWD/entities/dxf_ellipse.h \
    $$PWD/entities/dxf_entity.h \
    $$PWD/entities/dxf_graphicobject.h \
    $$PWD/entities/dxf_hatch.h \
    $$PWD/entities/dxf_insert.h \
    $$PWD/entities/dxf_line.h \
    $$PWD/entities/dxf_lwpolyline.h \
    $$PWD/entities/dxf_mtext.h \
    $$PWD/entities/dxf_point.h \ # unimplemented
    $$PWD/entities/dxf_polyline.h \
    $$PWD/entities/dxf_solid.h \
    $$PWD/entities/dxf_spline.h \
    $$PWD/entities/dxf_text.h \
    $$PWD/entities/dxf_vec2.h \
    $$PWD/entities/dxf_vertex.h \
    $$PWD/section/dxf_blocks.h \
    $$PWD/section/dxf_entities.h \
    $$PWD/section/dxf_headerparser.h \
    $$PWD/section/dxf_sectionparser.h \
    $$PWD/section/dxf_tables.h \
    $$PWD/tables/dxf_abstracttable.h \
    $$PWD/tables/dxf_layer.h \
    $$PWD/tables/dxf_layermodel.h \
#    $$PWD/entities/dxf_attrib.h \ # unimplemented
#    $$PWD/entities/dxf_body.h \ # unimplemented
#    $$PWD/entities/dxf_dimension.h \ # unimplemented
#    $$PWD/entities/dxf_helix.h \ # unimplemented
#    $$PWD/entities/dxf_image.h \ # unimplemented
#    $$PWD/entities/dxf_leader.h \ # unimplemented
#    $$PWD/entities/dxf_light.h \ # unimplemented
#    $$PWD/entities/dxf_mesh.h \ # unimplemented
#    $$PWD/entities/dxf_mleader.h \ # unimplemented
#    $$PWD/entities/dxf_mleaderstyle.h \ # unimplemented
#    $$PWD/entities/dxf_mline.h \ # unimplemented
#    $$PWD/entities/dxf_ole2frame.h \ # unimplemented
#    $$PWD/entities/dxf_oleframe.h \ # unimplemented
#    $$PWD/entities/dxf_ray.h \ # unimplemented
#    $$PWD/entities/dxf_region.h \ # unimplemented
#    $$PWD/entities/dxf_section.h \ # unimplemented
#    $$PWD/entities/dxf_shape.h \ # unimplemented
#    $$PWD/entities/dxf_sun.h \ # unimplemented
#    $$PWD/entities/dxf_surface.h \ # unimplemented
#    $$PWD/entities/dxf_table.h \ # unimplemented
#    $$PWD/entities/dxf_tolerance.h \ # unimplemented
#    $$PWD/entities/dxf_trace.h \ # unimplemented
#    $$PWD/entities/dxf_underlay.h \ # unimplemented
#    $$PWD/entities/dxf_viewport.h \ # unimplemented
#    $$PWD/entities/dxf_wipeout.h \ # unimplemented
#    $$PWD/entities/dxf_xline.h \ # unimplemented
#    $$PWD/section/dxf_classes.h \
#    $$PWD/section/dxf_objects.h \
#    $$PWD/section/dxf_thumbnailimage.h \
#    $$PWD/tables/dxf_appid.h \
#    $$PWD/tables/dxf_block_record.h \
#    $$PWD/tables/dxf_dimstyle.h \
#    $$PWD/tables/dxf_ltype.h \
#    $$PWD/tables/dxf_style.h \
#    $$PWD/tables/dxf_ucs.h \
#    $$PWD/tables/dxf_view.h \
#    $$PWD/tables/dxf_vport.h \

SOURCES += \
    $$PWD/dxf_block.cpp \
    $$PWD/dxf_codedata.cpp \
    $$PWD/dxf_file.cpp \
    $$PWD/dxf_node.cpp \
    $$PWD/dxf_parser.cpp \
    $$PWD/dxf_types.cpp \
    $$PWD/entities/dxf_arc.cpp \
    $$PWD/entities/dxf_attdef.cpp \
    $$PWD/entities/dxf_circle.cpp \
    $$PWD/entities/dxf_dummy.cpp \
    $$PWD/entities/dxf_ellipse.cpp \
    $$PWD/entities/dxf_entity.cpp \
    $$PWD/entities/dxf_graphicobject.cpp \
    $$PWD/entities/dxf_hatch.cpp \
    $$PWD/entities/dxf_insert.cpp \
    $$PWD/entities/dxf_line.cpp \
    $$PWD/entities/dxf_lwpolyline.cpp \
    $$PWD/entities/dxf_mtext.cpp \
    $$PWD/entities/dxf_point.cpp \ # unimplemented
    $$PWD/entities/dxf_polyline.cpp \
    $$PWD/entities/dxf_solid.cpp \
    $$PWD/entities/dxf_spline.cpp \
    $$PWD/entities/dxf_text.cpp \
    $$PWD/entities/dxf_vec2.cpp \
    $$PWD/entities/dxf_vertex.cpp \
    $$PWD/section/dxf_blocks.cpp \
    $$PWD/section/dxf_entities.cpp \
    $$PWD/section/dxf_headerparser.cpp \
    $$PWD/section/dxf_sectionparser.cpp \
    $$PWD/section/dxf_tables.cpp \
    $$PWD/tables/dxf_abstracttable.cpp \
    $$PWD/tables/dxf_layer.cpp \
    $$PWD/tables/dxf_layermodel.cpp \
#    $$PWD/entities/dxf_attrib.cpp \ # unimplemented
#    $$PWD/entities/dxf_body.cpp \ # unimplemented
#    $$PWD/entities/dxf_dimension.cpp \ # unimplemented
#    $$PWD/entities/dxf_helix.cpp \ # unimplemented
#    $$PWD/entities/dxf_image.cpp \ # unimplemented
#    $$PWD/entities/dxf_leader.cpp \ # unimplemented
#    $$PWD/entities/dxf_light.cpp \ # unimplemented
#    $$PWD/entities/dxf_mesh.cpp \ # unimplemented
#    $$PWD/entities/dxf_mleader.cpp \ # unimplemented
#    $$PWD/entities/dxf_mleaderstyle.cpp \ # unimplemented
#    $$PWD/entities/dxf_mline.cpp \ # unimplemented
#    $$PWD/entities/dxf_ole2frame.cpp \ # unimplemented
#    $$PWD/entities/dxf_oleframe.cpp \ # unimplemented
#    $$PWD/entities/dxf_ray.cpp \ # unimplemented
#    $$PWD/entities/dxf_region.cpp \ # unimplemented
#    $$PWD/entities/dxf_section.cpp \ # unimplemented
#    $$PWD/entities/dxf_shape.cpp \ # unimplemented
#    $$PWD/entities/dxf_sun.cpp \ # unimplemented
#    $$PWD/entities/dxf_surface.cpp \ # unimplemented
#    $$PWD/entities/dxf_table.cpp \ # unimplemented
#    $$PWD/entities/dxf_tolerance.cpp \ # unimplemented
#    $$PWD/entities/dxf_trace.cpp \ # unimplemented
#    $$PWD/entities/dxf_underlay.cpp \ # unimplemented
#    $$PWD/entities/dxf_viewport.cpp \ # unimplemented
#    $$PWD/entities/dxf_wipeout.cpp \ # unimplemented
#    $$PWD/entities/dxf_xline.cpp \ # unimplemented
#    $$PWD/section/dxf_classes.cpp \
#    $$PWD/section/dxf_objects.cpp \
#    $$PWD/section/dxf_thumbnailimage.cpp \
#    $$PWD/tables/dxf_appid.cpp \
#    $$PWD/tables/dxf_block_record.cpp \
#    $$PWD/tables/dxf_dimstyle.cpp \
#    $$PWD/tables/dxf_ltype.cpp \
#    $$PWD/tables/dxf_style.cpp \
#    $$PWD/tables/dxf_ucs.cpp \
#    $$PWD/tables/dxf_view.cpp \
#    $$PWD/tables/dxf_vport.cpp \

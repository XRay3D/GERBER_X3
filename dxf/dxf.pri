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
    $$PWD/block.h \
    $$PWD/codedata.h \
    $$PWD/dxffile.h \
    $$PWD/dxfnode.h \
    $$PWD/dxfparser.h \
    $$PWD/dxfvalues.h \
    $$PWD/entities/dxf_arc.h \
    $$PWD/entities/dxf_attdef.h \
    $$PWD/entities/dxf_circle.h \
    $$PWD/entities/dxf_ellipse.h \
    $$PWD/entities/dxf_entity.h \
    $$PWD/entities/dxf_graphicobject.h \
    $$PWD/entities/dxf_hatch.h \
    $$PWD/entities/dxf_insert.h \
    $$PWD/entities/dxf_line.h \
    $$PWD/entities/dxf_lwpolyline.h \
    $$PWD/entities/dxf_mtext.h \
    $$PWD/entities/dxf_polyline.h \
    $$PWD/entities/dxf_solid.h \
    $$PWD/entities/dxf_spline.h \
    $$PWD/entities/dxf_text.h \
    $$PWD/entities/dxf_vertex.h \
    $$PWD/header.h \
    $$PWD/section/blocks.h \
    $$PWD/section/classes.h \
    $$PWD/section/entities.h \
    $$PWD/section/headerparser.h \
    $$PWD/section/objects.h \
    $$PWD/section/sectionparser.h \
    $$PWD/section/tables.h \
    $$PWD/section/thumbnailimage.h \
    $$PWD/tables/appid.h \
    $$PWD/tables/block_record.h \
    $$PWD/tables/dimstyle.h \
    $$PWD/tables/layer.h \
    $$PWD/tables/layermodel.h \
    $$PWD/tables/ltype.h \
    $$PWD/tables/style.h \
    $$PWD/tables/tableitem.h \
    $$PWD/tables/ucs.h \
    $$PWD/tables/view.h \
    $$PWD/tables/vport.h \
#    $$PWD/entities/attrib.h \
#    $$PWD/entities/body.h \
#    $$PWD/entities/dimension.h \
#    $$PWD/entities/helix.h \
#    $$PWD/entities/image.h \
#    $$PWD/entities/leader.h \
#    $$PWD/entities/light.h \
#    $$PWD/entities/mesh.h \
#    $$PWD/entities/mleader.h \
#    $$PWD/entities/mleaderstyle.h \
#    $$PWD/entities/mline.h \
#    $$PWD/entities/ole2frame.h \
#    $$PWD/entities/oleframe.h \
#    $$PWD/entities/point.h \
#    $$PWD/entities/ray.h \
#    $$PWD/entities/region.h \
#    $$PWD/entities/section.h \
#    $$PWD/entities/shape.h \
#    $$PWD/entities/sun.h \
#    $$PWD/entities/surface.h \
#    $$PWD/entities/table.h \
#    $$PWD/entities/tolerance.h \
#    $$PWD/entities/trace.h \
#    $$PWD/entities/underlay.h \
#    $$PWD/entities/viewport.h \
#    $$PWD/entities/wipeout.h \
#    $$PWD/entities/xline.h \

SOURCES += \
    $$PWD/block.cpp \
    $$PWD/codedata.cpp \
    $$PWD/dxffile.cpp \
    $$PWD/dxfnode.cpp \
    $$PWD/dxfparser.cpp \
    $$PWD/dxfvalues.cpp \
    $$PWD/entities/dxf_arc.cpp \
    $$PWD/entities/dxf_attdef.cpp \
    $$PWD/entities/dxf_circle.cpp \
    $$PWD/entities/dxf_ellipse.cpp \
    $$PWD/entities/dxf_entity.cpp \
    $$PWD/entities/dxf_graphicobject.cpp \
    $$PWD/entities/dxf_hatch.cpp \
    $$PWD/entities/dxf_insert.cpp \
    $$PWD/entities/dxf_line.cpp \
    $$PWD/entities/dxf_lwpolyline.cpp \
    $$PWD/entities/dxf_mtext.cpp \
    $$PWD/entities/dxf_polyline.cpp \
    $$PWD/entities/dxf_solid.cpp \
    $$PWD/entities/dxf_spline.cpp \
    $$PWD/entities/dxf_text.cpp \
    $$PWD/entities/dxf_vertex.cpp \
    $$PWD/header.cpp \
    $$PWD/section/blocks.cpp \
    $$PWD/section/classes.cpp \
    $$PWD/section/entities.cpp \
    $$PWD/section/headerparser.cpp \
    $$PWD/section/objects.cpp \
    $$PWD/section/sectionparser.cpp \
    $$PWD/section/tables.cpp \
    $$PWD/section/thumbnailimage.cpp \
    $$PWD/tables/appid.cpp \
    $$PWD/tables/block_record.cpp \
    $$PWD/tables/dimstyle.cpp \
    $$PWD/tables/layer.cpp \
    $$PWD/tables/layermodel.cpp \
    $$PWD/tables/ltype.cpp \
    $$PWD/tables/style.cpp \
    $$PWD/tables/tableitem.cpp \
    $$PWD/tables/ucs.cpp \
    $$PWD/tables/view.cpp \
    $$PWD/tables/vport.cpp \
#    $$PWD/entities/attrib.cpp \
#    $$PWD/entities/body.cpp \
#    $$PWD/entities/dimension.cpp \
#    $$PWD/entities/helix.cpp \
#    $$PWD/entities/image.cpp \
#    $$PWD/entities/leader.cpp \
#    $$PWD/entities/light.cpp \
#    $$PWD/entities/mesh.cpp \
#    $$PWD/entities/mleader.cpp \
#    $$PWD/entities/mleaderstyle.cpp \
#    $$PWD/entities/mline.cpp \
#    $$PWD/entities/ole2frame.cpp \
#    $$PWD/entities/oleframe.cpp \
#    $$PWD/entities/point.cpp \
#    $$PWD/entities/ray.cpp \
#    $$PWD/entities/region.cpp \
#    $$PWD/entities/section.cpp \
#    $$PWD/entities/shape.cpp \
#    $$PWD/entities/sun.cpp \
#    $$PWD/entities/surface.cpp \
#    $$PWD/entities/table.cpp \
#    $$PWD/entities/tolerance.cpp \
#    $$PWD/entities/trace.cpp \
#    $$PWD/entities/underlay.cpp \
#    $$PWD/entities/viewport.cpp \
#    $$PWD/entities/wipeout.cpp \
#    $$PWD/entities/xline.cpp \

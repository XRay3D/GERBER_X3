#/*******************************************************************************
#*                                                                              *
#* Author    :  Damir Bakiev                                                    *
#* Version   :  na                                                              *
#* Date      :  01 February 2020                                                *
#* Website   :  na                                                              *
#* Copyright :  Damir Bakiev 2016-2020                                          *
#*                                                                              *
#* License:                                                                     *
#* Use, modification & distribution is subject to Boost Software License Ver 1. *
#* http://www.boost.org/LICENSE_1_0.txt                                         *
#*                                                                              *
#*******************************************************************************/
include(../plugin.pri)
include(../defines.pri)
include(../suffix.pri)

TARGET = $$TARGET$$SUFFIX

message($$TARGET)

msvc* {
    LIBS += -lsetupapi -lAdvapi32
    QMAKE_CXXFLAGS += /std:c++latest
    #DEFINES += LEAK_DETECTOR
    LIBS += -l$$_PRO_FILE_PWD_/../lib/clipper$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/settings$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/graphicsview$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/gi$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/filetree$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/project$$SUFFIX
}

gcc* {
    CONFIG += c++17
    win* {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
    LIBS += "-L"$$_PRO_FILE_PWD_/../lib
    LIBS += -lclipper$$SUFFIX
    LIBS += -lsettings$$SUFFIX
    LIBS += -lgraphicsview$$SUFFIX
    LIBS += -lgi$$SUFFIX
    LIBS += -lfiletree$$SUFFIX
    LIBS += -lproject$$SUFFIX
}

linux {
    # sudo apt install mesa-common-dev
    DEFINES += linux
    greaterThan(QT_MINOR_VERSION, 12){
        LIBS += -ltbb # Why????? sudo apt-get install libtbb-dev
    }
}

DESTDIR = $$_PRO_FILE_PWD_/../bin

INCLUDEPATH += ../GGEasy
INCLUDEPATH += ../clipper
INCLUDEPATH += ../settings
INCLUDEPATH += ../graphicsview
INCLUDEPATH += ../gi
INCLUDEPATH += ../filetree
INCLUDEPATH += ../project

EXAMPLE_FILES = gerber.json

HEADERS += \
    dxf_block.h \
    dxf_codedata.h \
    dxf_file.h \
    dxf_node.h \
    dxf_plugin.h \
    dxf_sourcedialog.h \
    dxf_types.h \
    entities/dxf_arc.h \
    entities/dxf_attdef.h \
    entities/dxf_circle.h \
    entities/dxf_dummy.h \
    entities/dxf_ellipse.h \
    entities/dxf_entity.h \
    entities/dxf_graphicobject.h \
    entities/dxf_hatch.h \
    entities/dxf_insert.h \
    entities/dxf_line.h \
    entities/dxf_lwpolyline.h \
    entities/dxf_mtext.h \
    entities/dxf_point.h \ # unimplemented
    entities/dxf_polyline.h \
    entities/dxf_solid.h \
    entities/dxf_spline.h \
    entities/dxf_text.h \
    entities/dxf_vec2.h \
    entities/dxf_vertex.h \
    section/dxf_blocks.h \
    section/dxf_entities.h \
    section/dxf_headerparser.h \
    section/dxf_sectionparser.h \
    section/dxf_tables.h \
    tables/dxf_abstracttable.h \
    tables/dxf_layer.h \
    tables/dxf_layermodel.h \
    tables/dxf_style.h \
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
#    $$PWD/tables/dxf_ucs.h \
#    $$PWD/tables/dxf_view.h \
#    $$PWD/tables/dxf_vport.h \

SOURCES += \
    dxf_block.cpp \
    dxf_codedata.cpp \
    dxf_file.cpp \
    dxf_node.cpp \
    dxf_plugin.cpp \
    dxf_sourcedialog.cpp \
    dxf_types.cpp \
    entities/dxf_arc.cpp \
    entities/dxf_attdef.cpp \
    entities/dxf_circle.cpp \
    entities/dxf_dummy.cpp \
    entities/dxf_ellipse.cpp \
    entities/dxf_entity.cpp \
    entities/dxf_graphicobject.cpp \
    entities/dxf_hatch.cpp \
    entities/dxf_insert.cpp \
    entities/dxf_line.cpp \
    entities/dxf_lwpolyline.cpp \
    entities/dxf_mtext.cpp \
    entities/dxf_point.cpp \ # unimplemented
    entities/dxf_polyline.cpp \
    entities/dxf_solid.cpp \
    entities/dxf_spline.cpp \
    entities/dxf_text.cpp \
    entities/dxf_vec2.cpp \
    entities/dxf_vertex.cpp \
    section/dxf_blocks.cpp \
    section/dxf_entities.cpp \
    section/dxf_headerparser.cpp \
    section/dxf_sectionparser.cpp \
    section/dxf_tables.cpp \
    tables/dxf_abstracttable.cpp \
    tables/dxf_layer.cpp \
    tables/dxf_layermodel.cpp \
    tables/dxf_style.cpp \
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
#    $$PWD/tables/dxf_ucs.cpp \
#    $$PWD/tables/dxf_view.cpp \
#    $$PWD/tables/dxf_vport.cpp \

DISTFILES += \
    dxf.json

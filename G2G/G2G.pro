QT += core gui opengl widgets printsupport

TARGET = Getber2Gcode
TEMPLATE = app

QMAKE_CXXFLAGS += /std:c++latest

RESOURCES += \
    res/resources.qrc \

#DEFINES += QT_DEBUG
DEFINES += QT_DEPRECATED_WARNINGS G2G
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 0.7.7
VER_MAJ = 0
VER_MIN = 7
VER_PAT = 7

ICON = 256.png

#macx: ICON = resources/icon.icns

win32-msvc* {
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
}
win32* {
    LIBS += -lsetupapi -lAdvapi32 -lpsapi
    RC_FILE = myapp.rc
}

TRANSLATIONS += \
    translations/g2g_en.ts \
    translations/g2g_ru.ts

HEADERS += \
    aboutform.h \
    application.h \
    colorselector.h \
    doublespinbox.h \
    filetree/abstractnode.h \
    filetree/drillnode.h \
    filetree/filemodel.h \
    filetree/foldernode.h \
    filetree/gcodenode.h \
    filetree/gerbernode.h \
    filetree/layerdelegate.h \
    filetree/treeview.h \
    forms/depthform.h \
    forms/drillform.h \
    forms/drillmodel.h \
    forms/formsutil.h \
    forms/gcodepropertiesform.h \
    forms/pocketform.h \
    forms/profileform.h \
    forms/thermalform.h \
    forms/thermalmodel.h \
    forms/thermalpreviewitem.h \
    forms/voronoiform.h \
    gcode/gccreator.h \
    gcode/gcfile.h \
    gi/boarditem.h \
    gi/bridgeitem.h \
    gi/drillitem.h \
    gi/gerberitem.h \
    gi/graphicsitem.h \
    gi/itemgroup.h \
    gi/pathitem.h \
    gi/rawitem.h \
    mainwindow.h \
    openingdialog.h \
    point.h \
    settingsdialog.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooleditdialog.h \
    tooldatabase/tooleditform.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    icons.h \
    forms/previewitem.h \
    version.h \
    voroni/jc_voronoi.h \
    settings.h \
    project.h


SOURCES += \
    aboutform.cpp \
    colorselector.cpp \
    doublespinbox.cpp \
    filetree/abstractnode.cpp \
    filetree/drillnode.cpp \
    filetree/filemodel.cpp \
    filetree/foldernode.cpp \
    filetree/gcodenode.cpp \
    filetree/gerbernode.cpp \
    filetree/layerdelegate.cpp \
    filetree/treeview.cpp \
    forms/depthform.cpp \
    forms/drillform.cpp \
    forms/drillmodel.cpp \
    forms/formsutil.cpp \
    forms/gcodepropertiesform.cpp \
    forms/pocketform.cpp \
    forms/profileform.cpp \
    forms/thermalform.cpp \
    forms/thermalmodel.cpp \
    forms/thermalpreviewitem.cpp \
    forms/voronoiform.cpp \
    gcode/gccreator.cpp \
    gcode/gcfile.cpp \
    gi/boarditem.cpp \
    gi/bridgeitem.cpp \
    gi/drillitem.cpp \
    gi/gerberitem.cpp \
    gi/graphicsitem.cpp \
    gi/itemgroup.cpp \
    gi/pathitem.cpp \
    gi/rawitem.cpp \
    main.cpp \
    mainwindow.cpp \
    point.cpp \
    settingsdialog.cpp \
    tooldatabase/tool.cpp \
    tooldatabase/tooldatabase.cpp \
    tooldatabase/tooleditdialog.cpp \
    tooldatabase/tooleditform.cpp \
    tooldatabase/toolitem.cpp \
    tooldatabase/toolmodel.cpp \
    tooldatabase/tooltreeview.cpp \
    forms/previewitem.cpp \
    voroni/jc_voronoi.cpp \
    settings.cpp \
    project.cpp



FORMS += \
    aboutform.ui \
    colorselector.ui \
    forms/drillform.ui \
    forms/gcodepropertiesform.ui \
    forms/pocketform.ui \
    forms/profileform.ui \
    forms/thermalform.ui \
    forms/voronoiform.ui \
    mainwindow.ui \
    settingsdialog.ui \
    tooldatabase/tooleditdialog.ui \
    tooldatabase/tooleditform.ui \
    tooldatabase/tooldatabase.ui \

DISTFILES += \
    ../icons/breeze/actions/16/acrobat.svg \
    ../icons/breeze/actions/16/application-exit.svg \
    ../icons/breeze/actions/16/configure-shortcuts.svg \
    ../icons/breeze/actions/16/document-close.svg \
    ../icons/breeze/actions/16/document-edit.svg \
    ../icons/breeze/actions/16/document-export.svg \
    ../icons/breeze/actions/16/document-open.svg \
    ../icons/breeze/actions/16/document-save-all.svg \
    ../icons/breeze/actions/16/document-save.svg \
    ../icons/breeze/actions/16/draw-ellipse-arc.svg \
    ../icons/breeze/actions/16/draw-ellipse-segment.svg \
    ../icons/breeze/actions/16/draw-ellipse-whole.svg \
    ../icons/breeze/actions/16/draw-ellipse.svg \
    ../icons/breeze/actions/16/draw-line.svg \
    ../icons/breeze/actions/16/draw-rectangle.svg \
    ../icons/breeze/actions/16/draw-text.svg \
    ../icons/breeze/actions/16/edit-copy.svg \
    ../icons/breeze/actions/16/edit-cut.svg \
    ../icons/breeze/actions/16/edit-delete.svg \
    ../icons/breeze/actions/16/edit-select-all-layers.svg \
    ../icons/breeze/actions/16/edit-select-all.svg \
    ../icons/breeze/actions/16/folder-sync.svg \
    ../icons/breeze/actions/16/hint.svg \
    ../icons/breeze/actions/16/list-add.svg \
    ../icons/breeze/actions/16/list-remove-user.svg \
    ../icons/breeze/actions/16/list-remove.svg \
    ../icons/breeze/actions/16/node.svg \
    ../icons/breeze/actions/16/object-to-path.svg \
    ../icons/breeze/actions/16/path-difference.svg \
    ../icons/breeze/actions/16/path-exclusion.svg \
    ../icons/breeze/actions/16/path-intersection.svg \
    ../icons/breeze/actions/16/path-reverse.svg \
    ../icons/breeze/actions/16/path-union.svg \
    ../icons/breeze/actions/16/roll.svg \
    ../icons/breeze/actions/16/snap-nodes-cusp.svg \
    ../icons/breeze/actions/16/stroke-cap-butt.svg \
    ../icons/breeze/actions/16/stroke-cap-round.svg \
    ../icons/breeze/actions/16/stroke-cap-square.svg \
    ../icons/breeze/actions/16/stroke-to-path.svg \
    ../icons/breeze/actions/16/thermal.svg \
    ../icons/breeze/actions/16/tools-wizard.svg \
    ../icons/breeze/actions/16/view-form-action.svg \
    ../icons/breeze/actions/16/view-form.svg \
    ../icons/breeze/actions/16/window-close.svg \
    ../icons/breeze/actions/22/acrobat.svg \
    ../icons/breeze/actions/22/application-exit.svg \
    ../icons/breeze/actions/22/configure-shortcuts.svg \
    ../icons/breeze/actions/22/document-close.svg \
    ../icons/breeze/actions/22/document-edit.svg \
    ../icons/breeze/actions/22/document-export.svg \
    ../icons/breeze/actions/22/document-open.svg \
    ../icons/breeze/actions/22/document-save-all.svg \
    ../icons/breeze/actions/22/document-save.svg \
    ../icons/breeze/actions/22/draw-ellipse.svg \
    ../icons/breeze/actions/22/draw-line.svg \
    ../icons/breeze/actions/22/draw-rectangle.svg \
    ../icons/breeze/actions/22/draw-text.svg \
    ../icons/breeze/actions/22/edit-copy.svg \
    ../icons/breeze/actions/22/edit-cut.svg \
    ../icons/breeze/actions/22/edit-delete.svg \
    ../icons/breeze/actions/22/edit-select-all.svg \
    ../icons/breeze/actions/22/folder-sync.svg \
    ../icons/breeze/actions/22/hint.svg \
    ../icons/breeze/actions/22/list-add.svg \
    ../icons/breeze/actions/22/list-remove-user.svg \
    ../icons/breeze/actions/22/list-remove.svg \
    ../icons/breeze/actions/22/node.svg \
    ../icons/breeze/actions/22/object-to-path.svg \
    ../icons/breeze/actions/22/path-reverse.svg \
    ../icons/breeze/actions/22/roll.svg \
    ../icons/breeze/actions/22/stroke-cap-butt.svg \
    ../icons/breeze/actions/22/stroke-cap-round.svg \
    ../icons/breeze/actions/22/stroke-cap-square.svg \
    ../icons/breeze/actions/22/stroke-to-path.svg \
    ../icons/breeze/actions/22/thermal.svg \
    ../icons/breeze/actions/22/tools-wizard.svg \
    ../icons/breeze/actions/22/view-form-action.svg \
    ../icons/breeze/actions/22/view-form.svg \
    ../icons/breeze/actions/22/window-close.svg \
    ../icons/breeze/actions/22/zoom-fit-best.svg \
    ../icons/breeze/actions/22/zoom-in.svg \
    ../icons/breeze/actions/22/zoom-original.svg \
    ../icons/breeze/actions/22/zoom-out.svg \
    ../icons/breeze/actions/24/acrobat.svg \
    ../icons/breeze/actions/24/application-exit.svg \
    ../icons/breeze/actions/24/configure-shortcuts.svg \
    ../icons/breeze/actions/24/document-close.svg \
    ../icons/breeze/actions/24/document-edit.svg \
    ../icons/breeze/actions/24/document-export.svg \
    ../icons/breeze/actions/24/document-open.svg \
    ../icons/breeze/actions/24/document-save-all.svg \
    ../icons/breeze/actions/24/document-save.svg \
    ../icons/breeze/actions/24/draw-ellipse.svg \
    ../icons/breeze/actions/24/draw-line.svg \
    ../icons/breeze/actions/24/draw-rectangle.svg \
    ../icons/breeze/actions/24/draw-text.svg \
    ../icons/breeze/actions/24/edit-copy.svg \
    ../icons/breeze/actions/24/edit-cut.svg \
    ../icons/breeze/actions/24/edit-delete.svg \
    ../icons/breeze/actions/24/edit-select-all.svg \
    ../icons/breeze/actions/24/folder-sync.svg \
    ../icons/breeze/actions/24/hint.svg \
    ../icons/breeze/actions/24/list-add.svg \
    ../icons/breeze/actions/24/list-remove-user.svg \
    ../icons/breeze/actions/24/list-remove.svg \
    ../icons/breeze/actions/24/node.svg \
    ../icons/breeze/actions/24/object-to-path.svg \
    ../icons/breeze/actions/24/path-reverse.svg \
    ../icons/breeze/actions/24/roll.svg \
    ../icons/breeze/actions/24/snap-nodes-cusp.svg \
    ../icons/breeze/actions/24/stroke-cap-butt.svg \
    ../icons/breeze/actions/24/stroke-cap-round.svg \
    ../icons/breeze/actions/24/stroke-cap-square.svg \
    ../icons/breeze/actions/24/stroke-to-path.svg \
    ../icons/breeze/actions/24/thermal.svg \
    ../icons/breeze/actions/24/tools-wizard.svg \
    ../icons/breeze/actions/24/view-form-action.svg \
    ../icons/breeze/actions/24/view-form.svg \
    ../icons/breeze/actions/24/window-close.svg \
    ../icons/breeze/actions/24/zoom-fit-best.svg \
    ../icons/breeze/actions/24/zoom-in.svg \
    ../icons/breeze/actions/24/zoom-original.svg \
    ../icons/breeze/actions/24/zoom-out.svg \
    ../icons/breeze/actions/24/zoom-to-selected.svg \
    ../icons/breeze/index.theme \
    ../icons/breeze/places/16/folder.svg \
    ../icons/breeze/places/22/folder.svg \
    ../icons/colors.txt \
    translations/g2g_en.ts \
    translations/g2g_ru.ts\
    G2G_TR.pro

#SUBDIRS += \
#    ../file/file.pri \
#    ../clipper/clipper.pri \
#    ../excellon/excellon.pri \
#    ../gerber/gerber.pri \
#    ../graphicsview/mygraphicsview.pri \
#    ../voronoi/voronoi.pri \

include(../file/file.pri)
include(../clipper/clipper.pri)
include(../excellon/excellon.pri)
include(../gerber/gerber.pri)
include(../graphicsview/mygraphicsview.pri)
include(../voronoi/voronoi.pri)

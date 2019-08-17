QT += core gui opengl widgets printsupport concurrent

TARGET = Getber2Gcode
TEMPLATE = app


RESOURCES += \
    res/resources.qrc \

#DEFINES += QT_DEBUG
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VERSION = 0.7.8
VER_MAJ = 0
VER_MIN = 7
VER_PAT = 8

ICON = 256.png

#macx: ICON = resources/icon.icns

win32-msvc* {
    QMAKE_CXXFLAGS += /std:c++latest
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
}
win32*-gcc* {
    QMAKE_CXXFLAGS += -std=c++1z
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
    sh/circle.h \
    sh/constructor.h \
    sh/handler.h \
    sh/rectangle.h \
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
    sh/circle.cpp \
    sh/constructor.cpp \
    sh/handler.cpp \
    sh/rectangle.cpp \
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
    translations/g2g_en.ts \
    translations/g2g_ru.ts\
    G2G_TR.pro


#    SUBDIRS += \
#        ../file/file.pri \
#        ../clipper/clipper.pri \
#        ../excellon/excellon.pri \
#        ../gerber/gerber.pri \
#        ../graphicsview/mygraphicsview.pri \

include(../clipper/clipper.pri)
include(../excellon/excellon.pri)
include(../file/file.pri)
include(../gcode/gcode.pri)
include(../gerber/gerber.pri)
include(../graphicsview/mygraphicsview.pri)

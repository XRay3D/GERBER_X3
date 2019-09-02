QT += core gui opengl widgets printsupport concurrent

TARGET = Getber2Gcode
TEMPLATE = app

RESOURCES += res/resources.qrc \

#DEFINES += QT_DEBUG
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

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
    datastream.h \
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
    forms/drillform/drillform.h \
    forms/drillform/drillmodel.h \
    forms/drillform/drillpreviewgi.h \
    forms/formsutil.h \
    forms/gcodepropertiesform.h \
    forms/pocketform.h \
    forms/profileform.h \
    forms/thermal/thermalform.h \
    forms/thermal/thermalmodel.h \
    forms/thermal/thermalpreviewitem.h \
    forms/voronoiform.h \
    gi/bridgeitem.h \
    gi/drillitem.h \
    gi/gerberitem.h \
    gi/graphicsitem.h \
    gi/itemgroup.h \
    gi/pathitem.h \
    gi/rawitem.h \
    icons.h \
    mainwindow.h \
    openingdialog.h \
    point.h \
    project.h \
    settings.h \
    settingsdialog.h \
    sh/circle.h \
    sh/constructor.h \
    sh/handler.h \
    sh/rectangle.h \
    sh/sh.h \
    sh/shape.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooleditdialog.h \
    tooldatabase/tooleditform.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    version.h \
    voroni/jc_voronoi.h \


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
    forms/drillform/drillform.cpp \
    forms/drillform/drillmodel.cpp \
    forms/drillform/drillpreviewgi.cpp \
    forms/formsutil.cpp \
    forms/gcodepropertiesform.cpp \
    forms/pocketform.cpp \
    forms/profileform.cpp \
    forms/thermal/thermalform.cpp \
    forms/thermal/thermalmodel.cpp \
    forms/thermal/thermalpreviewitem.cpp \
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
    project.cpp \
    settings.cpp \
    settingsdialog.cpp \
    sh/circle.cpp \
    sh/constructor.cpp \
    sh/handler.cpp \
    sh/rectangle.cpp \
    sh/sh.cpp \
    sh/shape.cpp \
    tooldatabase/tool.cpp \
    tooldatabase/tooldatabase.cpp \
    tooldatabase/tooleditdialog.cpp \
    tooldatabase/tooleditform.cpp \
    tooldatabase/toolitem.cpp \
    tooldatabase/toolmodel.cpp \
    tooldatabase/tooltreeview.cpp \
    voroni/jc_voronoi.cpp \


FORMS += \
    aboutform.ui \
    colorselector.ui \
    forms/drillform/drillform.ui \
    forms/gcodepropertiesform.ui \
    forms/pocketform.ui \
    forms/profileform.ui \
    forms/thermal/thermalform.ui \
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

include(../clipper/clipper.pri)
include(../excellon/excellon.pri)
include(../file/file.pri)
include(../gcode/gcode.pri)
include(../gerber/gerber.pri)
include(../graphicsview/graphicsview.pri)

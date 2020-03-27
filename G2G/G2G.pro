QT += core gui opengl widgets printsupport concurrent

contains(QT_ARCH, i386) {
    message("32-bit")
    TARGET = Getber2Gcode_x32
} else {
    message("64-bit")
    TARGET = Getber2Gcode_x64
}

TEMPLATE = app

RESOURCES += res/resources.qrc

#DEFINES += QT_DEBUG
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

ICON = 256.png

CONFIG += c++17

#macx: ICON = resources/icon.icns

#debug {
#    CONFIG += console
#}

msvc* {
#    QMAKE_CXXFLAGS += /std:c++latest
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
}

gcc* {
#    QMAKE_CXXFLAGS += -std=c++1z
    RC_FILE = myapp.rc
    win32 {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
}

linux {
    DEFINES += linux
    greaterThan(QT_MINOR_VERSION,12){
        LIBS += -ltbb # Why?????
    }
}

DEFINES += "BUILD_DATE=\"\\\"$$_DATE_\\\"\""

DESTDIR = $$_PRO_FILE_PWD_/../bin

INCLUDEPATH += $$PWD/forms/formsutil/

TRANSLATIONS += \
    translations/g2g_en.ts \
    translations/g2g_ru.ts \

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
    forms/drillform/drillform.h \
    forms/drillform/drillmodel.h \
    forms/drillform/drillpreviewgi.h \
    forms/formsutil/depthform.h \
    forms/formsutil/formsutil.h \
    forms/formsutil/toolselectorform.h \
    forms/gcodepropertiesform.h \
    forms/pocketoffsetform.h \
    forms/pocketrasterform.h \
    forms/profileform.h \
    forms/thermal/thermaldelegate.h \
    forms/thermal/thermalform.h \
    forms/thermal/thermalmodel.h \
    forms/thermal/thermalnode.h \
    forms/thermal/thermalpreviewitem.h \
    forms/voronoiform.h \
    gi/aperturepathitem.h \
    gi/bridgeitem.h \
    gi/componentitem.h \
    gi/drillitem.h \
    gi/gerberitem.h \
    gi/graphicsitem.h \
    gi/itemgroup.h \
    gi/pathitem.h \
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
    splashscreen.h \
    tooldatabase/tool.h \
    tooldatabase/tooldatabase.h \
    tooldatabase/tooleditdialog.h \
    tooldatabase/tooleditform.h \
    tooldatabase/toolitem.h \
    tooldatabase/toolmodel.h \
    tooldatabase/tooltreeview.h \
    version.h \

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
    forms/drillform/drillform.cpp \
    forms/drillform/drillmodel.cpp \
    forms/drillform/drillpreviewgi.cpp \
    forms/formsutil/depthform.cpp \
    forms/formsutil/formsutil.cpp \
    forms/formsutil/toolselectorform.cpp \
    forms/gcodepropertiesform.cpp \
    forms/pocketoffsetform.cpp \
    forms/pocketrasterform.cpp \
    forms/profileform.cpp \
    forms/thermal/thermaldelegate.cpp \
    forms/thermal/thermalform.cpp \
    forms/thermal/thermalmodel.cpp \
    forms/thermal/thermalnode.cpp \
    forms/thermal/thermalpreviewitem.cpp \
    forms/voronoiform.cpp \
    gi/aperturepathitem.cpp \
    gi/bridgeitem.cpp \
    gi/componentitem.cpp \
    gi/drillitem.cpp \
    gi/gerberitem.cpp \
    gi/graphicsitem.cpp \
    gi/itemgroup.cpp \
    gi/pathitem.cpp \
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

FORMS += \
    aboutform.ui \
    colorselector.ui \
    forms/drillform/drillform.ui \
    forms/gcodepropertiesform.ui \
    forms/pocketoffsetform.ui \
    forms/pocketrasterform.ui \
    forms/profileform.ui \
    forms/thermal/thermalform.ui \
    forms/voronoiform.ui \
    mainwindow.ui \
    settingsdialog.ui \
    tooldatabase/tooldatabase.ui \
    tooldatabase/tooleditdialog.ui \
    tooldatabase/tooleditform.ui \

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

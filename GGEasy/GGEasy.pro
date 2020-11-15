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
#* Attributions:                                                                *
#* The code in this library is an extension of Bala Vatti's clipping algorithm: *
#* "A generic solution to polygon clipping"                                     *
#* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
#* http://portal.acm.org/citation.cfm?id=129906                                 *
#*                                                                              *
#*******************************************************************************/

QT += core gui opengl widgets printsupport concurrent

TARGET = GGEasy

contains(QT_ARCH, i386) {
    message("32-bit")
    TARGET = $$TARGET"_x32"
} else {
    message("64-bit")
    TARGET = $$TARGET"_x64"
}

TEMPLATE = app

RESOURCES += res/resources.qrc

#DEFINES += QT_DEBUG
DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += "BUILD_DATE=\"\\\"$$_DATE_\\\"\""

ICON = 256.png

#macx: ICON = resources/icon.icns

msvc* {
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
    TARGET = $$TARGET"_msvc"
#    QMAKE_CXXFLAGS -= /std:c++17
    QMAKE_CXXFLAGS += /std:c++latest
    message($$TARGET)
}

gcc* {
    CONFIG += c++17

    RC_FILE = myapp.rc
    win32 {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
    TARGET = $$TARGET"_gcc"
    message($$TARGET)
}

CONFIG(debug, debug|release){
    message("debug")
    TARGET = $$TARGET"_d"
    message($$TARGET)
}

linux {
# sudo apt install mesa-common-dev
    DEFINES += linux
    greaterThan(QT_MINOR_VERSION, 12){
        LIBS += -ltbb # Why????? sudo apt-get install libtbb-dev
    }
}

DESTDIR = $$_PRO_FILE_PWD_/../bin

INCLUDEPATH += $$PWD/forms/formsutil/
INCLUDEPATH += $$PWD/../magic_get-1.0.4/include/

TRANSLATIONS += \
    translations/GGEasy_en.ts \
    translations/GGEasy_ru.ts \

HEADERS += \
    aboutform.h \
    app.h \
    application.h \
    colorselector.h \
    datastream.h \
    doublespinbox.h \
    filetree/filemodel.h \
    filetree/foldernode.h \
    filetree/layerdelegate.h \
    filetree/treeview.h \
    forms/drillform/drillform.h \
    forms/drillform/drillmodel.h \
    forms/drillform/drillpreviewgi.h \
    forms/formsutil/depthform.h \
    forms/formsutil/errordialog.h \
    forms/formsutil/formsutil.h \
    forms/formsutil/toolselectorform.h \
    forms/gcodepropertiesform.h \
    forms/pocketoffsetform.h \
    forms/pocketrasterform.h \
    forms/profileform.h \
    forms/voronoiform.h \
    gi/aperturepathitem.h \
    gi/bridgeitem.h \
    gi/componentitem.h \
    gi/drillitem.h \
    gi/erroritem.h \
    gi/gerberitem.h \
    gi/graphicsitem.h \
    gi/itemgroup.h \
    gi/pathitem.h \
    leakdetector.h \
    mainwindow.h \
    openingdialog.h \
    point.h \
    project.h \
    recent.h \
    settings.h \
    settingsdialog.h \
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
    filetree/filemodel.cpp \
    filetree/foldernode.cpp \
    filetree/layerdelegate.cpp \
    filetree/treeview.cpp \
    forms/drillform/drillform.cpp \
    forms/drillform/drillmodel.cpp \
    forms/drillform/drillpreviewgi.cpp \
    forms/formsutil/depthform.cpp \
    forms/formsutil/errordialog.cpp \
    forms/formsutil/formsutil.cpp \
    forms/formsutil/toolselectorform.cpp \
    forms/gcodepropertiesform.cpp \
    forms/pocketoffsetform.cpp \
    forms/pocketrasterform.cpp \
    forms/profileform.cpp \
    forms/voronoiform.cpp \
    gi/aperturepathitem.cpp \
    gi/bridgeitem.cpp \
    gi/componentitem.cpp \
    gi/drillitem.cpp \
    gi/erroritem.cpp \
    gi/gerberitem.cpp \
    gi/graphicsitem.cpp \
    gi/itemgroup.cpp \
    gi/pathitem.cpp \
    main.cpp \
    mainwindow.cpp \
    point.cpp \
    project.cpp \
    recent.cpp \
    settings.cpp \
    settingsdialog.cpp \
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
    forms/formsutil/errordialog.ui \
    forms/gcodepropertiesform.ui \
    forms/pocketoffsetform.ui \
    forms/pocketrasterform.ui \
    forms/profileform.ui \
    forms/voronoiform.ui \
    mainwindow.ui \
    settingsdialog.ui \
    tooldatabase/tooldatabase.ui \
    tooldatabase/tooleditdialog.ui \
    tooldatabase/tooleditform.ui \


include(../clipper/clipper.pri)
include(../excellon/excellon.pri)
include(../file/file.pri)
include(../gcode/gcode.pri)
include(../gerber/gerber.pri)
include(../graphicsview/graphicsview.pri)
include(../shapes/shapes.pri)
include(../thermal/thermal.pri)

#pvs_studio.target = pvs
#pvs_studio.output = true
#pvs_studio.cxxflags = -std=c++17
#pvs_studio.sources = $${SOURCES}
#include(../PVS-Studio.pri)

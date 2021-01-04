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

TEMPLATE = app

RESOURCES += res/resources.qrc

ICON = 256.png

#macx: ICON = resources/icon.icns

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
DEFINES += "BUILD_DATE=\"\\\"$$_DATE_\\\"\""

SUFIX = ""

contains(QT_ARCH, i386) {
    SUFIX = "_x32"
} else {
    SUFIX = "_x64"
}
msvc* {
    SUFIX = $$SUFIX"_msvc"
}
gcc* {
    SUFIX = $$SUFIX"_gcc"
}
CONFIG(debug, debug|release){
    SUFIX = $$SUFIX"_d"
}

TARGET = $$TARGET$$SUFIX

message($$TARGET)

msvc* {
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
    QMAKE_CXXFLAGS += /std:c++latest
    #DEFINES += LEAK_DETECTOR
    LIBS += -l$$_PRO_FILE_PWD_/../lib/clipper$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/filetree$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/gi$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/graphicsview$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/settings$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/project$$SUFIX
}

gcc* {
    CONFIG += c++17
    #QMAKE_CXXFLAGS += -std=c++17
    RC_FILE = myapp.rc
    win* {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
    LIBS += "-L"$$_PRO_FILE_PWD_/../lib
    LIBS += -lclipper$$SUFIX
    LIBS += -lfiletree$$SUFIX
    LIBS += -lgi$$SUFIX
    LIBS += -lgraphicsview$$SUFIX
    LIBS += -lsettings$$SUFIX
    LIBS += -lproject$$SUFIX
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
#INCLUDEPATH += $$PWD/../magic_get-1.0.4/include/
INCLUDEPATH += ../clipper
INCLUDEPATH += ../filetree
INCLUDEPATH += ../gerber
INCLUDEPATH += ../gi
INCLUDEPATH += ../settings
INCLUDEPATH += ../project

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
    forms/bridgeitem.h \
    gcode/gccreator.h \
    gcode/gcfile.h \
    gcode/gch.h \
    gcode/gcnode.h \
    gcode/gcode.h \
    gcode/gcparser.h \
    gcode/gcpocketoffset.h \
    gcode/gcpocketraster.h \
    gcode/gcprofile.h \
    gcode/gcthermal.h \
    gcode/gctypes.h \
    gcode/gcutils.h \
    gcode/gcvoronoi.h \
    gcode/voroni/jc_voronoi.h \
    interfaces/file.h \
    interfaces/node.h \
    interfaces/parser.h \
    leakdetector.h \
    mainwindow.h \
    openingdialog.h \
    point.h \
    recent.h \
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
    forms/bridgeitem.cpp \
    gcode/gccreator.cpp \
    gcode/gcfile.cpp \
    gcode/gch.cpp \
    gcode/gcnode.cpp \
    gcode/gcparser.cpp \
    gcode/gcpocketoffset.cpp \
    gcode/gcpocketraster.cpp \
    gcode/gcprofile.cpp \
    gcode/gcthermal.cpp \
    gcode/gcutils.cpp \
    gcode/gcvoronoi.cpp \
    gcode/voroni/jc_voronoi.cpp \
    interfaces/file.cpp \
    interfaces/node.cpp \
    interfaces/parser.cpp \
    main.cpp \
    mainwindow.cpp \
    point.cpp \
    recent.cpp \
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


#include(../dxf/dxf.pri)
#include(../excellon/excellon.pri)
#include(../file/file.pri)
#include(../gcode/gcode.pri)
#include(../shapes/shapes.pri)
#include(../thermal/thermal.pri)
include(../clipper/clipper.pri)
include(../filetree/filetree.pri)
#include(../gerber/gerber.pri)
include(../gi/gi.pri)
include(../graphicsview/graphicsview.pri)
include(../project/project.pri)

#pvs_studio.target = pvs
#pvs_studio.output = true
#pvs_studio.cxxflags = -std=c++17
#pvs_studio.sources = $${SOURCES}
#include(../PVS-Studio.pri)

DISTFILES += \
    ChangeLog.txt

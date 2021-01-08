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

include(../defines.pri)
include(../suffix.pri)

TEMPLATE = app

RESOURCES += res/resources.qrc

ICON = 256.png
#macx: ICON = resources/icon.icns

TARGET = $$TARGET$$SUFFIX

message($$TARGET)

DESTDIR = $$_PRO_FILE_PWD_/../bin

msvc* {
    LIBS += -lsetupapi -lAdvapi32
    RC_FILE = myapp.rc
    QMAKE_CXXFLAGS += /std:c++latest
    #DEFINES += LEAK_DETECTOR
    LIBS += -l$$_PRO_FILE_PWD_/../lib/clipper$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/filetree$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/gi$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/graphicsview$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/project$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/settings$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/tooldatabase$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/thermal$$SUFFIX
}

gcc* {
    CONFIG += c++17
    #QMAKE_CXXFLAGS += -std=c++17
    RC_FILE = myapp.rc
    win* {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
    LIBS += "-L"$$_PRO_FILE_PWD_/../lib
    LIBS += -lclipper$$SUFFIX
    LIBS += -lfiletree$$SUFFIX
    LIBS += -lgi$$SUFFIX
    LIBS += -lgraphicsview$$SUFFIX
    LIBS += -lproject$$SUFFIX
    LIBS += -lsettings$$SUFFIX
    LIBS += -ltooldatabase$$SUFFIX
    LIBS += -lthermal$$SUFFIX
}

linux {
    # sudo apt install mesa-common-dev
    DEFINES += linux
    greaterThan(QT_MINOR_VERSION, 12){
        LIBS += -ltbb # Why????? sudo apt-get install libtbb-dev
    }
}


#INCLUDEPATH += $$PWD/forms/formsutil/
#INCLUDEPATH += $$PWD/../magic_get-1.0.4/include/
INCLUDEPATH += ../clipper
INCLUDEPATH += ../filetree
INCLUDEPATH += ../gerber
INCLUDEPATH += ../gi
INCLUDEPATH += ../graphicsview
INCLUDEPATH += ../project
INCLUDEPATH += ../settings
INCLUDEPATH += ../thermal
INCLUDEPATH += ../tooldatabase

TRANSLATIONS += \
    translations/GGEasy_en.ts \
    translations/GGEasy_ru.ts \

HEADERS += \
    aboutform.h \
    app.h \
    application.h \
    colorselector.h \
    datastream.h \
    depthform.h \
    doublespinbox.h \
    forms/drillform/drillform.h \
    forms/drillform/drillmodel.h \
    forms/formsutil/errordialog.h \
    forms/formsutil/formsutil.h \
    forms/gcodepropertiesform.h \
    forms/pocketoffsetform.h \
    forms/pocketrasterform.h \
    forms/profileform.h \
    forms/voronoiform.h \
    gcode/gccreator.h \
    gcode/gcdrillitem.h \
    gcode/gcfile.h \
    gcode/gch.h \
    gcode/gcnode.h \
    gcode/gcode.h \
    gcode/gcplugin.h \
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
    interfaces/pluginfile.h \
    interfaces/plugintypes.h \
    leakdetector.h \
    mainwindow.h \
    mvector.h \
    openingdialog.h \
    plugindialog.h \
    point.h \
    recent.h \
    settingsdialog.h \
    splashscreen.h \
    toolselectorform.h \
    version.h \

SOURCES += \
    aboutform.cpp \
    colorselector.cpp \
    depthform.cpp \
    doublespinbox.cpp \
    forms/drillform/drillform.cpp \
    forms/drillform/drillmodel.cpp \
    forms/formsutil/errordialog.cpp \
    forms/formsutil/formsutil.cpp \
    forms/gcodepropertiesform.cpp \
    forms/pocketoffsetform.cpp \
    forms/pocketrasterform.cpp \
    forms/profileform.cpp \
    forms/voronoiform.cpp \
    gcode/gccreator.cpp \
    gcode/gcdrillitem.cpp \
    gcode/gcfile.cpp \
    gcode/gch.cpp \
    gcode/gcnode.cpp \
    gcode/gcplugin.cpp \
    gcode/gcpocketoffset.cpp \
    gcode/gcpocketraster.cpp \
    gcode/gcprofile.cpp \
    gcode/gcthermal.cpp \
    gcode/gcutils.cpp \
    gcode/gcvoronoi.cpp \
    gcode/voroni/jc_voronoi.cpp \
    main.cpp \
    mainwindow.cpp \
    plugindialog.cpp \
    point.cpp \
    recent.cpp \
    settingsdialog.cpp \
    toolselectorform.cpp \

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

#include(../dxf/dxf.pri)
#include(../excellon/excellon.pri)
#include(../file/file.pri)
#include(../gcode/gcode.pri)
#include(../gerber/gerber.pri)
#include(../shapes/shapes.pri)

#include(../clipper/clipper.pri)
#include(../filetree/filetree.pri)
#include(../gi/gi.pri)
#include(../graphicsview/graphicsview.pri)
#include(../project/project.pri)
#include(../thermal/thermal.pri)
#include(../tooldatabase/tooldatabase.pri)

#pvs_studio.target = pvs
#pvs_studio.output = true
#pvs_studio.cxxflags = -std=c++17
#pvs_studio.sources = $${SOURCES}
#include(../PVS-Studio.pri)

DISTFILES += \
    ChangeLog.txt

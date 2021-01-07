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
QT += concurrent

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
    LIBS += -l$$_PRO_FILE_PWD_/../lib/tooldatabase$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/thermal$$SUFFIX
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

INCLUDEPATH += ../GGEasy
INCLUDEPATH += ../clipper
INCLUDEPATH += ../settings
INCLUDEPATH += ../graphicsview
INCLUDEPATH += ../gi
INCLUDEPATH += ../filetree
INCLUDEPATH += ../project
INCLUDEPATH += ../tooldatabase
INCLUDEPATH += ../thermal

EXAMPLE_FILES = gerber.json

HEADERS += \
    compdialog.h \
    compmodel.h \
    compnode.h \
    componentitem.h \
    compview.h \
    gbrattraperfunction.h \
    gbrattrfilefunction.h \
    gbrattributes.h \
    gbrcomponent.h \
    gbrnode.h \
    gbrparser.h \
    gbrplugin.h \
    gbrtypes.h \
    mathparser.h \
    gbraperture.h \
    gbrfile.h

SOURCES += \
    compdialog.cpp \
    compmodel.cpp \
    compnode.cpp \
    componentitem.cpp \
    compview.cpp \
    gbraperture.cpp \
    gbrattraperfunction.cpp \
    gbrattrfilefunction.cpp \
    gbrattributes.cpp \
    gbrcomponent.cpp \
    gbrfile.cpp \
    gbrnode.cpp \
    gbrparser.cpp \
    gbrplugin.cpp \
    mathparser.cpp \

FORMS += \
    compdialog.ui

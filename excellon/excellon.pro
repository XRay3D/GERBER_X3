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
    LIBS += -l$$_PRO_FILE_PWD_/../lib/tooldatabase$$SUFFIX
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

EXAMPLE_FILES = gerber.json

FORMS += \
    excellondialog.ui

HEADERS += \
    drillitem.h \
    excellon.h \
    excellondialog.h \
    exfile.h \
    exnode.h \
    exparser.h \
    explugin.h \
    extypes.h

SOURCES += \
    drillitem.cpp \
    excellondialog.cpp \
    exfile.cpp \
    exformatstate.cpp \
    exnode.cpp \
    exparser.cpp \
    explugin.cpp

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
include(../staticlib.pri)
include(../defines.pri)
include(../suffix.pri)

TARGET = $$TARGET$$SUFFIX

message($$TARGET)

msvc* {
    LIBS += -lsetupapi -lAdvapi32
    QMAKE_CXXFLAGS += /std:c++latest
    #DEFINES += LEAK_DETECTOR
    LIBS += -l$$_PRO_FILE_PWD_/../lib/project$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/settings$$SUFFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/tooldatabase$$SUFFIX
}

gcc* {
    CONFIG += c++17
    win* {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
    LIBS += "-L"$$_PRO_FILE_PWD_/../lib
    LIBS += -lproject$$SUFFIX
    LIBS += -lsettings$$SUFFIX
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
INCLUDEPATH += ../settings
INCLUDEPATH += ../clipper
INCLUDEPATH += ../graphicsview
INCLUDEPATH += ../filetree
INCLUDEPATH += ../project
INCLUDEPATH += ../tooldatabase

SOURCES += \
    componentitem.cpp \
    datapathitem.cpp \
    datasoliditem.cpp \
    drillitem.cpp \
    drillpreviewgi.cpp \
    erroritem.cpp \
    gcpathitem.cpp \
    graphicsitem.cpp \
    itemgroup.cpp \

HEADERS += \
    componentitem.h \
    datapathitem.h \
    datasoliditem.h \
    drillitem.h \
    drillpreviewgi.h \
    erroritem.h \
    gcpathitem.h \
    graphicsitem.h \
    itemgroup.h \

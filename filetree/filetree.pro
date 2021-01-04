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

QT += gui widgets

TEMPLATE = lib
CONFIG += staticlib

DESTDIR = $$_PRO_FILE_PWD_/../lib

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
    QMAKE_CXXFLAGS += /std:c++latest
    #DEFINES += LEAK_DETECTOR
    LIBS += -l$$_PRO_FILE_PWD_/../lib/project$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/settings$$SUFIX
}

gcc* {
    CONFIG += c++17
    win* {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
    LIBS += "-L"$$_PRO_FILE_PWD_/../lib
    LIBS += -lproject$$SUFIX
    LIBS += -lsettings$$SUFIX
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
INCLUDEPATH += ../gi
INCLUDEPATH += ../graphicsview
INCLUDEPATH += ../settings
INCLUDEPATH += ../project

#OBJECTS += $$_PRO_FILE_PWD_/../project/moc_project.obj

HEADERS += \
    filemodel.h \
    foldernode.h \
#    radiodelegate.h \
    sidedelegate.h \
    textdelegate.h \
    treeview.h \
    typedelegate.h


SOURCES += \
    filemodel.cpp \
    foldernode.cpp \
#    radiodelegate.cpp \
    sidedelegate.cpp \
    textdelegate.cpp \
    treeview.cpp \
    typedelegate.cpp


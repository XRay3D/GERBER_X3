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
INCLUDEPATH += ../settings
INCLUDEPATH += ../clipper
INCLUDEPATH += ../graphicsview
INCLUDEPATH += ../filetree
INCLUDEPATH += ../project

SOURCES += \
    componentitem.cpp \
    datapathitem.cpp \
    datasoliditem.cpp \
    erroritem.cpp \
    gcpathitem.cpp \
    graphicsitem.cpp \
    itemgroup.cpp \

HEADERS += \
    componentitem.h \
    datapathitem.h \
    datasoliditem.h \
    erroritem.h \
    gcpathitem.h \
    graphicsitem.h \
    itemgroup.h \

# Default rules for deployment.
unix {
    target.path = $$[QT_INSTALL_PLUGINS]/generic
}
!isEmpty(target.path): INSTALLS += target

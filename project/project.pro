TEMPLATE    = lib
CONFIG      += staticlib
QT          += widgets

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
    LIBS += -l$$_PRO_FILE_PWD_/../lib/clipper$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/settings$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/graphicsview$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/gi$$SUFIX
    LIBS += -l$$_PRO_FILE_PWD_/../lib/filetree$$SUFIX
}

gcc* {
    CONFIG += c++17
    win* {
        LIBS += -lsetupapi -lAdvapi32 -lpsapi
    }
    LIBS += "-L"$$_PRO_FILE_PWD_/../lib
    LIBS += -lclipper$$SUFIX
    LIBS += -lsettings$$SUFIX
    LIBS += -lgraphicsview$$SUFIX
    LIBS += -lgi$$SUFIX
    LIBS += -lfiletree$$SUFIX
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

#OBJECTS_DIR = $$_PRO_FILE_PWD_

EXAMPLE_FILES = gerber.json

# install
#target.path = $$[QT_INSTALL_EXAMPLES]/widgets/tools/echoplugin/plugins
#INSTALLS += target

#CONFIG += install_ok  # Do not cargo-cult this!

HEADERS += \
    project.h \

SOURCES += \
    project.cpp \

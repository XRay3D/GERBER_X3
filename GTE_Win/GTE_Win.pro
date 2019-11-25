QT += widgets winextras #xml

TARGET = GerberThumbnailExtension
TEMPLATE = lib

include(clipper/clipper.pri)
include(gerber/gerber.pri)

#DEFINES += _WIN32 WIN32 NDEBUG _WINDOWS _USRDLL CPPSHELLEXTTHUMBNAILHANDLER_EXPORTS _WINDLL _UNICODE UNICODE

#win32-msvc* {
#    QMAKE_CXXFLAGS += /std:c++latest /MAP /Zi /debug /opt:ref
#}

win32:LIBS += \
    shlwapi.lib \
    advapi32.lib \
    gdiplus.lib \
    shell32.lib \
    ole32.lib \


DISTFILES += \
    GlobalExportFunctions.def \
    ReadMe.txt

DEF_FILE += \
    GlobalExportFunctions.def

HEADERS += \
    ClassFactory.h \
    Reg.h \
    gerberthumbnailprovider.h \
    gerberthumbnailprovider.h \

SOURCES += \
    ClassFactory.cpp \
    dllmain.cpp \
    Reg.cpp \
    gerberthumbnailprovider.cpp \

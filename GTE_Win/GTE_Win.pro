QT += widgets xml winextras

TARGET = GerberThumbnailExtension
TEMPLATE = lib

CONFIG(release, debug|release):DEFINES += NDEBUG
include(../clipper/clipper.pri)
include(../gerber/gerber.pri)
#include(../file/file.pri)

DEFINES += _WIN32 WIN32 NDEBUG _WINDOWS _USRDLL CPPSHELLEXTTHUMBNAILHANDLER_EXPORTS _WINDLL _UNICODE UNICODE

win32:LIBS += \
    shlwapi.lib \
    advapi32.lib \
    gdiplus.lib \
#    kernel32.lib \
#    user32.lib \
#    gdi32.lib \
#    winspool.lib \
#    comdlg32.lib \
#    advapi32.lib \
    shell32.lib \
    ole32.lib \
#    oleaut32.lib \
#    uuid.lib \
#    odbc32.lib \
#    odbccp32.lib

DISTFILES += \
    GlobalExportFunctions.def \
    ReadMe.txt

DEF_FILE += \
    GlobalExportFunctions.def

#OTHER_FILES += \
#    GlobalExportFunctions.def

HEADERS += \
    ClassFactory.h \
    Reg.h \
    gerberthumbnailprovider.h \
    gerberthumbnailprovider.h \
#    ../G2G/clipper/clipper.hpp \
#    ../G2G/clipper/myclipper.h \
#    ../G2G/gerber/gerber.h \
#    ../G2G/gerber/gerberaperture.h \
#    ../G2G/gerber/gerberparser.h \
#    ../G2G/gerber/mathparser.h \
#    ../G2G/toolpathcreator.h
    ../G2G/gi/drillitem.h \
    ../G2G/gi/gerberitem.h \
    ../G2G/gi/graphicsitem.h \
    ../G2G/gi/itemgroup.h \
    ../G2G/gi/pathitem.h \
    ../G2G/gi/rawitem.h

SOURCES += \
    ClassFactory.cpp \
    dllmain.cpp \
    Reg.cpp \
    gerberthumbnailprovider.cpp \
#    ../G2G/clipper/clipper.cpp \
#    ../G2G/clipper/myclipper.cpp \
#    ../G2G/gerber/gerberaperture.cpp \
#    ../G2G/gerber/gerberparser.cpp \
#    ../G2G/gerber/mathparser.cpp \
#    ../G2G/toolpathcreator.cpp
    ../G2G/gi/drillitem.cpp \
    ../G2G/gi/gerberitem.cpp \
    ../G2G/gi/graphicsitem.cpp \
    ../G2G/gi/itemgroup.cpp \
    ../G2G/gi/pathitem.cpp \
    ../G2G/gi/rawitem.cpp

#########################################


#symbian {
#    MMP_RULES += EXPORTUNFROZEN
#    TARGET.UID3 = 0xE6F54BF5
#    TARGET.CAPABILITY =
#    TARGET.EPOCALLOWDLLDATA = 1
#    addFiles.sources = CppShellExtThumbnailHandler.dll
#    addFiles.path = !:/sys/bin
#    DEPLOYMENT += addFiles
#}

#unix:!symbian {
#    maemo5 {
#        target.path = /opt/usr/lib
#    } else {
#        target.path = /usr/lib
#    }
#    INSTALLS += target
#}

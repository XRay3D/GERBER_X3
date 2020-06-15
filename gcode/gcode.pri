# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INCLUDEPATH += $$PWD

contains(QT_ARCH, i386) {
    message("32-bit")
} else {
    message("64-bit")
    msvc* {
        LIBS += \
            -lC:/dev/CGAL-5.0/auxiliary/gmp/lib/libmpfr-4 \
            -lC:/dev/CGAL-5.0/auxiliary/gmp/lib/libgmp-10
        INCLUDEPATH += C:/local/boost_1_71_0
        INCLUDEPATH += C:/dev/CGAL-5.0/include
        INCLUDEPATH += C:/dev/CGAL-5.0/auxiliary/gmp/include
        DEFINES += _USE_CGAL_
    }
}



HEADERS += \
    $$PWD/gccreator.h \
    $$PWD/gcfile.h \
    $$PWD/gcnode.h \
    $$PWD/gcpocketoffset.h \
    $$PWD/gcpocketraster.h \
    $$PWD/gcprofile.h \
    $$PWD/gcthermal.h \
    $$PWD/gctypes.h \
    $$PWD/gcutils.h \
    $$PWD/gcvoronoi.h \
    $$PWD/voroni/jc_voronoi.h \

SOURCES += \
    $$PWD/gccreator.cpp \
    $$PWD/gcfile.cpp \
    $$PWD/gcnode.cpp \
    $$PWD/gcpocketoffset.cpp \
    $$PWD/gcpocketraster.cpp \
    $$PWD/gcprofile.cpp \
    $$PWD/gcthermal.cpp \
    $$PWD/gcutils.cpp \
    $$PWD/gcvoronoi.cpp \
    $$PWD/voroni/jc_voronoi.cpp \

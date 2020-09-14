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
    #message("32-bit")
} else {
    #message("64-bit")
    msvc* {
        LIBS += \
            -lC:/dev/CGAL-5.1/auxiliary/gmp/lib/libmpfr-4 \
            -lC:/dev/CGAL-5.1/auxiliary/gmp/lib/libgmp-10
        INCLUDEPATH += C:/local/boost_1_71_0
        INCLUDEPATH += C:/dev/CGAL-5.1/include
        INCLUDEPATH += C:/dev/CGAL-5.1/auxiliary/gmp/include
        DEFINES += _USE_CGAL_
    }
}

HEADERS += \
    $$PWD/compdialog.h \
    $$PWD/compmodel.h \
    $$PWD/compnode.h \
    $$PWD/compview.h \
    $$PWD/gbrapmacro.h \
    $$PWD/gbrattributes.h \
    $$PWD/gbrcomponent.h \
    $$PWD/gbrnode.h \
    $$PWD/gbrtypes.h \
    $$PWD/mathparser.h \
    $$PWD/gbraperture.h \
    $$PWD/gbrfile.h \
    $$PWD/gbrparser.h

SOURCES += \
    $$PWD/compdialog.cpp \
    $$PWD/compmodel.cpp \
    $$PWD/compnode.cpp \
    $$PWD/compview.cpp \
    $$PWD/gbrapmacro.cpp \
    $$PWD/gbrattributes.cpp \
    $$PWD/gbrcomponent.cpp \
    $$PWD/gbrnode.cpp \
    $$PWD/mathparser.cpp \
    $$PWD/gbrparser.cpp \
    $$PWD/gbrfile.cpp \
    $$PWD/gbraperture.cpp

FORMS += \
    $$PWD/compdialog.ui

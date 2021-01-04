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
INCLUDEPATH += $$PWD

DEFINES += GERBER

HEADERS += \
    $$PWD/compdialog.h \
    $$PWD/compmodel.h \
    $$PWD/compnode.h \
    $$PWD/compview.h \
    $$PWD/gbrattrfilefunction.h \
    $$PWD/gbrattributes.h \
    $$PWD/gbrcomponent.h \
    $$PWD/gbrnode.h \
    $$PWD/gbrtypes.h \
    $$PWD/mathparser.h \
    $$PWD/gbraperture.h \
    $$PWD/gbrfile.h \
    $$PWD/gbrparser.h

#SOURCES += \
#    $$PWD/compdialog.cpp \
#    $$PWD/compmodel.cpp \
#    $$PWD/compnode.cpp \
#    $$PWD/compview.cpp \
#    $$PWD/gbrattrfilefunction.cpp \
#    $$PWD/gbrattributes.cpp \
#    $$PWD/gbrcomponent.cpp \
#    $$PWD/gbrnode.cpp \
#    $$PWD/mathparser.cpp \
#    $$PWD/gbrparser.cpp \
#    $$PWD/gbrfile.cpp \
#    $$PWD/gbraperture.cpp

FORMS += \
    $$PWD/compdialog.ui

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
#* Attributions:                                                                *
#* The code in this library is an extension of Bala Vatti's clipping algorithm: *
#* "A generic solution to polygon clipping"                                     *
#* Communications of the ACM, Vol 35, Issue 7 (July 1992) pp 56-63.             *
#* http://portal.acm.org/citation.cfm?id=129906                                 *
#*                                                                              *
#*******************************************************************************/

#QT += core gui opengl widgets printsupport concurrent
#TARGET = GGEasy

#contains(QT_ARCH, i386) {
#    message("32-bit")
#    TARGET = $$TARGET"_x32"
#} else {
#    message("64-bit")
#    TARGET = $$TARGET"_x64"
#}

#TEMPLATE = app

#RESOURCES += res/resources.qrc

#DEFINES += QT_DEBUG
#DEFINES += QT_DEPRECATED_WARNINGS
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
#DEFINES += "BUILD_DATE=\"\\\"$$_DATE_\\\"\""

#ICON = 256.png

#macx: ICON = resources/icon.icns

#msvc* {
#    LIBS += -lsetupapi -lAdvapi32
#    RC_FILE = myapp.rc
#    TARGET = $$TARGET"_msvc"
#    QMAKE_CXXFLAGS += /std:c++latest
#    message($$TARGET)
##    DEFINES += LEAK_DETECTOR
#}

#gcc* {
#    CONFIG += c++17
#    #QMAKE_CXXFLAGS += -std=c++17
#    RC_FILE = myapp.rc
#    win* {
#        LIBS += -lsetupapi -lAdvapi32 -lpsapi
#    }
#    TARGET = $$TARGET"_gcc"
#    message($$TARGET)
#}

#CONFIG(debug, debug|release){
#    message("debug")
#    TARGET = $$TARGET"_d"
#    message($$TARGET)
#}

#linux {
## sudo apt install mesa-common-dev
#    DEFINES += linux
#    greaterThan(QT_MINOR_VERSION, 12){
#        LIBS += -ltbb # Why????? sudo apt-get install libtbb-dev
#    }
#}

#DESTDIR = $$_PRO_FILE_PWD_/../bin

#INCLUDEPATH += $$PWD/forms/formsutil/
#INCLUDEPATH += $$PWD/../magic_get-1.0.4/include/

TRANSLATIONS += \
#    $$PWD/translations/GGEasy_en.ts \
#    $$PWD/translations/GGEasy_ru.ts \

HEADERS += \
#    $$PWD/aboutform.h \
    $$PWD/app.h \
#    $$PWD/application.h \
#    $$PWD/colorselector.h \
#    $$PWD/datastream.h \
#    $$PWD/doublespinbox.h \
#    $$PWD/filetree/filemodel.h \
#    $$PWD/filetree/foldernode.h \
#    $$PWD/filetree/radiodelegate.h \
#    $$PWD/filetree/sidedelegate.h \
#    $$PWD/filetree/textdelegate.h \
#    $$PWD/filetree/treeview.h \
#    $$PWD/filetree/typedelegate.h \
#    $$PWD/forms/drillform/drillform.h \
#    $$PWD/forms/drillform/drillmodel.h \
#    $$PWD/forms/drillform/drillpreviewgi.h \
#    $$PWD/forms/formsutil/depthform.h \
#    $$PWD/forms/formsutil/errordialog.h \
#    $$PWD/forms/formsutil/formsutil.h \
#    $$PWD/forms/formsutil/toolselectorform.h \
#    $$PWD/forms/gcodepropertiesform.h \
#    $$PWD/forms/pocketoffsetform.h \
#    $$PWD/forms/pocketrasterform.h \
#    $$PWD/forms/profileform.h \
#    $$PWD/forms/voronoiform.h \
#    $$PWD/gi/bridgeitem.h \
#    $$PWD/gi/datapathitem.h \
#    $$PWD/gi/datasoliditem.h \
#    $$PWD/gi/drillitem.h \
#    $$PWD/gi/erroritem.h \
#    $$PWD/gi/gcpathitem.h \
    $$PWD/gi/graphicsitem.h \
#    $$PWD/gi/itemgroup.h \
#    $$PWD/leakdetector.h \
#    $$PWD/mainwindow.h \
#    $$PWD/openingdialog.h \
#    $$PWD/point.h \
#    $$PWD/project.h \
#    $$PWD/recent.h \
    $$PWD/settings.h \
#    $$PWD/settingsdialog.h \
#    $$PWD/splashscreen.h \
#    $$PWD/tooldatabase/tool.h \
#    $$PWD/tooldatabase/tooldatabase.h \
#    $$PWD/tooldatabase/tooleditdialog.h \
#    $$PWD/tooldatabase/tooleditform.h \
#    $$PWD/tooldatabase/toolitem.h \
#    $$PWD/tooldatabase/toolmodel.h \
#    $$PWD/tooldatabase/tooltreeview.h \
#    $$PWD/version.h \

SOURCES += \
#    $$PWD/aboutform.cpp \
#    $$PWD/colorselector.cpp \
#    $$PWD/doublespinbox.cpp \
#    $$PWD/filetree/filemodel.cpp \
#    $$PWD/filetree/foldernode.cpp \
#    $$PWD/filetree/radiodelegate.cpp \
#    $$PWD/filetree/sidedelegate.cpp \
#    $$PWD/filetree/textdelegate.cpp \
#    $$PWD/filetree/treeview.cpp \
#    $$PWD/filetree/typedelegate.cpp \
#    $$PWD/forms/drillform/drillform.cpp \
#    $$PWD/forms/drillform/drillmodel.cpp \
#    $$PWD/forms/drillform/drillpreviewgi.cpp \
#    $$PWD/forms/formsutil/depthform.cpp \
#    $$PWD/forms/formsutil/errordialog.cpp \
#    $$PWD/forms/formsutil/formsutil.cpp \
#    $$PWD/forms/formsutil/toolselectorform.cpp \
#    $$PWD/forms/gcodepropertiesform.cpp \
#    $$PWD/forms/pocketoffsetform.cpp \
#    $$PWD/forms/pocketrasterform.cpp \
#    $$PWD/forms/profileform.cpp \
#    $$PWD/forms/voronoiform.cpp \
#    $$PWD/gi/bridgeitem.cpp \
#    $$PWD/gi/datapathitem.cpp \
#    $$PWD/gi/datasoliditem.cpp \
#    $$PWD/gi/drillitem.cpp \
#    $$PWD/gi/erroritem.cpp \
    $$PWD/gi/gcpathitem.cpp \
#    $$PWD/gi/graphicsitem.cpp \
#    $$PWD/gi/itemgroup.cpp \
#    $$PWD/main.cpp \
#    $$PWD/mainwindow.cpp \
#    $$PWD/point.cpp \
#    $$PWD/project.cpp \
#    $$PWD/recent.cpp \
    $$PWD/settings.cpp \
#    $$PWD/settingsdialog.cpp \
#    $$PWD/tooldatabase/tool.cpp \
#    $$PWD/tooldatabase/tooldatabase.cpp \
#    $$PWD/tooldatabase/tooleditdialog.cpp \
#    $$PWD/tooldatabase/tooleditform.cpp \
#    $$PWD/tooldatabase/toolitem.cpp \
#    $$PWD/tooldatabase/toolmodel.cpp \
#    $$PWD/tooldatabase/tooltreeview.cpp \

FORMS += \
#    $$PWD/aboutform.ui \
#    $$PWD/colorselector.ui \
#    $$PWD/forms/drillform/drillform.ui \
#    $$PWD/forms/formsutil/errordialog.ui \
#    $$PWD/forms/gcodepropertiesform.ui \
#    $$PWD/forms/pocketoffsetform.ui \
#    $$PWD/forms/pocketrasterform.ui \
#    $$PWD/forms/profileform.ui \
#    $$PWD/forms/voronoiform.ui \
#    $$PWD/mainwindow.ui \
#    $$PWD/settingsdialog.ui \
#    $$PWD/tooldatabase/tooldatabase.ui \
#    $$PWD/tooldatabase/tooleditdialog.ui \
#    $$PWD/tooldatabase/tooleditform.ui \



// #include "a_pch.h"

#include "app.h"
#include "graphicsview.h"
#include "mainwindow.h"
#include "md5.h"

//// #include <QAction>
//// #include <QDockWidget>
#include <QDir>
#include <QTimer>
#include <QtWidgets>

bool MainWindow::debug() {

    while (App::isDebug()) { // NOTE need for debug
        int i = 100;
        int k = 100;

        if (0) {
            QDir dir(R"(E:\YandexDisk\G2G\test files\Ucamco\Gerber_File_Format_Examples 20210409)");
            // QDir dir("D:/Gerber Test Files/CopperCAM/");
            // QDir dir("C:/Users/X-Ray/Documents/3018/CNC");
            // QDir dir("E:/PRO/Новая папка/en.stm32f746g-disco_gerber/gerber_B01");
            if (!dir.exists())
                break;
            for (QString str : dir.entryList({"*.gbr"}, QDir::Files)) {
                str = dir.path() + '/' + str;
                QTimer::singleShot(i += k, [this, str] { loadFile(str); });
                //                break;
            }
        }
        // file:///C:/Users/X-Ray/YandexDisk/Табуретка2/Фрагмент3_1.dxf
        // file:///C:/Users/X-Ray/YandexDisk/Табуретка2/Фрагмент3_2.dxf

        if (0)
            QTimer::singleShot(i += k, [this] { loadFile(R"(E:\YandexDisk\G2G\RefUcamco Gerber\20191107_ciaa_acc\ciaa_acc/ciaa_acc-F_Mask.gbr)"); });

        if (0) {
            constexpr auto TYPE = md5::hash32("PocketRaster");
            if (!toolpathActions.contains(TYPE))
                break;
            QTimer::singleShot(i += k, [this] { selectAll(); });
            QTimer::singleShot(i += k, [this, TYPE] { toolpathActions[TYPE]->toggle(); });
            //            QTimer::singleShot(i += k, [this] { dockWidget_->findChild<QPushButton*>("pbAddBridge")->click(); });
            QTimer::singleShot(i += k, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        }

        if (1) {
            constexpr auto DRILLING = md5::hash32("Drilling");
            QTimer::singleShot(i += k, [this, DRILLING] { toolpathActions[DRILLING]->toggle(); });
        }
        //        if (0) {
        //            i = 1000;
        //            QTimer::singleShot(i += k, [this] { selectAll(); });
        //            QTimer::singleShot(i += k, [this] { toolpathActions[GCode::Pocket]->toggle(); });
        //            //            QTimer::singleShot(i += k, [this] { dockWidget_->findChild<QPushButton*>("pbAddBridge")->click(); });
        //            QTimer::singleShot(i += k, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        //            QTimer::singleShot(i += k, [this] { App::graphicsView()->zoomFit(); });
        //        }
        //        if (0)
        //            QTimer::singleShot(i += k, [this] { toolpathActions[GCode::Drill]->toggle(); });
        break;
    }
    return {};
}

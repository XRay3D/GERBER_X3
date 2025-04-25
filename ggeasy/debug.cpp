// #include "a_pch.h"

#include "app.h"
#include "gi.h"
#include "graphicsview.h"
#include "mainwindow.h"
#include "md5.h"

//// #include <QAction>
//// #include <QDockWidget>
#include <QDir>
#include <QTimer>
#include <QtWidgets>
#include <qfileinfo.h>

void delay_ms(int ms) {
    QEventLoop loop;
    QTimer::singleShot(ms, [&loop] { loop.exit(); });
    loop.exec();
}

namespace QtPrivate {
template <>
inline QDebug printSequentialContainer(QDebug debug, const char* which, const QList<QAction*>& c) {
    const bool oldSetting = debug.autoInsertSpaces();
    debug.nospace() << which << "QList<QAction*>(\n\t";
    typename QList<QAction*>::const_iterator it = c.begin(), end = c.end();
    if(it != end) {
        debug << std::distance(c.begin(), it) << *it;
        ++it;
    }
    while(it != end) {
        debug << ",\n\t" << std::distance(c.begin(), it) << *it;
        ++it;
    }
    debug << "\n)";
    debug.setAutoInsertSpaces(oldSetting);
    return debug.maybeSpace();
}
} // namespace QtPrivate

bool MainWindow::debug() {

    while(App::isDebug() || 1) { // FIXME NOTE need for debug
        int time = 100;
        int delay = 100; //-V654

        if(0) {
            QDir dir{
                R"(C:\Users\bakiev\Junk_Yard\SFT\CAM\CopperCAM)"
                // "C:/Users/X-Ray/Documents/3018/CNC"
                // "D:/Gerber Test Files/CopperCAM/"
                // "E:/PRO/Новая папка/en.stm32f 746g-disco_gerber/gerber_B01"
                // R"(/home/x-ray/projects/dxf/)"
                // R"(/home/x-ray/projects/qt/AMK-310/AMK_TESTER)"
                // R"(/home/x-ray/Загрузки/Gerber_TL-kontroler_PCB_TL-kontroler_2_2024-03-08/)"
            };
            if(!dir.exists()) break;
            for(auto&& str: dir.entryList({"*.gbr"}, QDir::Files)) {
                str = dir.path() % '/' % str;
                QTimer::singleShot(time += delay, [this, str] { loadFile(str); });
                break;
            }
            QTimer::singleShot(time += delay * 5, this, [this] { App::grView().fitInView(App::grView().scene()->itemsBoundingRect()); });
        }
        // file:///C:/Users/X-Ray/YandexDisk/Табуретка2/Фрагмент3_1.dxf
        // file:///C:/Users/X-Ray/YandexDisk/Табуретка2/Фрагмент3_2.dxf

        if(0) {

            for(auto* file: {
                    R"(/home/x-ray/Загрузки/gerber1.gbr)",
                    R"(C:\Users\bakiev\Downloads\gerber1.gbr)",
                    R"(E:\YandexDisk\G2G\RefUcamco Gerber\20191107_ciaa_acc\ciaa_acc/ciaa_acc-F_Mask.gbr)"}) {
                if(!QFileInfo::exists(file)) continue;
                QTimer::singleShot(time += delay, this, [this, file] { loadFile(file); });
                QTimer::singleShot(time += delay * 5, this, [this] { App::grView().fitInView(App::grView().scene()->itemsBoundingRect()); });
            }
        }

        if(0) {
            constexpr auto TYPE = md5::hash32("PocketRaster");
            if(!toolpathActions.contains(TYPE))
                break;
            QTimer::singleShot(time += delay, this, [this] { selectAll(); });
            QTimer::singleShot(time += delay, this, [this, TYPE] { toolpathActions[TYPE]->toggle(); });
            QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        }

        if(0) {
            constexpr auto TYPE = md5::hash32("PocketOffset");
            if(!toolpathActions.contains(TYPE))
                break;
            QTimer::singleShot(time += delay, this, [this] { selectAll(); });
            QTimer::singleShot(time += delay, this, [this, TYPE] { toolpathActions[TYPE]->toggle(); });
            QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        }

        if(0) {
            constexpr auto TYPE = md5::hash32("CrossHatch");
            if(!toolpathActions.contains(TYPE)) break;
            QTimer::singleShot(time += delay, this, [this] { selectAll(); });
            QTimer::singleShot(time += delay, this, [this, TYPE] { toolpathActions[TYPE]->toggle(); });
            QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        }

        if(0) {
            constexpr auto DRILLING = md5::hash32("Drilling");
            QTimer::singleShot(time += delay, this, [this, DRILLING] { toolpathActions[DRILLING]->toggle(); });
            QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        }

        if(0) {
            constexpr auto THERMAL = md5::hash32("Thermal");
            QTimer::singleShot(time += delay, this, [this, THERMAL] { toolpathActions[THERMAL]->toggle(); });
        }

        if(0) {
            constexpr auto PROFILE = md5::hash32("Profile");
            delay_ms(1000);
            QTimer::singleShot(time += delay, this, [this] { selectAll(); });
            QTimer::singleShot(time += delay, this, [this, PROFILE] { toolpathActions[PROFILE]->toggle(); });
            //            QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>("pbAddBridge")->click(); });
            QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
            QTimer::singleShot(time += delay, this, [this] { App::grView().zoomFit(); });
        }

        if(0) {
            constexpr auto THREAD = md5::hash32("Thread");
            QTimer::singleShot(time += delay, this, [this, THREAD] { toolpathActions[THREAD]->toggle(); });

            //            QTimer::singleShot(time += delay, this, [this] { selectAll(); });
            //            QTimer::singleShot(time += delay, this, [this, PROFILE] { toolpathActions[PROFILE]->toggle(); });
            //            //            QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>("pbAddBridge")->click(); });
            //            //            QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
            //            QTimer::singleShot(time += delay, this, [this] { App::grView().zoomFit(); });
        }

        if(0) {
            /*
            qDebug() << actionGroup.actions();
            0   QAction(0x210e31c2160 text="&G-Code Properties" toolTip="G-Code Properties" checked=false shortcut=QKeySequence("Ctrl+Shift+G") menuRole=TextHeuristicRole visible=true),
            1   QAction(0x210e31c2c50 text="Thermal" toolTip="Thermal" checked=false shortcut=QKeySequence("Ctrl+Shift+T") menuRole=TextHeuristicRole visible=true),
            2   QAction(0x210e31c2d40 text="Pocket Raster" toolTip="Pocket Raster" checked=false shortcut=QKeySequence("Ctrl+Shift+R") menuRole=TextHeuristicRole visible=true),
            3   QAction(0x210e31c36a0 text="Drill" toolTip="Drill" checked=false shortcut=QKeySequence("Ctrl+Shift+D") menuRole=TextHeuristicRole visible=true),
            4   QAction(0x210e31c3100 text="Profile" toolTip="Profile" checked=false shortcut=QKeySequence("Ctrl+Shift+F") menuRole=TextHeuristicRole visible=true),
            5   QAction(0x210e31c2ed0 text="Voronoi" toolTip="Voronoi" checked=false shortcut=QKeySequence("Ctrl+Shift+V") menuRole=TextHeuristicRole visible=true),
            6   QAction(0x210e31c3790 text="Pocket Offset" toolTip="Pocket Offset" checked=false shortcut=QKeySequence("Ctrl+Shift+P") menuRole=TextHeuristicRole visible=true),
            7   QAction(0x210e31c3420 text="Crosshatch" toolTip="Crosshatch" checked=false shortcut=QKeySequence("Ctrl+Shift+C") menuRole=TextHeuristicRole visible=true),
            8   QAction(0x210e31c35b0 text="Circle" toolTip="Circle" checked=false menuRole=TextHeuristicRole visible=true),
            9   QAction(0x210e31c3b50 text="Rectangle" toolTip="Rectangle" checked=false menuRole=TextHeuristicRole visible=true),
            10  QAction(0x210e31c3290 text="Poly Line" toolTip="Poly Line" checked=false menuRole=TextHeuristicRole visible=true),
            11  QAction(0x210e31c32e0 text="Circle Arc" toolTip="Circle Arc" checked=false menuRole=TextHeuristicRole visible=true),
            12  QAction(0x210e31c4320 text="Text" toolTip="Text" checked=false menuRole=TextHeuristicRole visible=true)
            */

            for(auto* action: actionGroup.actions())
                if(action->text() == "Text")
                    QTimer::singleShot(time += delay, this, [this, action] { action->toggle(); });
            QTimer::singleShot(time += delay, this, [this] { selectAll(); });
        }

        //        if (0) {
        //            i = 1000;
        //            QTimer::singleShot(time += delay, this,[this] { selectAll(); });
        //            QTimer::singleShot(time += delay, this,[this] { toolpathActions[GCode::Pocket]->toggle(); });
        //            //            QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>("pbAddBridge")->click(); });
        //            QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        //            QTimer::singleShot(time += delay, this,[this] { App::grView().zoomFit(); });
        //        }
        //        if (0)
        //            QTimer::singleShot(time += delay, this,[this] { toolpathActions[GCode::Drill]->toggle(); });
        break;
    }
    return {};
}

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
    debug.nospace() << which << u"QList<QAction*>(\n\t"_s;
    typename QList<QAction*>::const_iterator it = c.begin(), end = c.end();
    if(it != end) {
        debug << std::distance(c.begin(), it) << *it;
        ++it;
    }
    while(it != end) {
        debug << u",\n\t"_s << std::distance(c.begin(), it) << *it;
        ++it;
    }
    debug << u"\n)"_s;
    debug.setAutoInsertSpaces(oldSetting);
    return debug.maybeSpace();
}
} // namespace QtPrivate

bool MainWindow::cli(std::span<std::string_view> commands) {
    static int time = 100;
    static int delay = 100; //-V654
    using pair = std::pair<std::string_view, std::function<void()>>;
    static std::array commandsMap{
        pair{"LoadDir"sv,        [this] {
                 QSettings settings;
                 QDir dir{(
                     recentFiles.readRecentFiles(settings)
                     | v::filter(qOverload<const QString&>(&QFileInfo::exists)) | v::take(1))
                         .front()};
                 for(auto&& str: dir.entryList({u"*.gbr"_s}, QDir::Files)) {
                     str = dir.path() + u'/' + str;
                     QTimer::singleShot(time += delay, [this, str] { loadFile(str); });
                     break;
                 }
                 QTimer::singleShot(time += delay * 5, this, [] { App::grView().fitInView(App::grView().scene()->itemsBoundingRect()); });
             }       },
        pair{"LoadRecentFile"sv, [this] {
                 QSettings settings;
                 for(auto&& file: recentFiles.readRecentFiles(settings) | v::take(1)) {
                     if(!QFileInfo::exists(file)) continue;
                     QTimer::singleShot(time += delay, this, [this, file] { loadFile(file); });
                     QTimer::singleShot(time += delay * 5, this, [] { App::grView().zoomFit(); });
                     // QTimer::singleShot(time += delay * 5, this, [this] { App::grView().fitInView(App::grView().scene()->itemsBoundingRect()); });
                 }
             }},
        pair{"PocketRaster"sv,   [this] {
                 constexpr auto TYPE = "PocketRaster"_hash32;
                 if(!toolpathActions.contains(TYPE)) return;
                 QTimer::singleShot(time += delay, this, [this] { selectAll(); });
                 QTimer::singleShot(time += delay, this, [this, TYPE] { toolpathActions[TYPE]->toggle(); });
                 QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>(u"pbCreate"_s)->click(); });
             }  },
        pair{"PocketOffset"sv,   [this] {
                 constexpr auto TYPE = "PocketOffset"_hash32;
                 if(!toolpathActions.contains(TYPE)) return;
                 QTimer::singleShot(time += delay, this, [this] { selectAll(); });
                 QTimer::singleShot(time += delay, this, [this, TYPE] { toolpathActions[TYPE]->toggle(); });
                 QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>(u"pbCreate"_s)->click(); });
             }  },
        pair{"CrossHatch"sv,     [this] {
                 constexpr auto TYPE = "CrossHatch"_hash32;
                 if(!toolpathActions.contains(TYPE)) return;
                 QTimer::singleShot(time += delay, this, [this] { selectAll(); });
                 QTimer::singleShot(time += delay, this, [this, TYPE] { toolpathActions[TYPE]->toggle(); });
                 QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>(u"pbCreate"_s)->click(); });
             }    },
        pair{"Drilling"sv,       [this] {
                 constexpr auto DRILLING = "Drilling"_hash32;
                 QTimer::singleShot(time += delay, this, [this, DRILLING] { toolpathActions[DRILLING]->toggle(); });
                 QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>(u"pbCreate"_s)->click(); });
             }      },
        pair{"Thermal"sv,        [this] {
                 constexpr auto THERMAL = "Thermal"_hash32;
                 QTimer::singleShot(time += delay, this, [this, THERMAL] { toolpathActions[THERMAL]->toggle(); });
             }       },
        pair{"Profile"sv,        [this] {
                 constexpr auto PROFILE = "Profile"_hash32;
                 delay_ms(1000);
                 QTimer::singleShot(time += delay, this, [this] { selectAll(); });
                 QTimer::singleShot(time += delay, this, [this, PROFILE] { toolpathActions[PROFILE]->toggle(); });
                 // QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>(u"pbAddBridge"_s)->click(); });
                 QTimer::singleShot(time += delay, this, [this] { dockWidget_->findChild<QPushButton*>(u"pbCreate"_s)->click(); });
                 QTimer::singleShot(time += delay, this, [this] { App::grView().zoomFit(); });
             }       },
        pair{"Thread"sv,         [this] {
                 constexpr auto THREAD = "Thread"_hash32;
                 QTimer::singleShot(time += delay, this, [this, THREAD] { toolpathActions[THREAD]->toggle(); });

                 // QTimer::singleShot(time += delay, this, [this] { selectAll(); });
                 // QTimer::singleShot(time += delay, this, [this, PROFILE] { toolpathActions[PROFILE]->toggle(); });
                 // // QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>(u"pbAddBridge"_s)->click(); });
                 // // QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>(u"pbCreate"_s)->click(); });
                 // QTimer::singleShot(time += delay, this, [this] { App::grView().zoomFit(); });
             }        },
        pair{"Test"sv,           [this] {
                 /*
                 qDebug() << actionGroup.actions();
                 0   QAction(0x210e31c2160 text=u"&G-Code Properties"_s toolTip=u"G-Code Properties"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+G"_s) menuRole=TextHeuristicRole visible=true),
                 1   QAction(0x210e31c2c50 text=u"Thermal"_s toolTip=u"Thermal"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+T"_s) menuRole=TextHeuristicRole visible=true),
                 2   QAction(0x210e31c2d40 text=u"Pocket Raster"_s toolTip=u"Pocket Raster"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+R"_s) menuRole=TextHeuristicRole visible=true),
                 3   QAction(0x210e31c36a0 text=u"Drill"_s toolTip=u"Drill"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+D"_s) menuRole=TextHeuristicRole visible=true),
                 4   QAction(0x210e31c3100 text=u"Profile"_s toolTip=u"Profile"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+F"_s) menuRole=TextHeuristicRole visible=true),
                 5   QAction(0x210e31c2ed0 text=u"Voronoi"_s toolTip=u"Voronoi"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+V"_s) menuRole=TextHeuristicRole visible=true),
                 6   QAction(0x210e31c3790 text=u"Pocket Offset"_s toolTip=u"Pocket Offset"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+P"_s) menuRole=TextHeuristicRole visible=true),
                 7   QAction(0x210e31c3420 text=u"Crosshatch"_s toolTip=u"Crosshatch"_s checked=false shortcut=QKeySequence(u"Ctrl+Shift+C"_s) menuRole=TextHeuristicRole visible=true),
                 8   QAction(0x210e31c35b0 text=u"Circle"_s toolTip=u"Circle"_s checked=false menuRole=TextHeuristicRole visible=true),
                 9   QAction(0x210e31c3b50 text=u"Rectangle"_s toolTip=u"Rectangle"_s checked=false menuRole=TextHeuristicRole visible=true),
                 10  QAction(0x210e31c3290 text=u"Poly Line"_s toolTip=u"Poly Line"_s checked=false menuRole=TextHeuristicRole visible=true),
                 11  QAction(0x210e31c32e0 text=u"Circle Arc"_s toolTip=u"Circle Arc"_s checked=false menuRole=TextHeuristicRole visible=true),
                 12  QAction(0x210e31c4320 text=u"Text"_s toolTip=u"Text"_s checked=false menuRole=TextHeuristicRole visible=true)
                 */

                 for(auto* action: actionGroup.actions())
                     if(action->text() == u"Text"_s)
                         QTimer::singleShot(time += delay, this, [this, action] { action->toggle(); });
                 QTimer::singleShot(time += delay, this, [this] { selectAll(); });
             }          },
    };

    // for(auto&& cmd: commands) {
    //     if(auto it = r::find(commandsMap, cmd, &pair::first);
    //         it != commandsMap.end())
    //         it->second();
    // }

    for(auto&& [name, func]: commandsMap)
        if(r::find(commands, name) != commands.end())
            func();

    time = 100;
    delay = 100;

    // if (0) {
    // i = 1000;
    // QTimer::singleShot(time += delay, this,[this] { selectAll(); });
    // QTimer::singleShot(time += delay, this,[this] { toolpathActions[GCode::Pocket]->toggle(); });
    // // QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>(u"pbAddBridge"_s)->click(); });
    // QTimer::singleShot(time += delay, this,[this] { dockWidget_->findChild<QPushButton*>(u"pbCreate"_s)->click(); });
    // QTimer::singleShot(time += delay, this,[this] { App::grView().zoomFit(); });
    // }
    // if (0)
    // QTimer::singleShot(time += delay, this,[this] { toolpathActions[GCode::Drill]->toggle(); });

    return {};
}

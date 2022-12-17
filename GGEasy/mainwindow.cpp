// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/********************************************************************************
 * Author : Damir Bakiev *
 * Version : na *
 * Date : 11 November 2021 *
 * Website : na *
 * Copyright : Damir Bakiev 2016-2022 *
 * License: *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt *
 ********************************************************************************/
#include "mainwindow.h"

#include "aboutform.h"
#include "file.h"
#include "file_plugin.h"
#include "gc_plugin.h"
#include "gc_propertiesform.h"
#include "gcode.h"
#include "gi_datapath.h"
#include "gi_datasolid.h"
#include "gi_point.h"
#include "graphicsview.h"
#include "plugindialog.h"
#include "project.h"
#include "settingsdialog.h"
#include "shapepluginin.h"
#include "tool_database.h"
// #include "qt.h"

#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QtWidgets>
#include <forward_list>

static auto IntPointConverter = QMetaType::registerConverter(&IntPoint::toString);

bool operator<(const QPair<Tool, Side>& p1, const QPair<Tool, Side>& p2) {
    return p1.first.hash() < p2.first.hash() || (!(p2.first.hash() < p1.first.hash()) && p1.second < p2.second);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , recentFiles {this, "recentFiles"}
    , recentProjects {this, "recentProjects"}
    , project_ {new Project {this}}
    , actionGroup {this}
    , reloadQuestion {this} {
    App::setMainWindow(this);
    App::setUndoStack(&undoStack_);

    ui.setupUi(this);

    initWidgets();
    LayoutFrames* lfp;
    ui.graphicsView->scene()->addItem(new GiMarker(GiMarker::Home));
    ui.graphicsView->scene()->addItem(new GiMarker(GiMarker::Zero));
    ui.graphicsView->scene()->addItem(new GiPin());
    ui.graphicsView->scene()->addItem(new GiPin());
    ui.graphicsView->scene()->addItem(new GiPin());
    ui.graphicsView->scene()->addItem(new GiPin());
    ui.graphicsView->scene()->addItem(lfp = new LayoutFrames());

    GCodePropertiesForm(); // init default vars;

    connect(project_, &Project::homePosChanged, App::home(), qOverload<const QPointF&>(&GiMarker::setPos));
    connect(project_, &Project::zeroPosChanged, App::zero(), qOverload<const QPointF&>(&GiMarker::setPos));
    connect(project_, &Project::pinsPosChanged, qOverload<const QPointF[4]>(&GiPin::setPos));
    connect(project_, &Project::layoutFrameUpdate, lfp, &LayoutFrames::updateRect);
    connect(project_, &Project::changed, this, &MainWindow::documentWasModified);
    menuBar()->installEventFilter(this);

    // connect plugins
    for (auto& [type, ptr] : App::filePlugins()) {
        if (ptr->type() == int(FileType::GCode))
            continue;
        ptr->moveToThread(&parserThread);
        connect(ptr, &FilePlugin::fileError, this, &MainWindow::fileError, Qt::QueuedConnection);
        connect(ptr, &FilePlugin::fileProgress, this, &MainWindow::fileProgress, Qt::QueuedConnection);
        connect(ptr, &FilePlugin::fileReady, this, &MainWindow::addFileToPro, Qt::QueuedConnection);
        connect(this, &MainWindow::parseFile, ptr, &FilePlugin::parseFile, Qt::QueuedConnection);
        connect(project_, &Project::parseFile, ptr, &FilePlugin::parseFile, Qt::QueuedConnection);
    }

    parserThread.start(QThread::HighestPriority);

    connect(ui.graphicsView, &GraphicsView::fileDroped, this, &MainWindow::loadFile);

    // Shapes::Constructor

    // status bar
    connect(ui.graphicsView, &GraphicsView::mouseMove, [this](const QPointF& point) {
        ui.statusbar->showMessage(QString("X = %1, Y = %2").arg(point.x()).arg(point.y()));
    });

    ui.treeView->setModel(new FileTree::Model(ui.treeView));

    connect(ui.treeView, &FileTree::View::saveGCodeFile, this, &MainWindow::saveGCodeFile);
    connect(ui.treeView, &FileTree::View::saveGCodeFiles, this, &MainWindow::saveGCodeFiles); // NOTE unused
    connect(ui.treeView, &FileTree::View::saveSelectedGCodeFiles, this, &MainWindow::saveSelectedGCodeFiles);

    App::toolHolder().readTools();
    setCurrentFile(QString());

    readSettings();
    toolpathActions[GCode::GCodeProperties]->triggered();

    connect(&GiDataPath::timer, &QTimer::timeout, [] { ++GiDataPath::dashOffset; });
    GiDataPath::timer.start(50);

    if (qApp->applicationDirPath().contains("GERBER_X3/bin/")) { // NOTE (need for debug)
        int i = 100;
        int k = 100;

        if (0) {
            QDir dir(R"(C:\Users\X-Ray\YandexDisk\G2G\test files\Ucamco\gerber_file_format_examples 20181113)");
            // QDir dir("D:/Gerber Test Files/CopperCAM/");
            // QDir dir("C:/Users/X-Ray/Documents/3018/CNC");
            // QDir dir("E:/PRO/Новая папка/en.stm32f746g-disco_gerber/gerber_B01");
            if (dir.exists())
                for (QString str : dir.entryList({"*.gbr"}, QDir::Files)) {
                    str = dir.path() + '/' + str;
                    QTimer::singleShot(i += k, [this, str] { loadFile(str); });
                    // break;
                }
        }
        // file:///C:/Users/X-Ray/YandexDisk/Табуретка2/Фрагмент3_1.dxf
        // file:///C:/Users/X-Ray/YandexDisk/Табуретка2/Фрагмент3_2.dxf

        if (1)
            //            QTimer::singleShot(i += k, [this] { loadFile(R"(D:/Downloads/ФАЛЬШПАНЕЛИ РАСХОДОМЕТРИЯ-dxf/725327.003 Панель EL-SV-11-R-AG-измен.dxf)"); });
            QTimer::singleShot(i += k, [this] { loadFile(R"(C:/Users/X-Ray/YandexDisk/ESAMP/ELECTROSTATIC_AMP/T/ELECTROSTATIC_AMP_A.fst)"); });

        if (0) {
            QTimer::singleShot(i += k, [this] { selectAll(); });
            QTimer::singleShot(i += k, [this] { toolpathActions[GCode::Drill]->toggle(); });
            QTimer::singleShot(i += k, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
            QTimer::singleShot(i += k, [] { App::graphicsView()->zoomToSelected(); });
        }

        if (0) {
            i = 1000;
            // QTimer::singleShot(i += k, [this] { loadFile(R"(D:\ARM\MagicTable\SchPcb469\en.MB1189_manufacturing\MB1189_B\MB1189_REVB_150522_FAB2_GBR\MB1189_REVB_150522_FAB2-1-6.drl)"); });
            QTimer::singleShot(i += k, [this] { toolpathActions[GCode::Pocket]->toggle(); });
            QTimer::singleShot(i += k, [this] { selectAll(); });
            QTimer::singleShot(i += k, [this] { dockWidget_->findChild<QPushButton*>("pbCreate")->click(); });
        }
        if (0)
            QTimer::singleShot(i += k, [this] { toolpathActions[GCode::Drill]->toggle(); });
    }
}

MainWindow::~MainWindow() {
    parserThread.quit();
    parserThread.wait();
    App::setMainWindow(nullptr);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (qApp->applicationDirPath().contains("GERBER_X3/bin") || maybeSave()) {
        writeSettings();
        delete dockWidget_;
        qApp->closeAllWindows();
        App::fileModel()->closeProject();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::closeProject() {
    if (maybeSave()) {
        dockWidget_->close();
        App::fileModel()->closeProject();
        setCurrentFile(QString());
        project_->close();
        // ui.graphicsView->scene()->clear();
        return true;
    }
    return false;
}

void MainWindow::about() {
    AboutForm a(this);
    a.exec();
}

void MainWindow::initWidgets() {
    createActions();
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::createActions() {
    dockWidget_ = new QDockWidget(this);
    dockWidget_->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockWidget_->setObjectName(QStringLiteral("dwCreatePath"));
    dockWidget_->installEventFilter(this);

    QFont font;
    font.setBold(true);
    dockWidget_->setFont(font);

    // fileMenu
    createActionsFile();
    // zoomToolBar
    createActionsZoom();
    // fileEdit // Selection / Delete selected
    createActionsEdit();
    // serviceMenu
    createActionsService();
    // toolpathToolBar
    createActionsToolPath();
    // grafica
    createActionsShape();
    // helpMenu
    createActionsHelp();

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createActionsFile() {
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName(QStringLiteral("fileMenu"));

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName(QStringLiteral("fileToolBar"));
    fileToolBar->setToolTip(tr("File"));

    fileToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fileToolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::customContextMenuForToolBar);
    QAction* action;
    // New
    action = fileMenu->addAction(QIcon::fromTheme("project-development-new-template"), tr("&New project"), this, &MainWindow::newFile);
    action->setShortcuts(QKeySequence::New);
    action->setStatusTip(tr("Create a new file"));
    fileToolBar->addAction(QIcon::fromTheme("project-development-new-template"), tr("&New project"), this, &MainWindow::newFile);

    // Open
    action = fileMenu->addAction(QIcon::fromTheme("document-open"), tr("&Open..."), this, &MainWindow::open);
    action->setShortcuts(QKeySequence::Open);
    action->setStatusTip(tr("Open an existing file"));
    fileToolBar->addAction(QIcon::fromTheme("document-open"), tr("&Open..."), this, &MainWindow::open);

    // Save
    action = fileMenu->addAction(QIcon::fromTheme("document-save"), tr("&Save project"), this, &MainWindow::save);
    action->setShortcuts(QKeySequence::Save);
    action->setStatusTip(tr("Save the document to disk"));
    fileToolBar->addAction(QIcon::fromTheme("document-save"), tr("&Save project"), this, &MainWindow::save);

    // Save As
    action = fileMenu->addAction(QIcon::fromTheme("document-save-as"), tr("Save project &As..."), this, &MainWindow::saveAs);
    action->setShortcuts(QKeySequence::SaveAs);
    action->setStatusTip(tr("Save the document under a new name"));
    fileToolBar->addAction(QIcon::fromTheme("document-save-as"), tr("Save project &As..."), this, &MainWindow::saveAs);

    // Close project
    closeAllAct_ = fileMenu->addAction(QIcon::fromTheme("document-close"), tr("&Close project \"%1\""), this, &MainWindow::closeProject);
    closeAllAct_->setShortcuts(QKeySequence::Close);
    closeAllAct_->setStatusTip(tr("Close project"));
    // closeAllAct_->setEnabled(false);
    fileToolBar->addAction(QIcon::fromTheme("document-close"), tr("&Close project \"%1\"").arg(""), this, &MainWindow::closeProject);

    fileMenu->addSeparator();
    fileToolBar->addSeparator();

    // Save Selected Tool Paths
    action = fileMenu->addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."), this, &MainWindow::saveSelectedGCodeFiles);
    action->setStatusTip(tr("Save selected toolpaths"));
    fileToolBar->addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."), this, &MainWindow::saveSelectedGCodeFiles);

    // Export PDF
    // FIXME       action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), App::graphicsView()->scene(), &Scene::renderPdf);
    //        action->setStatusTip(tr("Export to PDF file"));
    //        fileToolBar->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), App::graphicsView()->scene(), &Scene::renderPdf);

    fileMenu->addSeparator();
    fileMenu->addSeparator();

    recentFiles.createMenu(fileMenu, tr("Recent Files..."));
    recentProjects.createMenu(fileMenu, tr("Recent Projects..."));

    fileMenu->addSeparator();

    action = fileMenu->addAction(QIcon::fromTheme("document-print"), tr("P&rint"), this, &MainWindow::printDialog);
    action->setShortcuts(QKeySequence::Print);
    action->setStatusTip(tr("Print"));

    fileMenu->addSeparator();

    action = fileMenu->addAction(QIcon::fromTheme("application-exit"), tr("E&xit"), qApp, &QApplication::closeAllWindows);
    action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application"));
}

void MainWindow::createActionsEdit() {
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->setObjectName(QStringLiteral("editMenu"));
    QAction* action;
    action = editMenu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select all"), this, &MainWindow::selectAll);
    action->setShortcut(QKeySequence::SelectAll);

    auto dsaShortcut = new QShortcut(this);                                      // Инициализируем объект
    dsaShortcut->setKey(Qt::Key_Escape);                                         // Устанавливаем код клавиши
    connect(dsaShortcut, &QShortcut::activated, this, &MainWindow::deSelectAll); // цепляем обработчик нажатия клавиши

    editMenu->addSeparator();

    undoAct = undoStack_.createUndoAction(this, tr("Undo"));
    undoAct->setShortcut(QKeySequence::Undo);
    undoAct->setIcon(QIcon::fromTheme("edit-undo"));
    editMenu->addAction(undoAct);

    redoAct = undoStack_.createRedoAction(this, tr("Redo"));
    redoAct->setShortcut(QKeySequence::Redo);
    redoAct->setIcon(QIcon::fromTheme("edit-redo"));
    editMenu->addAction(redoAct);
}

void MainWindow::createActionsService() {
    serviceMenu = menuBar()->addMenu(tr("&Service"));

    toolpathToolBar = addToolBar(tr("Service"));
    toolpathToolBar->setObjectName("tbService");
    toolpathToolBar->setToolTip(tr("Service"));

    toolpathToolBar->addAction(redoAct);
    toolpathToolBar->addAction(undoAct);
    toolpathToolBar->addSeparator();

    QAction* action;
    // Settings
    action = serviceMenu->addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Settings"), [this] { SettingsDialog(this).exec(); });
    action->setStatusTip(tr("Show the application's settings box"));
    // Separator
    // toolpathToolBar->addSeparator();
    serviceMenu->addSeparator();
    // G-Code Properties
    serviceMenu->addAction(action = toolpathToolBar->addAction(QIcon::fromTheme("node"), tr("&G-Code Properties")));
    connect(action, &QAction::toggled, [=, this](bool checked) { if (checked) setDockWidget(new GCodePropertiesForm); });
    action->setShortcut(QKeySequence("Ctrl+Shift+G"));
    action->setCheckable(true);
    toolpathActions.emplace(GCode::GCodeProperties, action);
    actionGroup.addAction(action);
    // Tool Base
    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"), [this] { ToolDatabase(this, {}).exec(); }));
    // Separator
    serviceMenu->addAction(toolpathToolBar->addSeparator());
    // Autoplace All Refpoints
    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Autoplace All Refpoints"), [this] {
        if (updateRect()) {
            GiPin::resetPos(false);
            App::home()->resetPos(false);
            App::zero()->resetPos(false);
        }
        ui.graphicsView->zoomFit();
    }));
    // Separator
    serviceMenu->addAction(toolpathToolBar->addSeparator());
    // Snap to grid
    serviceMenu->addAction(action = toolpathToolBar->addAction(QIcon::fromTheme("snap-to-grid"), tr("Snap to grid"), [](bool checked) { App::settings().setSnap(checked); }));
    action->setCheckable(true);
    // Separator
    serviceMenu->addAction(toolpathToolBar->addSeparator());
    serviceMenu->addAction(action = toolpathToolBar->addAction(QIcon::fromTheme("ruller-on"), tr("Ruller"), ui.graphicsView, &GraphicsView::setRuler));
    action->setCheckable(true);
    // Resize
    if (qApp->applicationDirPath().contains("GERBER_X3/bin")) { // (need for debug)
        serviceMenu->addSeparator();
        toolpathToolBar->addSeparator();
        serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Resize"), [this] {
            setGeometry(QRect {0, 1080, 1024, 720});
        }));
    }
}

void MainWindow::createActionsHelp() {
    helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* action;
    // About
    action = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    action->setStatusTip(tr("Show the application's About box"));
    // About Qt
    action = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    action->setStatusTip(tr("Show the Qt library's About box"));
    // Separator
    helpMenu->addSeparator();
    // About Plugins
    action = helpMenu->addAction(tr("About &Plugins…"), [this] { DialogAboutPlugins(this).exec(); });
    action->setStatusTip(tr("Show loaded plugins…"));
}

void MainWindow::createActionsZoom() {
    auto vievMenu = menuBar()->addMenu(tr("&Viev"));
    vievMenu->setObjectName("vievMenu");

    zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    zoomToolBar->setToolTip(tr("Zoom ToolBar"));

    zoomToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(zoomToolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::customContextMenuForToolBar);

    QAction* action;
    // Fit best
    zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), ui.graphicsView, &GraphicsView::zoomFit);
    action = vievMenu->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), ui.graphicsView, &GraphicsView::zoomFit);
    action->setShortcut(QKeySequence::FullScreen);
    vievMenu->addAction(action);

    // 100%
    zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), ui.graphicsView, &GraphicsView::zoom100);
    action = vievMenu->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), ui.graphicsView, &GraphicsView::zoom100);
    action->setShortcut(tr("Ctrl+0"));
    vievMenu->addAction(action);

    // Zoom in
    zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), ui.graphicsView, &GraphicsView::zoomIn);
    action = vievMenu->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), ui.graphicsView, &GraphicsView::zoomIn);
    action->setShortcut(QKeySequence::ZoomIn);
    vievMenu->addAction(action);

    // Zoom out
    zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), ui.graphicsView, &GraphicsView::zoomOut);
    action = vievMenu->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), ui.graphicsView, &GraphicsView::zoomOut);
    action->setShortcut(QKeySequence::ZoomOut);
    vievMenu->addAction(action);

    // Separator
    zoomToolBar->addSeparator();
    vievMenu->addSeparator();

    // Zoom to selected
    zoomToolBar->addAction(QIcon::fromTheme("zoom-to-selected"), tr("Zoom to selected"), ui.graphicsView, &GraphicsView::zoomToSelected);
    action = vievMenu->addAction(QIcon::fromTheme("zoom-to-selected"), tr("Zoom to selected"), ui.graphicsView, &GraphicsView::zoomToSelected);
    action->setShortcut(QKeySequence("F12"));
    vievMenu->addAction(action);
}

void MainWindow::createActionsToolPath() {
    if (!App::gCodePlugins().size())
        return;

    QMenu* menu = menuBar()->addMenu(tr("&Paths"));

    toolpathToolBar = addToolBar(tr("Toolpath"));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));
    toolpathToolBar->setToolTip(tr("Toolpath"));

    addDockWidget(Qt::RightDockWidgetArea, dockWidget_);

    for (auto& [type, gCodePlugin] : App::gCodePlugins()) {
        auto action = gCodePlugin->addAction(menu, toolpathToolBar);
        action->setCheckable(true);
        actionGroup.addAction(action);
        toolpathActions.emplace(type, action);
        connect(gCodePlugin, &GCodePlugin::setDockWidget, this, &MainWindow::setDockWidget);
    }
}

void MainWindow::createActionsShape() {
    if (App::shapePlugins().empty())
        return;

    QToolBar* toolBar = addToolBar(tr("Graphics Items"));
    toolBar->setObjectName("GraphicsItemsToolBar");
    toolBar->setToolTip(tr("Graphics Items"));

    toolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::customContextMenuForToolBar);

    for (auto& [type, shPlugin] : App::shapePlugins()) {
        auto action = toolBar->addAction(shPlugin->icon(), shPlugin->info().value("Name").toString());
        action->setCheckable(true);
        actionGroup.addAction(action);
        connect(shPlugin, &Shapes::Plugin::actionUncheck, action, &QAction::setChecked);
        connect(action, &QAction::toggled, [shPlugin = shPlugin, this](bool checked) {
            if (checked) {
                connect(ui.graphicsView, &GraphicsView::mouseMove, shPlugin, &Shapes::Plugin::updPoint);
                connect(ui.graphicsView, &GraphicsView::mouseClickR, shPlugin, &Shapes::Plugin::finalizeShape);
                connect(ui.graphicsView, &GraphicsView::mouseClickL, shPlugin, &Shapes::Plugin::addPoint);
                setDockWidget(new QPushButton);
            } else {
                shPlugin->finalizeShape();
                disconnect(ui.graphicsView, &GraphicsView::mouseMove, shPlugin, &Shapes::Plugin::updPoint);
                disconnect(ui.graphicsView, &GraphicsView::mouseClickR, shPlugin, &Shapes::Plugin::finalizeShape);
                disconnect(ui.graphicsView, &GraphicsView::mouseClickL, shPlugin, &Shapes::Plugin::addPoint);
            }
        });
    }

    toolBar->addSeparator();

    auto executor = [](ClipType type) {
        qDebug("На переделке");

        auto selectedItems(App::graphicsView()->scene()->selectedItems());
        Paths clipPaths;
        for (QGraphicsItem* clipItem : selectedItems) {
            if (clipItem->type() >= GiType::ShCircle)
                clipPaths.append(static_cast<GraphicsItem*>(clipItem)->paths());
        }

        QList<GraphicsItem*> rmi;
        for (QGraphicsItem* item : selectedItems) {
            if (item->type() == GiType::DataSolid) {
                auto gitem = static_cast<GiDataSolid*>(item);
                Clipper clipper;
                clipper.AddPaths(gitem->paths(), ptSubject, true);
                clipper.AddPaths(clipPaths, ptClip, true);
                Paths paths;
                clipper.Execute(type, paths, pftEvenOdd, pftPositive);
                if (paths.empty()) {
                    rmi.push_back(gitem);
                } else {
                    ReversePaths(paths); // NOTE ??
                    gitem->setPaths(paths);
                }
            }
        }
        for (GraphicsItem* item : rmi)
            delete item->file()->itemGroup()->takeAt(item);
    };
    toolBar->addAction(QIcon::fromTheme("path-union"), tr("Union"), [executor] { executor(ctUnion); });
    toolBar->addAction(QIcon::fromTheme("path-difference"), tr("Difference"), [executor] { executor(ctDifference); });
    toolBar->addAction(QIcon::fromTheme("path-exclusion"), tr("Exclusion"), [executor] { executor(ctXor); });
    toolBar->addAction(QIcon::fromTheme("path-intersection"), tr("Intersection"), [executor] { executor(ctIntersection); });
}

void MainWindow::customContextMenuForToolBar(const QPoint& pos) {
    auto toolBar = qobject_cast<QToolBar*>(sender());
    if (!toolBar)
        return;
    QMenu menu;
    for (auto actFromTb : toolBar->actions()) {
        if (actFromTb->isSeparator()) {
            menu.addSeparator();
            continue;
        }
        auto action = menu.addAction(actFromTb->icon(), actFromTb->text(), [actFromTb](bool checked) { actFromTb->setVisible(checked); });
        action->setCheckable(true);
        action->setChecked(actFromTb->isVisible());
    }
    menu.exec(toolBar->mapToGlobal(pos));
}

void MainWindow::saveGCodeFile(int id) {
    if (project_->pinsPlacedMessage())
        return;
    auto* file = project_->file<GCode::File>(id);
    QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
        GCode::GCFile::getLastDir().append(file->shortName()),
        tr("GCode (*.%1)").arg(GCode::Settings::fileExtension())));

    if (name.isEmpty())
        return;

    file->save(name);
}

void MainWindow::saveGCodeFiles() {
}

void MainWindow::saveSelectedGCodeFiles() {

    if (project_->pinsPlacedMessage())
        return;

    mvector<GCode::File*> gcFiles(project_->files<GCode::File>());
    for (size_t i = 0; i < gcFiles.size(); ++i) {
        if (!gcFiles[i]->itemGroup()->isVisible())
            gcFiles.remove(i--);
    }

    using Key = std::pair<size_t, Side>;
    using GcFiles = QList<GCode::File*>;

    std::map<Key, GcFiles> gcFilesMap;
    for (GCode::File* file : gcFiles)
        gcFilesMap[{file->getTool().hash2(), file->side()}].append(file);

    for (const auto& [key, files] : gcFilesMap) {
        if (files.size() < 2) {
            for (GCode::File* file : files) {
                QString name(GCode::GCFile::getLastDir().append(file->shortName()));
                if (!name.endsWith(GCode::Settings::fileExtension()))
                    name += QStringList({"_TS", "_BS"})[file->side()];

                name = QFileDialog::getSaveFileName(nullptr,
                    QObject::tr("Save GCode file"),
                    name,
                    QObject::tr("GCode (*.%1)").arg(GCode::Settings::fileExtension()));

                if (name.isEmpty())
                    return;
                file->save(name);
                file->itemGroup()->setVisible(false);
            }
        } else {
            QString name(GCode::GCFile::getLastDir().append(files.first()->getTool().nameEnc()));
            if (!name.endsWith(GCode::Settings::fileExtension()))
                name += QStringList({"_TS", "_BS"})[files.first()->side()];

            name = QFileDialog::getSaveFileName(nullptr,
                QObject::tr("Save GCode file"),
                name,
                QObject::tr("GCode (*.%1)").arg(GCode::Settings::fileExtension()));

            if (name.isEmpty())
                return;
            mvector<QString> sl;

            sl.emplace_back(tr(";\tContains files:"));
            for (auto file : files)
                sl.push_back(";\t" + file->shortName());
            for (auto file : files) {
                file->itemGroup()->setVisible(false);
                file->initSave();
                if (file == files.front())
                    file->statFile();
                file->addInfo();
                file->genGcodeAndTile();
                if (file == files.back())
                    file->endFile();
                sl.append(file->gCodeText());
            }
            QFile file(name);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                QString str;
                for (QString& s : sl) {
                    if (!s.isEmpty())
                        str.push_back(s);
                    if (!str.endsWith('\n'))
                        str.push_back("\n");
                }
                out << str;
            }
            file.close();
        }
    }

    if (gcFilesMap.empty())
        QMessageBox::information(nullptr, "", QObject::tr("No selected toolpath files."));
}

void MainWindow::newFile() {
    if (closeProject()) {
        setCurrentFile(QString());
    }
}

void MainWindow::readSettings() {
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    restoreState(settings.value("state", QByteArray()).toByteArray());
    lastPath = settings.value("lastPath").toString();

    for (auto toolBar : findChildren<QToolBar*>()) {
        settings.beginReadArray(toolBar->objectName());
        int ctr = 0;
        for (auto action : toolBar->actions()) {
            settings.setArrayIndex(ctr++);
            action->setVisible(settings.value("actionIsVisible", true).toBool());
        }
        settings.endArray();
    }

    if (qApp->applicationDirPath().contains("GERBER_X3/bin"))
        loadFile(settings.value("project").toString());

    settings.endGroup();
}

void MainWindow::writeSettings() {
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("project", project_->name());
    // toolBar actions visible
    for (auto toolBar : findChildren<QToolBar*>()) {
        settings.beginWriteArray(toolBar->objectName());
        int ctr = 0;
        for (auto action : toolBar->actions()) {
            settings.setArrayIndex(ctr++);
            settings.setValue("actionIsVisible", action->isVisible());
        }
        settings.endArray();
    }
    settings.endGroup();
}

void MainWindow::selectAll() {
    if /* */ (toolpathActions.contains(GCode::Thermal) && toolpathActions[GCode::Thermal]->isChecked()) {
        for (QGraphicsItem* item : App::graphicsView()->scene()->items())
            if (item->type() == GiType::Preview)
                item->setSelected(true);
    } else if (toolpathActions.contains(GCode::Drill) && toolpathActions[GCode::Drill]->isChecked()) {
        for (QGraphicsItem* item : App::graphicsView()->scene()->items())
            if (item->type() == GiType::Preview)
                item->setSelected(true);
    } else {
        for (QGraphicsItem* item : App::graphicsView()->scene()->items())
            if (item->isVisible() && item->opacity() > 0)
                item->setSelected(true);
    }
}

void MainWindow::deSelectAll() {
    for (QGraphicsItem* item : App::graphicsView()->scene()->items())
        if (item->isVisible())
            item->setSelected(false);
}

void MainWindow::printDialog() {
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, [](QPrinter* pPrinter) {
        // FIXME       ScopedTrue sTrue(App::graphicsView()->scene()->drawPdf_);
        QRectF rect;
        for (QGraphicsItem* item : App::graphicsView()->scene()->items())
            if (item->isVisible() && !item->boundingRect().isNull())
                rect |= item->boundingRect();
        QSizeF size(rect.size());

        QMarginsF margins(10, 10, 10, 10);
        pPrinter->setPageMargins(margins);
        pPrinter->setPageSize(QPageSize(size + QSizeF(margins.left() + margins.right(), margins.top() + margins.bottom()), QPageSize::Millimeter));

        pPrinter->setResolution(4800);

        QPainter painter(pPrinter);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setTransform(QTransform().scale(1.0, -1.0));
        painter.translate(0, -(pPrinter->resolution() / 25.4) * size.height());
        App::graphicsView()->scene()->render(&painter,
            QRectF(0, 0, pPrinter->width(), pPrinter->height()),
            rect,
            Qt::KeepAspectRatio /*IgnoreAspectRatio*/);
    });
    preview.exec();
}

void MainWindow::fileProgress(const QString& fileName, int max, int value) {
    if (max && !value) {
        QProgressDialog* pd = new QProgressDialog(this);
        pd->setCancelButton(nullptr);
        pd->setLabelText(fileName);
        pd->setMaximum(max);
        // pd->setModal(true);
        // pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
        pd->show();
        progressDialogs_[fileName] = pd;
    } else if (max == 1 && value == 1) {
        progressDialogs_[fileName]->hide();
        progressDialogs_[fileName]->deleteLater();
        progressDialogs_.remove(fileName);
    } else
        progressDialogs_[fileName]->setValue(value);
}

void MainWindow::fileError(const QString& fileName, const QString& error) {
    qWarning() << "fileError " << fileName << error;

    static QDialog* fileErrordialog;
    static QTextBrowser* textBrowser;

    if (!fileErrordialog) {
        fileErrordialog = new QDialog(this);
        fileErrordialog->setObjectName(QString::fromUtf8("dialog"));
        fileErrordialog->setWindowTitle(tr("File open errors"));

        fileErrordialog->resize(600, 600);

        auto verticalLayout = new QVBoxLayout(fileErrordialog);
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));

        textBrowser = new QTextBrowser(fileErrordialog);
        textBrowser->setObjectName(QString::fromUtf8("textBrowser"));

        verticalLayout->addWidget(textBrowser);
        verticalLayout->setContentsMargins(6, 6, 6, 6);
        verticalLayout->setSpacing(6);
    }
    fileErrordialog->show();
    textBrowser->setTextColor(Qt::black);
    textBrowser->append(fileName);
    textBrowser->setTextColor(Qt::darkRed);
    textBrowser->append(error);
    textBrowser->append("");
}

void MainWindow::resetToolPathsActions() {
    delete dockWidget_->widget();
    dockWidget_->setWidget(nullptr);
    dockWidget_->setVisible(false);
    if (auto action {actionGroup.checkedAction()}; action)
        action->setChecked(false);
}

void MainWindow::documentWasModified() { setWindowModified(project_->isModified()); }

bool MainWindow::maybeSave() {
    if (!project_->isModified() && project_->size()) {
        return QMessageBox::warning(this, tr("Warning"), tr("Do you want to close this project?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok;
    } else if (!project_->size()) {
        return true;
    }

    const QMessageBox::StandardButton ret
        = QMessageBox::warning(this, tr("Warning"), tr("The document has been modified.\n"
                                                       "Do you want to save your changes?"),
            QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    switch (ret) {
    case QMessageBox::Save:
        return save();
    case QMessageBox::Cancel:
        return false;
    default:
        break;
    }
    return true;
}

void MainWindow::editGcFile(GCode::File* file) {
    switch (file->gtype()) {
    case GCode::Null:
    case GCode::Profile:
        // FIXME toolpathActions[GCode::Profile]->triggered();
        // FIXME reinterpret_cast<FormsUtil*>(dockWidget_->widget())->editFile(file);
        break;
    case GCode::Pocket:
    case GCode::Voronoi:
    case GCode::Thermal:
    case GCode::Drill:
    case GCode::GCodeProperties:
    case GCode::Raster:
    case GCode::LaserHLDI:
    default:
        break;
    }
}

bool MainWindow::saveFile(const QString& fileName) {
    bool ok;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (ok = project_->save(fileName); ok) {
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved"), 2000);
    } else {
        QMessageBox::warning(this,
            tr("Warning"),
            tr("Cannot write file %1:\n%2.")
                .arg(QDir::toNativeSeparators(fileName))
                .arg("file.errorString()"));
    }
    QApplication::restoreOverrideCursor();

    return ok;
}

void MainWindow::setCurrentFile(const QString& fileName) {
    project_->setName(fileName);
    project_->setModified(false);
    setWindowModified(false);
    if (!project_->isUntitled())
        recentProjects.prependToRecentFiles(project_->name());
    closeAllAct_->setText(tr(R"(&Close project "%1")").arg(strippedName(project_->name())));
    setWindowFilePath(project_->name());
}

void MainWindow::addFileToPro(FileInterface* file) {
    if (project_->isUntitled()) {
        QString name(QFileInfo(file->name()).path());
        setCurrentFile(name + "/" + name.split('/').back() + ".g2g");
    }
    project_->addFile(file);
    recentFiles.prependToRecentFiles(file->name());
    ui.graphicsView->zoomFit();
}

QString MainWindow::strippedName(const QString& fullFileName) {
    return QFileInfo(fullFileName).fileName();
}

QMenu* MainWindow::createPopupMenu() {
    QMenu* menu = QMainWindow::createPopupMenu();
    menu->removeAction(dockWidget_->toggleViewAction());
    menu->removeAction(toolpathToolBar->toggleViewAction());
    menu->removeAction(ui.treeDockWidget->toggleViewAction());

    menu->addAction(tr("Icon size = 24"), [this]() { setIconSize(QSize(24, 24)); });
    menu->addAction(tr("Icon size = 48"), [this]() { setIconSize(QSize(48, 48)); });
    menu->addAction(tr("Icon size = 72"), [this]() { setIconSize(QSize(72, 72)); });

    qDebug() << menu->parent();

    return menu;
}

const QDockWidget* MainWindow::dockWidget() const { return dockWidget_; }

QDockWidget* MainWindow::dockWidget() { return dockWidget_; }

void MainWindow::translate(const QString& locale) {
    static std::vector<std::unique_ptr<QTranslator>> translators;
    translators.clear();
    QDir dir(qApp->applicationDirPath() + "/translations");
    for (auto&& str : dir.entryList(QStringList {"*" + locale + ".qm"}, QDir::Files)) {
        translators.emplace_back(std::make_unique<QTranslator>());
        if (translators.back()->load(str, dir.path()))
            qApp->installTranslator(translators.back().get());
    }
}

void MainWindow::loadFile(const QString& fileName) {
    if (!QFile(fileName).exists())
        return;
    lastPath = QFileInfo(fileName).absolutePath();
    if (fileName.endsWith(".g2g")) {
        if (closeProject()) {
            project_->open(fileName);
            setCurrentFile(fileName);
            QTimer::singleShot(100, Qt::CoarseTimer, ui.graphicsView, &GraphicsView::zoomFit);
            return;
        }
    } else {
        if (project_->contains(fileName) > -1 && QMessageBox::question(this, tr("Warning"), //
                                                     tr("Do you want to reload file %1?").arg(QFileInfo(fileName).fileName()), QMessageBox::Ok | QMessageBox::Cancel)
                == QMessageBox::Cancel)
            return;
        for (auto& [type, ptr] : App::filePlugins()) {
            if (ptr->thisIsIt(fileName)) {
                emit parseFile(fileName, int(type));
                return;
            }
        }
    }
    qDebug() << fileName;
}

void MainWindow::updateTheme() {

    if (App::settings().theme()) {
        qApp->setStyle(QStyleFactory::create("Fusion"));

        QColor baseColor;
        QColor disabledColor;
        QColor highlightColor;
        QColor linkColor;
        QColor windowColor;
        QColor windowTextColor;

        switch (App::settings().theme()) {
        case LightBlue:
            baseColor = QColor(230, 230, 230);
            disabledColor = QColor(127, 127, 127);
            highlightColor = QColor(61, 174, 233);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(200, 200, 200);
            windowTextColor = QColor(0, 0, 0);
            break;
        case LightRed:
            baseColor = QColor(230, 230, 230);
            disabledColor = QColor(127, 127, 127);
            highlightColor = QColor(218, 68, 83);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(200, 200, 200);
            windowTextColor = QColor(0, 0, 0);
            break;
        case DarkBlue:
            baseColor = QColor(40, 40, 40);
            disabledColor = QColor(60, 60, 60);
            highlightColor = QColor(61, 174, 233);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(60, 60, 60);
            windowTextColor = QColor(220, 220, 220);
            break;
        case DarkRed:
            baseColor = QColor(40, 40, 40);
            disabledColor = QColor(60, 60, 60);
            highlightColor = QColor(218, 68, 83);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(60, 60, 60);
            windowTextColor = QColor(220, 220, 220);
            break;
        }

        QPalette palette;

        palette.setBrush(QPalette::Text, windowTextColor);
        palette.setBrush(QPalette::ToolTipText, windowTextColor);
        palette.setBrush(QPalette::WindowText, windowTextColor);
        palette.setBrush(QPalette::ButtonText, windowTextColor);
        palette.setBrush(QPalette::HighlightedText, Qt::black);
        palette.setBrush(QPalette::BrightText, Qt::red);

        palette.setBrush(QPalette::Link, linkColor);
        palette.setBrush(QPalette::LinkVisited, highlightColor);

        palette.setBrush(QPalette::AlternateBase, windowColor);
        palette.setBrush(QPalette::Base, baseColor);
        palette.setBrush(QPalette::Button, windowColor);

        palette.setBrush(QPalette::Highlight, highlightColor);

        palette.setBrush(QPalette::ToolTipBase, windowTextColor);
        palette.setBrush(QPalette::Window, windowColor);

        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        palette.setBrush(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
        palette.setBrush(QPalette::Disabled, QPalette::Text, disabledColor);
        palette.setBrush(QPalette::Disabled, QPalette::Shadow, disabledColor);

        //        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, disabledColor);
        //        palette.setBrush(QPalette::Inactive, QPalette::HighlightedText, disabledColor);
        //        palette.setBrush(QPalette::Inactive, QPalette::Text, disabledColor);
        //        palette.setBrush(QPalette::Inactive, QPalette::Shadow, disabledColor);

        qApp->setPalette(palette);
    } else {

        //        QPalette palette;

        //        palette.setCurrentColorGroup(QPalette::Active);

        //        palette.setBrush(QPalette::AlternateBase, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Base, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::BrightText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Button, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ButtonText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Dark, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Highlight, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::HighlightedText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Light, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Link, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::LinkVisited, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Mid, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Midlight, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::NoRole, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::PlaceholderText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Shadow, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Text, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ToolTipBase, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ToolTipText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Window, QColor(0, 0, 0, 255));

        //        palette.setCurrentColorGroup(QPalette::Disabled);
        //        palette.setBrush(QPalette::AlternateBase, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Base, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::BrightText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Button, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ButtonText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Dark, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Highlight, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::HighlightedText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Light, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Link, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::LinkVisited, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Mid, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Midlight, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::NoRole, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::PlaceholderText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Shadow, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Text, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ToolTipBase, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ToolTipText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Window, QColor(0, 0, 0, 255));

        //        palette.setCurrentColorGroup(QPalette::Inactive);
        //        palette.setBrush(QPalette::AlternateBase, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Base, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::BrightText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Button, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ButtonText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Dark, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Highlight, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::HighlightedText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Light, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Link, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::LinkVisited, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Mid, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Midlight, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::NoRole, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::PlaceholderText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Shadow, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Text, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ToolTipBase, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::ToolTipText, QColor(0, 0, 0, 255));
        //        palette.setBrush(QPalette::Window, QColor(0, 0, 0, 255));

        //        enum ColorRole { WindowText,
        //            Button,
        //            Light,
        //            Midlight,
        //            Dark,
        //            Mid,
        //            Text,
        //            BrightText,
        //            ButtonText,
        //            Base,
        //            Window,
        //            Shadow,
        //            Highlight,
        //            HighlightedText,
        //            Link,
        //            LinkVisited,
        //            AlternateBase,
        //            NoRole,
        //            ToolTipBase,
        //            ToolTipText,
        //            PlaceholderText,
        //        };

        //        palette.setBrush(QPalette::Disabled, QPalette::AlternateBase, brush13);
        //        palette.setBrush(QPalette::Disabled, QPalette::Base, brush01);
        //        palette.setBrush(QPalette::Disabled, QPalette::BrightText, brush00);
        //        palette.setBrush(QPalette::Disabled, QPalette::Button, brush01);
        //        palette.setBrush(QPalette::Disabled, QPalette::ButtonText, brush00);
        //        palette.setBrush(QPalette::Disabled, QPalette::Dark, brush04);
        //        palette.setBrush(QPalette::Disabled, QPalette::Light, brush02);
        //        palette.setBrush(QPalette::Disabled, QPalette::Mid, brush05);
        //        palette.setBrush(QPalette::Disabled, QPalette::Midlight, brush03);
        //        palette.setBrush(QPalette::Disabled, QPalette::Shadow, brush07);
        //        palette.setBrush(QPalette::Disabled, QPalette::Text, brush00);
        //        palette.setBrush(QPalette::Disabled, QPalette::ToolTipBase, brush08);
        //        palette.setBrush(QPalette::Disabled, QPalette::ToolTipText, brush07);
        //        palette.setBrush(QPalette::Disabled, QPalette::Window, brush01);
        //        palette.setBrush(QPalette::Disabled, QPalette::WindowText, brush07);

        //        palette.setBrush(QPalette::Inactive, QPalette::AlternateBase, brush13);
        //        palette.setBrush(QPalette::Inactive, QPalette::Base, brush00);
        //        palette.setBrush(QPalette::Inactive, QPalette::BrightText, brush00);
        //        palette.setBrush(QPalette::Inactive, QPalette::Button, brush09);
        //        palette.setBrush(QPalette::Inactive, QPalette::ButtonText, brush07);
        //        palette.setBrush(QPalette::Inactive, QPalette::Dark, brush11);
        //        palette.setBrush(QPalette::Inactive, QPalette::Light, brush00);
        //        palette.setBrush(QPalette::Inactive, QPalette::Mid, brush11);
        //        palette.setBrush(QPalette::Inactive, QPalette::Midlight, brush10);
        //        palette.setBrush(QPalette::Inactive, QPalette::Shadow, brush12);
        //        palette.setBrush(QPalette::Inactive, QPalette::Text, brush07);
        //        palette.setBrush(QPalette::Inactive, QPalette::ToolTipBase, brush08);
        //        palette.setBrush(QPalette::Inactive, QPalette::ToolTipText, brush07);
        //        palette.setBrush(QPalette::Inactive, QPalette::Window, brush09);
        //        palette.setBrush(QPalette::Inactive, QPalette::WindowText, brush07);
        //        qApp->setPalette(palette);
    }

    // if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows && QOperatingSystemVersion::current().majorVersion() > 7) {
    // App::mainWindow()->setStyleSheet("QGroupBox, .QFrame {"
    // //"background-color: white;"
    // "border: 1px solid gray; }"
    // "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
    // "QGroupBox::title {"
    // "subcontrol-origin: margin;"
    // "subcontrol-position: top center; }" /* position at the top center */
    // );
    // } else {
    // App::mainWindow()->setStyleSheet("QGroupBox, .QFrame {"
    // //"background-color: white;"
    // "border: 1px solid gray;"
    // "border-radius: 3px; }" // Win 7 or other
    // "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
    // "QGroupBox::title {"
    // "subcontrol-origin: margin;"
    // "subcontrol-position: top center; }" /* position at the top center */
    // );
    // }

    QIcon::setThemeName(App::settings().theme() < DarkBlue ? "ggeasy-light" : "ggeasy-dark");
    if (App::mainWindow() && App::mainWindow()->isVisible())
        SettingsDialog().show();
}

void MainWindow::setDockWidget(QWidget* dwContent) {
    if (!dwContent)
        exit(-66);

    delete dockWidget_->widget();
    dockWidget_->setWidget(dwContent);
    dockWidget_->setWindowTitle(dwContent->windowTitle());
    if (auto pbClose {dwContent->findChild<QPushButton*>("pbClose")}; pbClose)
        connect(pbClose, &QPushButton::clicked, this, &MainWindow::resetToolPathsActions);
    dockWidget_->show();
}

void MainWindow::open() {
    QStringList files(QFileDialog::getOpenFileNames(
        this,
        tr("Open File"),
        lastPath,
        tr("Any (*.*);;Gerber/Excellon (*.gbr *.exc *.drl);;Project (*.g2g)")));
    if (files.isEmpty())
        return;

    if (files.filter(QRegularExpression(".+g2g$")).size()) {
        loadFile(files.at(files.indexOf(QRegularExpression(".+g2g$"))));
        return;
    } else {
        for (QString& fileName : files) {
            loadFile(fileName);
        }
    }
    // QString name(QFileInfo(files.first()).path());
    // setCurrentFile(name + "/" + name.split('/').back() + ".g2g");
}

bool MainWindow::save() {
    if (project_->isUntitled())
        return saveAs();
    else
        return saveFile(project_->name());
}

bool MainWindow::saveAs() {
    QString file(
        QFileDialog::getSaveFileName(this, tr("Open File"), project_->name(), tr("Project (*.g2g)")));
    if (file.isEmpty())
        return false;
    return saveFile(file);
}

void MainWindow::showEvent(QShowEvent* event) {
    // toolpathActionList[GCode::GCodeProperties]->trigger();//////////////////////////////////////////////////////
    QMainWindow::showEvent(event);
}

void MainWindow::changeEvent(QEvent* event) {
    // В случае получения события изменения языка приложения
    if (event->type() == QEvent::LanguageChange)
        ui.retranslateUi(this); // переведём окно заново
}

bool MainWindow::eventFilter(QObject* watched, QEvent* event) {
    if (0 && watched == menuBar()) {
        static QPoint pt;
        auto mEvent = reinterpret_cast<QMouseEvent*>(event);
        switch (event->type()) {
        case QEvent::MouseMove:
            if (!pt.isNull())
                setGeometry(geometry().translated(mEvent->pos() - pt));
            break;
        case QEvent::MouseButtonPress:
            pt = mEvent->pos();
            break;
        case QEvent::MouseButtonRelease:
            pt = {};
            break;
        default:;
        }
    }
    if (watched == dockWidget_ && event->type() == QEvent::Close) {
        resetToolPathsActions();
    }
    return QMainWindow::eventFilter(watched, event);
}

void MainWindow::Ui::setupUi(QMainWindow* MainWindow) {
    if (MainWindow->objectName().isEmpty())
        MainWindow->setObjectName(QString::fromUtf8("MainWindow"));
    MainWindow->resize(1600, 1000);
    MainWindow->setWindowTitle(QString::fromUtf8("[*] GGEasy"));
    MainWindow->setDockOptions(QMainWindow::AllowTabbedDocks);
    centralwidget = new QWidget(MainWindow);
    centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
    horizontalLayout = new QHBoxLayout(centralwidget);
    horizontalLayout->setSpacing(0);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(3, 3, 3, 3);
    graphicsView = new GraphicsView(centralwidget);
    graphicsView->setObjectName(QString::fromUtf8("graphicsView"));
    graphicsView->setTransformationAnchor(QGraphicsView::AnchorUnderMouse);
    graphicsView->setResizeAnchor(QGraphicsView::AnchorUnderMouse);

    horizontalLayout->addWidget(graphicsView);

    MainWindow->setCentralWidget(centralwidget);
    menubar = new QMenuBar(MainWindow);
    menubar->setObjectName(QString::fromUtf8("menubar"));
    menubar->setGeometry(QRect(0, 0, 1600, 26));
    MainWindow->setMenuBar(menubar);
    statusbar = new QStatusBar(MainWindow);
    statusbar->setObjectName(QString::fromUtf8("statusbar"));
    MainWindow->setStatusBar(statusbar);
    treeDockWidget = new QDockWidget(MainWindow);
    treeDockWidget->setObjectName(QString::fromUtf8("treeDockWidget"));
    treeDockWidget->setMinimumSize(QSize(100, 119));
    treeDockWidget->setFeatures(QDockWidget::DockWidgetFloatable | QDockWidget::DockWidgetMovable);
    treeDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    widget = new QWidget();
    widget->setObjectName(QString::fromUtf8("widget"));
    verticalLayout = new QVBoxLayout(widget);
    verticalLayout->setSpacing(6);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
    verticalLayout->setContentsMargins(3, 3, 3, 3);
    treeView = new FileTree::View(widget);
    treeView->setObjectName(QString::fromUtf8("treeView"));
    treeView->setSelectionMode(QAbstractItemView::ExtendedSelection);

    verticalLayout->addWidget(treeView);

    treeDockWidget->setWidget(widget);
    MainWindow->addDockWidget(Qt::LeftDockWidgetArea, treeDockWidget);

    retranslateUi(MainWindow);

    QMetaObject::connectSlotsByName(MainWindow);
}

void MainWindow::Ui::retranslateUi(QMainWindow* MainWindow) {
    treeDockWidget->setWindowTitle(QCoreApplication::translate("MainWindow", "Files", nullptr));
    (void)MainWindow;
}

#include "moc_mainwindow.cpp"

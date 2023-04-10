// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com

/********************************************************************************
 * Author : Damir Bakiev                                                         *
 * Version : na                                                                  *
 * Date : 11 November 2021                                                       *
 * Website : na                                                                  *
 * Copyright : Damir Bakiev 2016-2023                                            *
 * License:                                                                      *
 * Use, modification & distribution is subject to Boost Software License Ver 1.  *
 * http://www.boost.org/LICENSE_1_0.txt                                          *
 ********************************************************************************/
// #include "a_pch.h"
#include "mainwindow.h"

#include "aboutform.h"
#include "abstract_file.h"
#include "abstract_fileplugin.h"
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

#include <QPdfWriter>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QtWidgets>

static auto PointConverter = QMetaType::registerConverter(&Point::toString);

inline constexpr auto G_CODE_PROPERTIES = md5::hash32("GCodeProperties");

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

    GCode::PropertiesForm(); // init default vars;

    connect(project_, &Project::homePosChanged, App::homePtr(), qOverload<const QPointF&>(&GiMarker::setPos));
    connect(project_, &Project::zeroPosChanged, App::zeroPtr(), qOverload<const QPointF&>(&GiMarker::setPos));
    connect(project_, &Project::pinsPosChanged, qOverload<const QPointF[4]>(&GiPin::setPos));
    connect(project_, &Project::layoutFrameUpdate, lfp, &LayoutFrames::updateRect);
    connect(project_, &Project::changed, this, &MainWindow::documentWasModified);
    menuBar()->installEventFilter(this);

    // connect plugins
    for (auto& [type, ptr] : App::filePlugins()) {
        //        if (ptr->type() == int(FileType::GCode_))
        //            continue;
        ptr->moveToThread(&parserThread);
        connect(ptr, &AbstractFilePlugin::fileError, this, &MainWindow::fileError, Qt::QueuedConnection);
        connect(ptr, &AbstractFilePlugin::fileProgress, this, &MainWindow::fileProgress, Qt::QueuedConnection);
        connect(ptr, &AbstractFilePlugin::fileReady, this, &MainWindow::addFileToPro, Qt::QueuedConnection);
        connect(this, &MainWindow::parseFile, ptr, &AbstractFilePlugin::parseFile, Qt::QueuedConnection);
        connect(project_, &Project::parseFile, ptr, &AbstractFilePlugin::parseFile, Qt::QueuedConnection);
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
    toolpathActions[G_CODE_PROPERTIES]->triggered();

    debug();
}

MainWindow::~MainWindow() {
    parserThread.quit();
    parserThread.wait();
    App::setMainWindow(nullptr);
}

void MainWindow::closeEvent(QCloseEvent* event) {
    if (App::isDebug() || maybeSave()) {
        delete dockWidget_;
        writeSettings();
        App::fileModel().closeProject();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::closeProject() {
    if (maybeSave()) {
        dockWidget_->close();
        App::fileModel().closeProject();
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
    // dockWidget_->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);
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
    action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), this, &MainWindow::renderPdf);
    action->setStatusTip(tr("Export to PDF file"));
    fileToolBar->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), this, &MainWindow::renderPdf);

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
    connect(action, &QAction::toggled, this, [=, this](bool checked) { if (checked) setDockWidget(new GCode::PropertiesForm); });
    action->setShortcut(QKeySequence("Ctrl+Shift+G"));
    action->setCheckable(true);
    toolpathActions.emplace(G_CODE_PROPERTIES, action);
    actionGroup.addAction(action);
    // Tool Base
    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"), [this] { ToolDatabase(this, {}).exec(); }));
    // Separator
    serviceMenu->addAction(toolpathToolBar->addSeparator());
    // Autoplace All Refpoints
    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Autoplace All Refpoints"), [this] {
        if (updateRect()) {
            GiPin::resetPos(false);
            App::home().resetPos(false);
            App::zero().resetPos(false);
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
    if (App::isDebug()) { // (need for debug)
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
    action = helpMenu->addAction(tr("About &Plugins…"), this, [this] { DialogAboutPlugins(this).exec(); });
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
        connect(gCodePlugin, &GCode::Plugin::setDockWidget, this, &MainWindow::setDockWidget);
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

        auto selectedItems(App::graphicsView().selectedItems());
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
                clipper.AddSubject(gitem->paths());
                clipper.AddClip(clipPaths);
                Paths paths;
                clipper.Execute(type, FillRule::EvenOdd, paths /*FillRule::Positive*/);
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
    toolBar->addAction(QIcon::fromTheme("path-union"), tr("Union"), [executor] { executor(ClipType::Union); });
    toolBar->addAction(QIcon::fromTheme("path-difference"), tr("Difference"), [executor] { executor(ClipType::Difference); });
    toolBar->addAction(QIcon::fromTheme("path-exclusion"), tr("Exclusion"), [executor] { executor(ClipType::Xor); });
    toolBar->addAction(QIcon::fromTheme("path-intersection"), tr("Intersection"), [executor] { executor(ClipType::Intersection); });
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

void MainWindow::saveGCodeFile(int32_t id) {
    if (project_->pinsPlacedMessage())
        return;
    auto* file = project_->file<GCode::File>(id);
    QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
        GCode::File::getLastDir().append(file->shortName()),
        tr("GCode (*.%1)").arg(App::gcSettings().fileExtension())));

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
                QString name(GCode::File::getLastDir().append(file->shortName()));
                if (!name.endsWith(App::gcSettings().fileExtension()))
                    name += QStringList({"_TS", "_BS"})[file->side()];

                name = QFileDialog::getSaveFileName(nullptr,
                    QObject::tr("Save GCode file"),
                    name,
                    QObject::tr("GCode (*.%1)").arg(App::gcSettings().fileExtension()));

                if (name.isEmpty())
                    return;
                file->save(name);
                file->itemGroup()->setVisible(false);
            }
        } else {
            QString name(GCode::File::getLastDir().append(files.first()->getTool().nameEnc()));
            if (!name.endsWith(App::gcSettings().fileExtension()))
                name += QStringList({"_TS", "_BS"})[files.first()->side()];

            name = QFileDialog::getSaveFileName(nullptr,
                QObject::tr("Save GCode file"),
                name,
                QObject::tr("GCode (*.%1)").arg(App::gcSettings().fileExtension()));

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

    if (App::isDebug())
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
    auto data {actionGroup.checkedAction() ? actionGroup.checkedAction()->data() : QVariant {}};
    if (!data.isNull() && data.toBool()) {
        for (QGraphicsItem* item : App::graphicsView().items(GiType::Preview))
            item->setSelected(true);
    } else {
        for (QGraphicsItem* item : App::graphicsView().items())
            if (item->isVisible() && item->opacity() > 0)
                item->setSelected(true);
    }
}

void MainWindow::deSelectAll() {
    for (QGraphicsItem* item : App::graphicsView().items())
        if (item->isVisible())
            item->setSelected(false);
}

void MainWindow::printDialog() {
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, [](QPrinter* pPrinter) {
        // ScopedTrue sTrue(App::app_->drawPdf_);
        // NOTE App::setDrawPdf(true);
        QRectF rect;
        for (QGraphicsItem* item : App::graphicsView().items())
            if (item->isVisible() && !item->boundingRect().isNull())
                rect |= item->boundingRect();
        QSizeF size(rect.size());

        QMarginsF margins(10, 10, 10, 10);
        QSizeF mSize(margins.left() + margins.right(), margins.top() + margins.bottom());
        pPrinter->setPageMargins(margins);
        pPrinter->setPageSize(QPageSize(size + mSize, QPageSize::Millimeter));
        pPrinter->setResolution(4800);
        QPainter painter(pPrinter);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setTransform(QTransform().scale(1.0, -1.0));
        painter.translate(0, -(pPrinter->resolution() / 25.4) * size.height());
        App::graphicsView().scene()->render(&painter,
            QRectF(0, 0, pPrinter->width(), pPrinter->height()),
            rect, Qt::KeepAspectRatio /*IgnoreAspectRatio*/);
        // NOTE App::setDrawPdf(false);
    });
    preview.exec();
}

void MainWindow::renderPdf() {
    QString curFile = QFileDialog::getSaveFileName(nullptr, tr("Save PDF file"), lastPath, tr("File(*.pdf)"));
    if (curFile.isEmpty())
        return;

    App::setDrawPdf(true);

    QRectF rect;
    for (QGraphicsItem* item : ui.graphicsView->scene()->items())
        if (item->isVisible() && !item->boundingRect().isNull())
            rect |= item->boundingRect();

    //    QRectF rect(ui.graphicsView->scene()->itemsBoundingRect());

    //    QRectF rect {App::layoutFrames().boundingRect()};

    QSizeF size(rect.size());

    QPdfWriter pdfWriter(curFile);
    //    pdfWriter.setPageSizeMM(size);
    //    pdfWriter.setMargins({0, 0, 0, 0});
    //    pdfWriter.setResolution(1000000);
    QMarginsF margins(10, 10, 10, 10);
    QSizeF mSize(margins.left() + margins.right(), margins.top() + margins.bottom());
    pdfWriter.setPageMargins(margins);
    pdfWriter.setPageSize(QPageSize(size + mSize, QPageSize::Millimeter));
    pdfWriter.setResolution(4800);

    QPainter painter(&pdfWriter);
    painter.setTransform(QTransform().scale(1.0, -1.0));
    painter.translate(0, -(pdfWriter.resolution() / 25.4) * size.height());
    ui.graphicsView->scene()->render(&painter,
        QRectF(0, 0, pdfWriter.width(), pdfWriter.height()),
        rect, Qt::IgnoreAspectRatio);

    App::setDrawPdf(false);
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

void MainWindow::editGcFile(GCode::File* file) { // TODO editGcFile
    qWarning(__FUNCTION__);
    // TODO   switch (file->gtype()) {
    //    case GCode::Null:
    //    case md5::hash32("Profile"):
    //        // toolpathActions[md5::hash32("Profile")]->triggered();
    //        // reinterpret_cast<FormsUtil*>(dockWidget_->widget())->editFile(file);
    //        break;
    //    case GCode::Pocket:
    //    case GCode::Voronoi:
    //    case GCode::Thermal:
    //    case GCode::Drill:
    //    case G_CODE_PROPERTIES:
    //    case GCode::Raster:
    //    case GCode::LaserHLDI:
    //    default:
    //        break;
    //    }
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
    if (fileName.isEmpty())
        setWindowTitle(tr("Untitled") + ".g2g[*]");
    project_->setName(fileName);
    project_->setModified(false);
    setWindowModified(false);
    if (!project_->isUntitled())
        recentProjects.prependToRecentFiles(project_->name());
    closeAllAct_->setText(tr(R"(&Close project "%1")").arg(strippedName(project_->name())));
    setWindowFilePath(project_->name());
}

void MainWindow::addFileToPro(AbstractFile* file) {
    if (project_->isUntitled()) {
        QString name(QFileInfo(file->name()).path());
        setCurrentFile(name + "/" + name.split('/').back() + ".g2g");
    }
    project_->addFile(file);
    recentFiles.prependToRecentFiles(file->name());
    //    ui.graphicsView->zoomFit();
}

QString MainWindow::strippedName(const QString& fullFileName) {
    return QFileInfo(fullFileName).fileName();
}

QMenu* MainWindow::createPopupMenu() {
    QMenu* menu = QMainWindow::createPopupMenu();
    menu->removeAction(dockWidget_->toggleViewAction());
    menu->removeAction(toolpathToolBar->toggleViewAction());
    menu->removeAction(ui.treeDockWidget->toggleViewAction());

    menu->addAction(tr("Icon size = 24"), this, [this]() { setIconSize(QSize(24, 24)); });
    menu->addAction(tr("Icon size = 48"), this, [this]() { setIconSize(QSize(48, 48)); });
    menu->addAction(tr("Icon size = 72"), this, [this]() { setIconSize(QSize(72, 72)); });

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

#if __has_include("xrstyle.h")
    #include "xrstyle.h"
#endif

void MainWindow::updateTheme() {
    if (App::settings().theme()) {

        static const char* const dwCloseXpm[] = {
            "11 13 3 1",
            "  c None",
            "@ c #6C6A67",
            "$ c #B5B0AC",
            "           ",
            "           ",
            "           ",
            "  $@   @$  ",
            "  @@@ @@@  ",
            "   @@@@@   ",
            "    @@@    ",
            "   @@@@@   ",
            "  @@@ @@@  ",
            "  $@   @$  ",
            "           ",
            "           ",
            "           ",
        };

        static const char* const dwRestoreXpm[] = {
            "11 13 3 1",
            "  c None",
            "@ c #6C6A67",
            "# c #ABA6A3",
            "           ",
            "           ",
            "           ",
            "    #@@@#  ",
            "    @   @  ",
            "  #@@@# @  ",
            "  @   @ @  ",
            "  @   @@@  ",
            "  @   @    ",
            "  #@@@#    ",
            "           ",
            "           ",
            "           ",
        };

        static const char* const dwMinimizeXpm[] = {
            "11 13 2 1",
            "  c None",
            "@ c #6C6A67",
            "           ",
            "           ",
            "           ",
            "           ",
            "           ",
            "           ",
            "  @@@@@@@  ",
            "  @@@@@@@  ",
            "           ",
            "           ",
            "           ",
            "           ",
            "           ",
        };

        static const char* const qtTitlebarContextHelp[] = {
            "10 10 3 1",
            "  c None",
            "# c #000000",
            "+ c #444444",
            "  +####+  ",
            " ###  ### ",
            " ##    ## ",
            "     +##+ ",
            "    +##   ",
            "    ##    ",
            "    ##    ",
            "          ",
            "    ##    ",
            "    ##    ",
        };

        class Style : public QProxyStyle {
        public:
            Style()
                : QProxyStyle("Fusion") { }

            QPixmap getPixmap(StandardPixmap standardPixmap) const {
                switch (standardPixmap) {
                case SP_TitleBarNormalButton:
                    return QPixmap(dwRestoreXpm);
                case SP_TitleBarMinButton:
                    return QPixmap(dwMinimizeXpm);
                case SP_TitleBarCloseButton:
                case SP_DockWidgetCloseButton:
                    return QPixmap(dwCloseXpm);
                default:
                    return {};
                }
            }

            QIcon standardIcon(StandardPixmap standardIcon, const QStyleOption* option, const QWidget* widget) const override {
                if (QPixmap pix = getPixmap(standardIcon); !pix.isNull())
                    return QIcon(pix);
                return QProxyStyle::standardIcon(standardIcon, option, widget);
            }
            QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* opt, const QWidget* widget) const override {
                if (QPixmap pix = getPixmap(standardPixmap); !pix.isNull())
                    return pix;
                return QProxyStyle::standardPixmap(standardPixmap, opt, widget);
            }
        };

        qApp->setStyle(new Style);

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
            baseColor = QColor(20, 20, 20);
            disabledColor = QColor(100, 100, 100);
            highlightColor = QColor(61, 174, 233);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(30, 30, 30);
            windowTextColor = QColor(220, 220, 220);
            break;
        case DarkRed:
            baseColor = QColor(20, 20, 20);
            disabledColor = QColor(100, 100, 100);
            highlightColor = QColor(218, 68, 83);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(30, 30, 30);
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
#if __has_include("xrstyle.h")
        QApplication::setStyle(new XrStyle);
#else
        qApp->setStyle(QStyleFactory::create("Fusion"));
        qApp->setPalette(QApplication::style()->standardPalette());
#endif
    }

    // if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows && QOperatingSystemVersion::current().majorVersion() > 7) {
    // App::mainWindow().setStyleSheet("QGroupBox, .QFrame {"
    // //"background-color: white;"
    // "border: 1px solid gray; }"
    // "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
    // "QGroupBox::title {"
    // "subcontrol-origin: margin;"
    // "subcontrol-position: top center; }" /* position at the top center */
    // );
    // } else {
    // App::mainWindow().setStyleSheet("QGroupBox, .QFrame {"
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
    if (App::mainWindowPtr() && App::mainWindow().isVisible())
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
    // toolpathActionList[G_CODE_PROPERTIES]->trigger();//////////////////////////////////////////////////////
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

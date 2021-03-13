// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "aboutform.h"
#include "forms/drillform/drillform.h"
#include "forms/gcodepropertiesform.h"
#include "forms/hatchingform.h"
#include "forms/pocketoffsetform.h"
#include "forms/pocketrasterform.h"
#include "forms/profileform.h"
#include "forms/voronoiform.h"
#include "interfaces/shapepluginin.h"
#include "plugindialog.h"
#include "point.h"
#include "settingsdialog.h"

#include "project.h"
#include <thermalform.h>
#include <tooldatabase.h>

#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QtWidgets>
#include <bridgeitem.h>
#include <datasoliditem.h>
#include <forward_list>

#include "leakdetector.h"

bool operator<(const QPair<Tool, Side>& p1, const QPair<Tool, Side>& p2)
{
    return p1.first.hash() < p2.first.hash() || (!(p2.first.hash() < p1.first.hash()) && p1.second < p2.second);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , recentFiles(this, "recentFiles")
    , recentProjects(this, "recentProjects")
    , m_project(new Project(this))
    , toolpathActionGroup(this)
    , reloadQuestion(this)
{
    App::setMainWindow(this);

    ui->setupUi(this);

    initWidgets();

    ui->graphicsView->scene()->addItem(new Marker(Marker::Home));
    ui->graphicsView->scene()->addItem(new Marker(Marker::Zero));
    ui->graphicsView->scene()->addItem(new Pin());
    ui->graphicsView->scene()->addItem(new Pin());
    ui->graphicsView->scene()->addItem(new Pin());
    ui->graphicsView->scene()->addItem(new Pin());

    GCodePropertiesForm(); // init default vars;

    connect(m_project, &Project::homePosChanged, Marker::get(Marker::Home), qOverload<const QPointF&>(&Marker::setPos));
    connect(m_project, &Project::zeroPosChanged, Marker::get(Marker::Zero), qOverload<const QPointF&>(&Marker::setPos));
    connect(m_project, &Project::pinsPosChanged, qOverload<const QPointF[4]>(&Pin::setPos));
    connect(m_project, &Project::layoutFrameUpdate, new LayoutFrames(), &LayoutFrames::updateRect);
    connect(m_project, &Project::changed, this, &MainWindow::documentWasModified);

    // connect plugins
    for (auto& [type, pair] : App::filePlugins()) {
        auto& [parser, pobj] = pair;
        pobj->moveToThread(&parserThread);
        connect(pobj, SIGNAL(fileError(const QString&, const QString&)), this, SLOT(fileError(const QString&, const QString&)), Qt::QueuedConnection);
        connect(pobj, SIGNAL(fileProgress(const QString&, int, int)), this, SLOT(fileProgress(const QString&, int, int)), Qt::QueuedConnection);
        connect(pobj, SIGNAL(fileReady(FileInterface*)), this, SLOT(addFileToPro(FileInterface*)), Qt::QueuedConnection);
        connect(this, SIGNAL(parseFile(const QString&, int)), pobj, SLOT(parseFile(const QString&, int)), Qt::QueuedConnection);
        connect(m_project, SIGNAL(parseFile(const QString&, int)), pobj, SLOT(parseFile(const QString&, int)), Qt::QueuedConnection);
    }

    { // add dummy gcode plugin
        auto parser = new GCode::Plugin(this);
        PIF pi {
            static_cast<FilePluginInterface*>(parser),
            static_cast<QObject*>(parser)
        };
        App::filePlugins().emplace(parser->type(), pi);
    }

    parserThread.start(QThread::HighestPriority);

    connect(ui->graphicsView, &GraphicsView::fileDroped, this, &MainWindow::loadFile);

    // Shapes::Constructor
    connect(ui->graphicsView, &GraphicsView::mouseMove, &ShapePluginInterface::updateShape_);
    connect(ui->graphicsView, &GraphicsView::mouseClickR, &ShapePluginInterface::finalizeShape_);
    connect(ui->graphicsView, &GraphicsView::mouseClickL, &ShapePluginInterface::addShapePoint_);
    // status bar
    connect(ui->graphicsView, &GraphicsView::mouseMove, [this](const QPointF& point) {
        ui->statusbar->showMessage(QString("X = %1, Y = %2").arg(point.x()).arg(point.y()));
    });

    ui->treeView->setModel(new FileTree::Model(ui->treeView));

    connect(ui->treeView, &FileTree::View::saveGCodeFile, this, &MainWindow::saveGCodeFile);
    connect(ui->treeView, &FileTree::View::saveGCodeFiles, this, &MainWindow::saveGCodeFiles);
    connect(ui->treeView, &FileTree::View::saveSelectedGCodeFiles, this, &MainWindow::saveSelectedGCodeFiles);

    App::toolHolder().readTools();
    setCurrentFile(QString());

    readSettings();

    if (qApp->applicationDirPath().contains("GERBER_X3/bin")) { // (need for debug)
        int i = 0;
        int k = 100;

        if (0) {
            QDir dir("D:/Gerber Test Files/ГГГ");
            //QDir dir("D:/Gerber Test Files/CopperCAM/");
            //QDir dir("C:/Users/X-Ray/Documents/3018/CNC");
            //QDir dir("E:/PRO/Новая папка/en.stm32f746g-disco_gerber/gerber_B01");
            QStringList listFiles;
            if (dir.exists())
                listFiles = dir.entryList(QStringList { "*.*" }, QDir::Files);
            for (QString str : listFiles) {
                str = dir.path() + '/' + str;
                qDebug() << str;
                QTimer::singleShot(++i * k, [this, str] { loadFile(str); });
                //break;
            }
        }
        if (1) {
            QTimer::singleShot(++i * 200, [this] { selectAll(); });
            QTimer::singleShot(++i * 200, [this] { toolpathActions[GCode::Voronoi]->triggered(); });
            QTimer::singleShot(++i * 200, [this] { m_dockWidget->findChild<QPushButton*>("pbCreate")->click(); });
        }
    }
}

MainWindow::~MainWindow()
{
    parserThread.quit();
    parserThread.wait();
    App::setMainWindow(nullptr);
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (qApp->applicationDirPath().contains("GERBER_X3/bin") || maybeSave()) {
        writeSettings();
        delete m_dockWidget; //->close();
        qApp->closeAllWindows();
        App::fileModel()->closeProject();
        event->accept();
    } else {
        event->ignore();
    }
}

bool MainWindow::closeProject()
{
    if (maybeSave()) {
        m_dockWidget->close();
        App::fileModel()->closeProject();
        setCurrentFile(QString());
        m_project->close();
        return true;
    }
    return false;
}

void MainWindow::about()
{
    AboutForm a(this);
    a.exec();
}

void MainWindow::initWidgets()
{
    createActions();
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::createActions()
{
    m_dockWidget = new DockWidget(this);
    m_dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    m_dockWidget->setObjectName(QStringLiteral("dwCreatePath"));
    // fileMenu
    createActionsFile();
    // zoomToolBar
    createActionsZoom();
    // fileEdit    // Selection / Delete selected
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

void MainWindow::createActionsFile()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName(QStringLiteral("fileMenu"));

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setObjectName(QStringLiteral("fileToolBar"));
    fileToolBar->setToolTip(tr("File"));

    fileToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(fileToolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::customContextMenuForToolBar);

    { // New
        auto action = fileMenu->addAction(QIcon::fromTheme("project-development-new-template"), tr("&New project"), this, &MainWindow::newFile);
        action->setShortcuts(QKeySequence::New);
        action->setStatusTip(tr("Create a new file"));
        fileToolBar->addAction(QIcon::fromTheme("project-development-new-template"), tr("&New project"), this, &MainWindow::newFile);
    }
    { // Open
        auto action = fileMenu->addAction(QIcon::fromTheme("document-open"), tr("&Open..."), this, &MainWindow::open);
        action->setShortcuts(QKeySequence::Open);
        action->setStatusTip(tr("Open an existing file"));
        fileToolBar->addAction(QIcon::fromTheme("document-open"), tr("&Open..."), this, &MainWindow::open);
    }
    { // Save
        auto action = fileMenu->addAction(QIcon::fromTheme("document-save"), tr("&Save project"), this, &MainWindow::save);
        action->setShortcuts(QKeySequence::Save);
        action->setStatusTip(tr("Save the document to disk"));
        fileToolBar->addAction(QIcon::fromTheme("document-save"), tr("&Save project"), this, &MainWindow::save);
    }
    { // Save As
        auto action = fileMenu->addAction(QIcon::fromTheme("document-save-as"), tr("Save project &As..."), this, &MainWindow::saveAs);
        action->setShortcuts(QKeySequence::SaveAs);
        action->setStatusTip(tr("Save the document under a new name"));
        fileToolBar->addAction(QIcon::fromTheme("document-save-as"), tr("Save project &As..."), this, &MainWindow::saveAs);
    }
    { // Close project
        m_closeAllAct = fileMenu->addAction(QIcon::fromTheme("document-close"), tr("&Close project \"%1\""), this, &MainWindow::closeProject);
        m_closeAllAct->setShortcuts(QKeySequence::Close);
        m_closeAllAct->setStatusTip(tr("Close project"));
        // m_closeAllAct->setEnabled(false);
        fileToolBar->addAction(QIcon::fromTheme("document-close"), tr("&Close project \"%1\"").arg(""), this, &MainWindow::closeProject);
    }

    fileMenu->addSeparator();
    fileToolBar->addSeparator();

    { // Save Selected Tool Paths
        auto action = fileMenu->addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."), this, &MainWindow::saveSelectedGCodeFiles);
        action->setStatusTip(tr("Save selected toolpaths"));
        fileToolBar->addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."), this, &MainWindow::saveSelectedGCodeFiles);
    }
    { // Export PDF
        auto action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), App::scene(), &Scene::RenderPdf);
        action->setStatusTip(tr("Export to PDF file"));
        fileToolBar->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), App::scene(), &Scene::RenderPdf);
    }

    fileMenu->addSeparator();
    fileMenu->addSeparator();

    recentFiles.createMenu(fileMenu, tr("Recent Files..."));
    recentProjects.createMenu(fileMenu, tr("Recent Projects..."));

    fileMenu->addSeparator();
    {
        auto action = fileMenu->addAction(QIcon::fromTheme("document-print"), tr("P&rint"), this, &MainWindow::printDialog);
        action->setShortcuts(QKeySequence::Print);
        action->setStatusTip(tr("Print"));
    }
    fileMenu->addSeparator();
    {
        auto action = fileMenu->addAction(QIcon::fromTheme("application-exit"), tr("E&xit"), qApp, &QApplication::closeAllWindows);
        action->setShortcuts(QKeySequence::Quit);
        action->setStatusTip(tr("Exit the application"));
    }
}

void MainWindow::createActionsEdit()
{
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->setObjectName(QStringLiteral("editMenu"));
    auto action = editMenu->addAction(QIcon::fromTheme("edit-select-all"), tr("Select all"), this, &MainWindow::selectAll);
    action->setShortcut(QKeySequence::SelectAll);

    auto dsaShortcut = new QShortcut(this); // Инициализируем объект
    dsaShortcut->setKey(Qt::Key_Escape); // Устанавливаем код клавиши
    connect(dsaShortcut, &QShortcut::activated, this, &MainWindow::deSelectAll); // цепляем обработчик нажатия клавиши

    editMenu->addSeparator();

    action = editMenu->addAction(tr("Undo"));
    action->setEnabled(false);
    action->setShortcut(QKeySequence::Undo);

    action = editMenu->addAction(tr("Redo"));
    action->setEnabled(false);
    action->setShortcut(QKeySequence::Redo);
}

void MainWindow::createActionsService()
{
    serviceMenu = menuBar()->addMenu(tr("&Service"));

    toolpathToolBar = addToolBar(tr("Service"));
    toolpathToolBar->setObjectName("tbService");
    toolpathToolBar->setToolTip(tr("Service"));

    // Settings
    auto action = serviceMenu->addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Settings"), [this] { SettingsDialog(this).exec(); });
    action->setStatusTip(tr("Show the application's settings box"));
    // Separator
    //toolpathToolBar->addSeparator();
    serviceMenu->addSeparator();
    // G-Code Properties
    serviceMenu->addAction(action = toolpathToolBar->addAction(QIcon::fromTheme("node"), tr("&G-Code Properties"), [this] { createDockWidget<GCodePropertiesForm>(GCode::GCodeProperties); }));
    action->setShortcut(QKeySequence("Ctrl+Shift+G"));
    toolpathActions.emplace(GCode::GCodeProperties, action);
    // Tool Base
    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"), [this] { ToolDatabase(this, {}).exec(); }));
    // Separator
    serviceMenu->addSeparator();
    toolpathToolBar->addSeparator();
    //Autoplace All Refpoints
    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Autoplace All Refpoints"), [this] {
        if (updateRect()) {
            Pin::resetPos(false);
            Marker::get(Marker::Home)->resetPos(false);
            Marker::get(Marker::Zero)->resetPos(false);
        }
        ui->graphicsView->zoomFit();
    }));
    // Separator
    serviceMenu->addSeparator();
    toolpathToolBar->addSeparator();
    // Snap to grid
    serviceMenu->addAction(action = toolpathToolBar->addAction(QIcon::fromTheme("snap-to-grid"), tr("Snap to grid"), [](bool checked) { App::settings().setSnap(checked); }));
    action->setCheckable(true);
    // Resize
    if (qApp->applicationDirPath().contains("GERBER_X3/bin")) { // (need for debug)
        serviceMenu->addSeparator();
        toolpathToolBar->addSeparator();
        serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Resize"), [this] {
            auto r(geometry());
            //r.setSize({ 1280, 720 });
            r.setSize({ 1920, 1080 });
            setGeometry(r);
        }));
    }
}

void MainWindow::createActionsHelp()
{
    helpMenu = menuBar()->addMenu(tr("&Help"));
    // About
    auto action = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
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

void MainWindow::createActionsZoom()
{
    auto vievMenu = menuBar()->addMenu(tr("&Viev"));
    vievMenu->setObjectName("vievMenu");

    zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    zoomToolBar->setToolTip(tr("Zoom ToolBar"));

    zoomToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(zoomToolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::customContextMenuForToolBar);

    { // Fit best
        zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), ui->graphicsView, &GraphicsView::zoomFit);
        auto action = vievMenu->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), ui->graphicsView, &GraphicsView::zoomFit);
        action->setShortcut(QKeySequence::FullScreen);
        vievMenu->addAction(action);
    }
    { // 100%
        zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), ui->graphicsView, &GraphicsView::zoom100);
        auto action = vievMenu->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), ui->graphicsView, &GraphicsView::zoom100);
        action->setShortcut(tr("Ctrl+0"));
        vievMenu->addAction(action);
    }
    { // Zoom in
        zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), ui->graphicsView, &GraphicsView::zoomIn);
        auto action = vievMenu->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), ui->graphicsView, &GraphicsView::zoomIn);
        action->setShortcut(QKeySequence::ZoomIn);
        vievMenu->addAction(action);
    }
    { // Zoom out
        zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), ui->graphicsView, &GraphicsView::zoomOut);
        auto action = vievMenu->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), ui->graphicsView, &GraphicsView::zoomOut);
        action->setShortcut(QKeySequence::ZoomOut);
        vievMenu->addAction(action);
    }
    { // Separator
        zoomToolBar->addSeparator();
        vievMenu->addSeparator();
    }
    { // Zoom to selected
        zoomToolBar->addAction(QIcon::fromTheme("zoom-to-selected"), tr("Zoom to selected"), ui->graphicsView, &GraphicsView::zoomToSelected);
        auto action = vievMenu->addAction(QIcon::fromTheme("zoom-to-selected"), tr("Zoom to selected"), ui->graphicsView, &GraphicsView::zoomToSelected);
        action->setShortcut(QKeySequence("F12"));
        vievMenu->addAction(action);
    }
}

void MainWindow::createActionsToolPath()
{
    QMenu* menu = menuBar()->addMenu(tr("&Paths"));

    toolpathToolBar = addToolBar(tr("Toolpath"));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));
    toolpathToolBar->setToolTip(tr("Toolpath"));

    //    toolpathToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    //    connect(toolpathToolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::customContextMenuForToolBar);

    connect(m_dockWidget, &DockWidget::visibilityChanged, [this](bool visible) { if (!visible) resetToolPathsActions(); });
    addDockWidget(Qt::RightDockWidgetArea, m_dockWidget);

    // Profile
    auto action = toolpathToolBar->addAction(QIcon::fromTheme("profile-path"), tr("Pro&file"), [this] { createDockWidget<ProfileForm>(GCode::Profile); });
    action->setShortcut(QKeySequence("Ctrl+Shift+F"));
    menu->addAction(action);
    toolpathActions.emplace(GCode::Profile, action);

    // Pocket
    action = toolpathToolBar->addAction(QIcon::fromTheme("pocket-path"), tr("&Pocket"), [this] { createDockWidget<PocketOffsetForm>(GCode::Pocket); });
    action->setShortcut(QKeySequence("Ctrl+Shift+P"));
    menu->addAction(action);
    toolpathActions.emplace(GCode::Pocket, action);

    // Pocket Raster
    action = toolpathToolBar->addAction(QIcon::fromTheme("raster-path"), tr("&PocketR"), [this] { createDockWidget<PocketRasterForm>(GCode::Raster); });
    action->setShortcut(QKeySequence("Ctrl+Shift+R"));
    menu->addAction(action);
    toolpathActions.emplace(GCode::Raster, action);

    // Voronoi
    action = toolpathToolBar->addAction(QIcon::fromTheme("voronoi-path"), tr("&Voronoi"), [this] { createDockWidget<VoronoiForm>(GCode::Voronoi); });
    action->setShortcut(QKeySequence("Ctrl+Shift+V"));
    menu->addAction(action);
    toolpathActions.emplace(GCode::Voronoi, action);

    // Thermal Insulation
    action = toolpathToolBar->addAction(QIcon::fromTheme("thermal-path"), tr("&Thermal Insulation"), [this] {
        ThermalForm::canToShow() ? createDockWidget<ThermalForm>(GCode::Thermal)
                                 : toolpathActions[GCode::Thermal]->setChecked(false);
    });
    action->setShortcut(QKeySequence("Ctrl+Shift+T"));
    menu->addAction(action);
    toolpathActions.emplace(GCode::Thermal, action);

    // Drilling
    action = toolpathToolBar->addAction(QIcon::fromTheme("drill-path"), tr("&Drilling"), [this] {
        DrillForm::canToShow() ? createDockWidget<DrillForm>(GCode::Drill)
                               : toolpathActions[GCode::Drill]->setChecked(false);
    });
    action->setShortcut(QKeySequence("Ctrl+Shift+D"));
    menu->addAction(action);
    toolpathActions.emplace(GCode::Drill, action);

    // Crosshatch
    action = toolpathToolBar->addAction(QIcon::fromTheme("crosshatch-path"), tr("&Crosshatch"), [this] { createDockWidget<HatchingForm>(GCode::Hatching); });
    action->setShortcut(QKeySequence("Ctrl+Shift+C"));
    menu->addAction(action);
    toolpathActions.emplace(GCode::Hatching, action);

    for (auto [key, action] : toolpathActions) {
        action->setCheckable(true);
        toolpathActionGroup.addAction(action);
    }
}

void MainWindow::createActionsShape()
{
    if (App::shapePlugins().empty())
        return;

    QToolBar* toolBar = addToolBar(tr("Graphics Items"));
    toolBar->setObjectName("GraphicsItemsToolBar");
    toolBar->setToolTip(tr("Graphics Items"));

    toolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(toolBar, &QToolBar::customContextMenuRequested, this, &MainWindow::customContextMenuForToolBar);

    for (auto& [type, pair] : App::shapePlugins()) {
        auto& [shInt, pobj] = pair;
        auto action = toolBar->addAction(shInt->icon(), shInt->info().value("Name").toString());
        action->setCheckable(true);
        connect(pobj, SIGNAL(actionUncheck(bool)), action, SLOT(setChecked(bool)));
        connect(action, &QAction::toggled, [shInt = shInt](bool checked) {
            checked ? ShapePluginInterface::finalizeShape_(),
                ShapePluginInterface::setShapePI(shInt)
                    : ShapePluginInterface::finalizeShape_();
        });
    }

    toolBar->addSeparator();

    auto executor = [](ClipType type) {
        auto selectedItems(App::scene()->selectedItems());
        Paths clipPaths;
        for (QGraphicsItem* clipItem : selectedItems) {
            if (static_cast<GiType>(clipItem->type()) >= GiType::ShCircle)
                clipPaths.append(static_cast<GraphicsItem*>(clipItem)->paths());
        }

        QList<GraphicsItem*> rmi;
        for (QGraphicsItem* item : selectedItems) {
            if (static_cast<GiType>(item->type()) == GiType::DataSolid) {
                auto gitem = static_cast<GiDataSolid*>(item);
                Clipper clipper;
                clipper.AddPaths(gitem->paths(), ptSubject, true);
                clipper.AddPaths(clipPaths, ptClip, true);
                clipper.Execute(type, *gitem->rPaths(), pftEvenOdd, pftPositive);
                if (gitem->rPaths()->empty()) {
                    rmi.push_back(gitem);
                } else {
                    ReversePaths(*gitem->rPaths()); //??
                    gitem->redraw();
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

void MainWindow::customContextMenuForToolBar(const QPoint& pos)
{
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

void MainWindow::saveGCodeFile(int id)
{
    qDebug();
    if (m_project->pinsPlacedMessage())
        return;
    auto* file = m_project->file<GCode::File>(id);
    QString name(QFileDialog::getSaveFileName(this, tr("Save GCode file"),
        GCode::GCUtils::getLastDir().append(file->shortName()),
        tr("GCode (*.%1)").arg(GCode::Settings::fileExtension())));

    if (name.isEmpty())
        return;

    file->save(name);
}

void MainWindow::saveGCodeFiles()
{
    qDebug();
}

void MainWindow::saveSelectedGCodeFiles()
{
    qDebug();
    if (m_project->pinsPlacedMessage())
        return;

    mvector<GCode::File*> gcFiles(m_project->files<GCode::File>());
    for (size_t i = 0; i < gcFiles.size(); ++i) {
        if (!gcFiles[i]->itemGroup()->isVisible())
            gcFiles.remove(i--);
    }

    using Key = std::pair<size_t, Side>;
    using GcFiles = QList<GCode::File*>;

    std::map<Key, GcFiles> gcFilesMap;
    for (GCode::File* file : gcFiles)
        gcFilesMap[{ file->getTool().hash(), file->side() }].append(file);

    for (const auto& [key, files] : gcFilesMap) {
        if (files.size() < 2) {
            for (GCode::File* file : files) {
                QString name(GCode::GCUtils::getLastDir().append(file->shortName()));
                if (!name.endsWith("tap")) ////////////////////////////////////////////////////////////////////////////////////////////
                    name += QStringList({ "_TS", "_BS" })[file->side()];

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
            QString name(GCode::GCUtils::getLastDir().append(files.first()->getTool().nameEnc()));
            if (!name.endsWith("tap")) ////////////////////////////////////////////////////////////////////////////////////////////
                name += QStringList({ "_TS", "_BS" })[files.first()->side()];

            name = QFileDialog::getSaveFileName(nullptr,
                QObject::tr("Save GCode file"),
                name,
                QObject::tr("GCode (*.%1)").arg(GCode::Settings::fileExtension()));

            if (name.isEmpty())
                return;
            mvector<QString> sl;

            sl.push_back(tr(";\tContains files:"));
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

void MainWindow::newFile()
{
    if (closeProject()) {
        setCurrentFile(QString());
    }
}

void MainWindow::readSettings()
{
    SettingsDialog().accept();

    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    restoreState(settings.value("state", QByteArray()).toByteArray());
    lastPath = settings.value("lastPath").toString();
    if (qApp->applicationDirPath().contains("GERBER_X3/bin"))
        loadFile(settings.value("project").toString());
    //r
    for (auto toolBar : findChildren<QToolBar*>()) {
        settings.beginReadArray(toolBar->objectName());
        int ctr = 0;
        for (auto action : toolBar->actions()) {
            settings.setArrayIndex(ctr++);
            action->setVisible(settings.value("actionIsVisible", true).toBool());
        }
        settings.endArray();
    }
    settings.endGroup();
    updateTheme();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("project", m_project->name());
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

void MainWindow::selectAll()
{
    for (auto [k, v] : toolpathActions)
        qDebug() << k << v->isChecked();

    if /*  */ (toolpathActions[GCode::Thermal]->isChecked()) {
        for (QGraphicsItem* item : App::scene()->items())
            if (const auto type = static_cast<GiType>(item->type());
                type == GiType::PrThermal)
                item->setSelected(true);
    } else if (toolpathActions[GCode::Drill]->isChecked()) {
        for (QGraphicsItem* item : App::scene()->items())
            if (const auto type = static_cast<GiType>(item->type());
                type == GiType::PrApetrure || type == GiType::PrDrill || type == GiType::PrSlot)
                item->setSelected(true);
    } else {
        for (QGraphicsItem* item : App::scene()->items())
            if (item->isVisible())
                item->setSelected(true);
    }
}

void MainWindow::deSelectAll()
{
    qDebug();
    for (QGraphicsItem* item : App::scene()->items())
        if (item->isVisible())
            item->setSelected(false);
}

void MainWindow::printDialog()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, [](QPrinter* pPrinter) {
        App::scene()->m_drawPdf = true;
        QRectF rect;
        for (QGraphicsItem* item : App::scene()->items())
            if (item->isVisible() && !item->boundingRect().isNull())
                rect |= item->boundingRect();
        QSizeF size(rect.size());

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        pPrinter->setMargins({ 10, 10, 10, 10 });
        pPrinter->setPageSizeMM(size + QSizeF(pPrinter->margins().left + pPrinter->margins().right, pPrinter->margins().top + pPrinter->margins().bottom));
#else
            QMarginsF margins( 10, 10, 10, 10 );
            pPrinter->setPageMargins(margins);
            pPrinter->setPageSize(QPageSize(size + QSizeF(margins.left() + margins.right(),
                                                          margins.top() + margins.bottom()),
                                            QPageSize::Millimeter));
#endif
        pPrinter->setResolution(4800);

        QPainter painter(pPrinter);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setTransform(QTransform().scale(1.0, -1.0));
        painter.translate(0, -(pPrinter->resolution() / 25.4) * size.height());
        App::scene()->render(&painter,
            QRectF(0, 0, pPrinter->width(), pPrinter->height()),
            rect,
            Qt::KeepAspectRatio /*IgnoreAspectRatio*/);
        App::scene()->m_drawPdf = false;
    });
    preview.exec();
}

void MainWindow::fileProgress(const QString& fileName, int max, int value)
{
    if (max && !value) {
        QProgressDialog* pd = new QProgressDialog(this);
        pd->setCancelButton(nullptr);
        pd->setLabelText(fileName);
        pd->setMaximum(max);
        //        pd->setModal(true);
        //        pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
        pd->show();
        m_progressDialogs[fileName] = pd;
    } else if (max == 1 && value == 1) {
        m_progressDialogs[fileName]->hide();
        m_progressDialogs[fileName]->deleteLater();
        m_progressDialogs.remove(fileName);
    } else
        m_progressDialogs[fileName]->setValue(value);
}

void MainWindow::fileError(const QString& fileName, const QString& error)
{
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

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        verticalLayout->setMargin(6);
#else
        verticalLayout->setContentsMargins(6, 6, 6, 6);
#endif
        verticalLayout->setSpacing(6);
    }
    fileErrordialog->show();
    textBrowser->setTextColor(Qt::black);
    textBrowser->append(fileName);
    textBrowser->setTextColor(Qt::darkRed);
    textBrowser->append(error);
    textBrowser->append("");
}

void MainWindow::resetToolPathsActions()
{
    for (auto [key, action] : toolpathActions)
        action->setChecked(false);
}

void MainWindow::documentWasModified() { setWindowModified(m_project->isModified()); }

bool MainWindow::maybeSave()
{
    if (!m_project->isModified() && m_project->size()) {
        return QMessageBox::warning(this, tr("Warning"), tr("Do you want to close this project?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok;
    } else if (!m_project->size()) {
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

void MainWindow::editGcFile(GCode::File* file)
{
    switch (file->gtype()) {
    case GCode::Null:
    case GCode::Profile:
        toolpathActions[GCode::Profile]->triggered();
        reinterpret_cast<FormsUtil*>(m_dockWidget->widget())->editFile(file);
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

bool MainWindow::saveFile(const QString& fileName)
{
    bool ok;
    QApplication::setOverrideCursor(Qt::WaitCursor);
    if (ok = m_project->save(fileName); ok) {
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

void MainWindow::setCurrentFile(const QString& fileName)
{
    m_project->setName(fileName);
    m_project->setModified(false);
    setWindowModified(false);
    if (!m_project->isUntitled())
        recentProjects.prependToRecentFiles(m_project->name());
    m_closeAllAct->setText(tr("&Close project \"%1\"").arg(strippedName(m_project->name())));
    setWindowFilePath(m_project->name());
}

void MainWindow::addFileToPro(FileInterface* file)
{
    if (m_project->isUntitled()) {
        QString name(QFileInfo(file->name()).path());
        setCurrentFile(name + "/" + name.split('/').back() + ".g2g");
    }
    m_project->addFile(file);
    recentFiles.prependToRecentFiles(file->name());
    ui->graphicsView->zoomFit();
}

QString MainWindow::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

template <class T>
void MainWindow::createDockWidget(int type)
{
    toolpathActions[type]->setChecked(true);
    if (dynamic_cast<T*>(m_dockWidget->widget()))
        return;

    auto dwContent = new T(m_dockWidget);
    dwContent->setObjectName(typeid(T).name());

    //    for (auto [key, action] : qAsConst(toolpathActions))
    //        action->setChecked(false);
    //    toolpathActions[type]->setChecked(true);

    m_dockWidget->pop();
    m_dockWidget->push(dwContent);
    m_dockWidget->show();
}

QMenu* MainWindow::createPopupMenu()
{
    QMenu* menu = QMainWindow::createPopupMenu();
    menu->removeAction(m_dockWidget->toggleViewAction());
    menu->removeAction(toolpathToolBar->toggleViewAction());
    menu->removeAction(ui->treeDockWidget->toggleViewAction());

    menu->addAction(tr("Icon size = 24"), [this]() { setIconSize(QSize(24, 24)); });
    menu->addAction(tr("Icon size = 48"), [this]() { setIconSize(QSize(48, 48)); });
    menu->addAction(tr("Icon size = 72"), [this]() { setIconSize(QSize(72, 72)); });

    qDebug() << menu->parent();

    return menu;
}

void MainWindow::translate(const QString& locale)
{
    static QTranslator qtTranslator;
    static QTranslator appTranslator;

    const QString trFolder(
        qApp->applicationDirPath().contains("GERBER_X3/bin")
            ? qApp->applicationDirPath() + "/../GGEasy/translations"
            : qApp->applicationDirPath() + "/translations");

    const QString qtTr("qtbase_" + locale + ".qm");
    const QString appTr(qApp->applicationDisplayName() + "_" + locale + ".qm");

    if (qtTranslator.load(qtTr, trFolder))
        qApp->installTranslator(&qtTranslator);
    else
        qDebug() << "err qtTranslator";

    if (appTranslator.load(appTr, trFolder))
        qApp->installTranslator(&appTranslator);
    else
        qDebug() << "err appTranslator";
}

void MainWindow::loadFile(const QString& fileName)
{
    if (!QFile(fileName).exists())
        return;
    lastPath = QFileInfo(fileName).absolutePath();
    if (fileName.endsWith(".g2g")) {
        if (closeProject()) {
            m_project->open(fileName);
            setCurrentFile(fileName);
            QTimer::singleShot(100, Qt::CoarseTimer, ui->graphicsView, &GraphicsView::zoomFit);
            return;
        }
    } else {
        if (m_project->contains(fileName) > -1 && QMessageBox::question(this, tr("Warning"), //
                                                      tr("Do you want to reload file %1?").arg(QFileInfo(fileName).fileName()), QMessageBox::Ok | QMessageBox::Cancel)
                == QMessageBox::Cancel)
            return;
        for (auto& [type, tuple] : App::filePlugins()) {
            auto& [parser, pobj] = tuple;
            if (parser->thisIsIt(fileName)) {
                emit parseFile(fileName, int(type));
                return;
            }
        }
    }
    qDebug() << fileName;
}

void MainWindow::updateTheme()
{
    //    class ProxyStyle : public QProxyStyle {
    //        //Q_OBJECT
    //    public:
    //        ProxyStyle(QStyle* style = nullptr)
    //            : QProxyStyle(style)
    //        {
    //        }
    //        ProxyStyle(const QString& key)
    //            : QProxyStyle(key)
    //        {
    //        }
    //        virtual int pixelMetric(QStyle::PixelMetric metric, const QStyleOption* option = 0, const QWidget* widget = 0) const override
    //        {
    //            //qDebug() << metric;
    //            //        switch (metric) {
    //            //        case QStyle::PM_SmallIconSize:
    //            //            return 22;
    //            //        default:
    //            //            return QProxyStyle::pixelMetric(metric, option, widget);
    //            //        }
    //            return QProxyStyle::pixelMetric(metric, option, widget);
    //        }
    //        //        virtual QPixmap standardPixmap(StandardPixmap standardPixmap, const QStyleOption* opt = nullptr, const QWidget* widget = nullptr) const override
    //        //        {
    //        //            qDebug() << standardPixmap;
    //        //            return {};
    //        //            return QProxyStyle::standardPixmap(standardPixmap, opt, widget);
    //        //        }
    //        QPixmap generatedIconPixmap(QIcon::Mode iconMode, const QPixmap& pixmap, const QStyleOption* opt) const override
    //        {
    //            qDebug() << iconMode;
    //            return {};
    //            return QProxyStyle::generatedIconPixmap(iconMode, pixmap, opt);
    //        }
    //    };

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
            disabledColor = QColor(127, 127, 127);
            highlightColor = QColor(61, 174, 233);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(60, 60, 60);
            windowTextColor = QColor(220, 220, 220);
            break;
        case DarkRed:
            baseColor = QColor(40, 40, 40);
            disabledColor = QColor(127, 127, 127);
            highlightColor = QColor(218, 68, 83);
            linkColor = QColor(61, 174, 233);
            windowColor = QColor(60, 60, 60);
            windowTextColor = QColor(220, 220, 220);
            break;
        }

        QPalette palette;
        palette.setColor(QPalette::Disabled, QPalette::ButtonText, disabledColor);
        palette.setColor(QPalette::Disabled, QPalette::HighlightedText, disabledColor);
        palette.setColor(QPalette::Disabled, QPalette::Text, disabledColor);
        palette.setColor(QPalette::Disabled, QPalette::Shadow, disabledColor);

        palette.setColor(QPalette::Text, windowTextColor);
        palette.setColor(QPalette::ToolTipText, windowTextColor);
        palette.setColor(QPalette::WindowText, windowTextColor);
        palette.setColor(QPalette::ButtonText, windowTextColor);
        palette.setColor(QPalette::HighlightedText, Qt::black);
        palette.setColor(QPalette::BrightText, Qt::red);

        palette.setColor(QPalette::Link, linkColor);
        palette.setColor(QPalette::LinkVisited, highlightColor);

        palette.setColor(QPalette::AlternateBase, windowColor);
        palette.setColor(QPalette::Base, baseColor);
        palette.setColor(QPalette::Button, windowColor);

        palette.setColor(QPalette::Highlight, highlightColor);

        palette.setColor(QPalette::ToolTipBase, windowTextColor);
        palette.setColor(QPalette::Window, windowColor);
        qApp->setPalette(palette);
    } else {
        qApp->setStyle(QStyleFactory::create("windowsvista"));
    }
    //    if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows && QOperatingSystemVersion::current().majorVersion() > 7) {
    //        App::mainWindow()->setStyleSheet("QGroupBox, .QFrame {"
    //                                         //"background-color: white;"
    //                                         "border: 1px solid gray; }"
    //                                         "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
    //                                         "QGroupBox::title {"
    //                                         "subcontrol-origin: margin;"
    //                                         "subcontrol-position: top center; }" /* position at the top center */
    //        );
    //    } else {
    //        App::mainWindow()->setStyleSheet("QGroupBox, .QFrame {"
    //                                         //"background-color: white;"
    //                                         "border: 1px solid gray;"
    //                                         "border-radius: 3px; }" // Win 7 or other
    //                                         "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
    //                                         "QGroupBox::title {"
    //                                         "subcontrol-origin: margin;"
    //                                         "subcontrol-position: top center; }" /* position at the top center */
    //        );
    //    }

    if (App::settings().theme() < DarkBlue) {
        QIcon::setThemeSearchPaths({
            qApp->applicationDirPath() + "/../icons/breezeLight/",
            qApp->applicationDirPath() + "/icons/breezeLight/",
        });
    } else {
        QIcon::setThemeSearchPaths({
            qApp->applicationDirPath() + "/../icons/breezeDark/",
            qApp->applicationDirPath() + "/icons/breezeDark/",
        });
        //        App::graphicsView()->setStyleSheet(
        //            QString("QScrollBar {"
        //                    //            "    border: 2px solid grey;"
        //                    "    background: %1;"
        //                    //            "    width: 15px;"
        //                    "    margin: 22px 0 22px 0;"
        //                    "}"
        //                    "QScrollBar::handle {"
        //                    "    background: %2;"
        //                    "    border-radius: 2px;"
        //                    "    border: 1px solid grey;"
        //                    //                        "    min-height: 20px;"
        //                    "}"
        //                    "QScrollBar::add-line {"
        //                    //                    "    border: 2px solid grey;"
        //                    //                    "    background: #32CC99;"
        //                    //                    "    height: 20px;"
        //                    //                    "    subcontrol-position: bottom;"
        //                    //                    "    subcontrol-origin: margin;"
        //                    "}"
        //                    "QScrollBar::sub-line {"
        //                    //                    "    border: 2px solid grey;"
        //                    //                    "    background: #32CC99;"
        //                    //                    "    height: 20px;"
        //                    //                    "    subcontrol-position: top;"
        //                    //                    "    subcontrol-origin: margin;"
        //                    "}"
        //                    "QScrollBar::up-arrow, QScrollBar::down-arrow {"
        //                    //                    "    border: 2px solid grey;"
        //                    //                    "    width: 3px;"
        //                    //                    "    height: 3px;"
        //                    //                    "    background: white;"
        //                    "}"
        //                    "QScrollBar::add-page, QScrollBar::sub-page {"
        //                    //                    "    background: none;"
        //                    "}")
        //                .arg(windowColor.name())
        //                .arg(highlightColor.darker().name())
        //                .arg(disabledColor.name()) //
        //        );
    }
    QIcon::setThemeName("Breeze");

    SettingsDialog d;
    d.show();
}

void MainWindow::open()
{
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
    //    QString name(QFileInfo(files.first()).path());
    //    setCurrentFile(name + "/" + name.split('/').back() + ".g2g");
}

bool MainWindow::save()
{
    if (m_project->isUntitled())
        return saveAs();
    else
        return saveFile(m_project->name());
}

bool MainWindow::saveAs()
{
    QString file(
        QFileDialog::getSaveFileName(this, tr("Open File"), m_project->name(), tr("Project (*.g2g)")));
    if (file.isEmpty())
        return false;
    return saveFile(file);
}

void MainWindow::showEvent(QShowEvent* event)
{
    //toolpathActionList[GCode::GCodeProperties]->trigger();//////////////////////////////////////////////////////
    QMainWindow::showEvent(event);
}

void MainWindow::changeEvent(QEvent* event)
{
    // В случае получения события изменения языка приложения
    if (event->type() == QEvent::LanguageChange) {
        ui->retranslateUi(this); // переведём окно заново
    }
}

//////////////////////////////////////////////////////
/// \brief DockWidget::DockWidget
/// \param parent
///
DockWidget::DockWidget(QWidget* parent)
    : QDockWidget(parent)
{
    hide();
    setVisible(false);
}

void DockWidget::push(QWidget* w)
{
    if (widget())
        widgets.push(widget());
    if (w)
        QDockWidget::setWidget(w);
}

void DockWidget::pop()
{
    if (widget()) {
        if (widget()->objectName() == "ErrorDialog") {
            static_cast<QDialog*>(widget())->reject();
            QTimer::singleShot(1, [this] { widgets.pop(); });
        } else {
            delete widget();
        }
    }
    if (!widgets.isEmpty())
        QDockWidget::setWidget(widgets.pop());
}

void DockWidget::closeEvent(QCloseEvent* event)
{
    pop();
    event->accept();
}

void DockWidget::showEvent(QShowEvent* event)
{
    event->ignore();
    if (widget() == nullptr)
        QTimer::singleShot(1, this, &QDockWidget::close);
}

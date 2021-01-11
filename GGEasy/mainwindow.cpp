// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
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

#include "leakdetector.h"

bool operator<(const QPair<Tool, Side>& p1, const QPair<Tool, Side>& p2)
{
    return p1.first.hash() < p2.first.hash() || (!(p2.first.hash() < p1.first.hash()) && p1.second < p2.second);
}

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , recentFiles(this, "recentFileList")
    , recentProjects(this, "recentProjectsList")
    , m_project(new Project(this))
{
    App::m_app->m_mainWindow = this;

    ui->setupUi(this);

    initWidgets();

    new Marker(Marker::Home);
    new Marker(Marker::Zero);
    new Pin();
    new Pin();
    new Pin();
    new Pin();

    GCodePropertiesForm(); // init default vars;

    connect(m_project, &Project::homePosChanged, Marker::get(Marker::Home), qOverload<const QPointF&>(&Marker::setPos));
    connect(m_project, &Project::zeroPosChanged, Marker::get(Marker::Zero), qOverload<const QPointF&>(&Marker::setPos));
    connect(m_project, &Project::pinsPosChanged, qOverload<const QPointF[4]>(&Pin::setPos));
    connect(m_project, &Project::layoutFrameUpdate, new LayoutFrames(), &LayoutFrames::updateRect);
    connect(m_project, &Project::changed, this, &MainWindow::documentWasModified);

    // connect plugins
    for (auto& [type, pair] : App::fileInterfaces()) {
        auto& [parser, pobj] = pair;
        pobj->moveToThread(&parserThread);
        connect(pobj, SIGNAL(fileError(const QString&, const QString&)), this, SLOT(fileError(const QString&, const QString&)), Qt::QueuedConnection);
        connect(pobj, SIGNAL(fileProgress(const QString&, int, int)), this, SLOT(fileProgress(const QString&, int, int)), Qt::QueuedConnection);
        connect(pobj, SIGNAL(fileReady(FileInterface*)), this, SLOT(addFileToPro(FileInterface*)), Qt::QueuedConnection);
        connect(this, SIGNAL(parseFile(const QString&, int)), pobj, SLOT(parseFile(const QString&, int)), Qt::QueuedConnection);
    }

    { // add dummy gcode plugin
        auto parser = new GCode::Plugin(this);
        PIF pi {
            static_cast<FilePluginInterface*>(parser),
            static_cast<QObject*>(parser)
        };
        App::fileInterfaces().emplace(parser->type(), pi);
    }

    parserThread.start(QThread::HighestPriority);

    connect(ui->graphicsView, &GraphicsView::fileDroped, this, &MainWindow::loadFile);

    // Shapes::Constructor
    connect(ui->graphicsView, &GraphicsView::mouseMove, &ShapePluginInterface::updateShape_);
    connect(ui->graphicsView, &GraphicsView::mouseClickR, &ShapePluginInterface::finalizeShape_);
    connect(ui->graphicsView, &GraphicsView::mouseClickL, &ShapePluginInterface::addShapePoint_);

    ui->treeView->setModel(new FileModel(ui->treeView));

    connect(ui->treeView, &FileTreeView::saveGCodeFile, this, &MainWindow::saveGCodeFile);
    connect(ui->treeView, &FileTreeView::saveGCodeFiles, this, &MainWindow::saveGCodeFiles);
    connect(ui->treeView, &FileTreeView::saveSelectedGCodeFiles, this, &MainWindow::saveSelectedGCodeFiles);

    if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows && QOperatingSystemVersion::current().majorVersion() > 7) {
        setStyleSheet("QGroupBox, .QFrame {"
                      "background-color: white;"
                      "border: 1px solid gray; }"
                      "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
                      "QGroupBox::title {"
                      "subcontrol-origin: margin;"
                      "subcontrol-position: top center; }" /* position at the top center */
        );
    } else {
        setStyleSheet("QGroupBox, .QFrame {"
                      "background-color: white;"
                      "border: 1px solid gray;"
                      "border-radius: 3px; }" // Win 7 or other
                      "QGroupBox { margin-top: 3ex; }" /* leave space at the top for the title */
                      "QGroupBox::title {"
                      "subcontrol-origin: margin;"
                      "subcontrol-position: top center; }" /* position at the top center */
        );
    }
    App::toolHolder().readTools();
    setCurrentFile(QString());

    readSettings();

    if (0 || qApp->applicationDirPath().contains("GERBER_X3/bin")) { // (need for debug)
        int i = 0;
        int k = 100;

        if (1) {
            //            QDir dir("D:/Gerber Test Files/Ucamco/2019 12 08 KiCad X3 sample - dvk-mx8m-bsb");
            QDir dir("C:/Свалка/SSR_V4");
            QStringList listFiles;
            if (dir.exists())
                listFiles = dir.entryList(QStringList("*.gbr"), QDir::Files);
            for (QString str : listFiles) {
                str = dir.path() + '/' + str;
                qDebug() << __FUNCTION__ << str;
                QTimer::singleShot(++i * k, [this, str] { loadFile(str); });
            }
        }

        //        k = 300;
        //        QTimer::singleShot(++i * k, [this] { selectAll(); });
        //        QTimer::singleShot(++i * k, [this] { toolpathActions[GCode::Profile]->triggered(); });
        //        QTimer::singleShot(++i * k, [this] { m_dockWidget->findChild<QPushButton*>("pbCreate")->click(); });
        // "D:/Gerber Test Files/Ucamco/2019 12 08 KiCad X3 sample - dvk-mx8m-bsb/dvk-mx8m-bsb-pnp_bottom.gbr"
        // "D:/QtPro/MAN2/МАН2_SCH_PCB/V2/МАН2_МСИС_V2_.dxf"
        // "C:/Users/X-Ray/Documents/TopoR/Examples/Example_02/Placement.dxf"
        // "D:\\Gerber Test Files\\DXF\\ELEMER\\МАН2_МСИС_V2_.DXF"
        // "D:/Gerber Test Files/DXF/misc01.dxf"
        // "D:/ELECTROSTATIC_AMP_A.dxf"
        // "D:/T/ELECTROSTATIC_AMP_A_TOP.dxf"
        // "D:/Gerber Test Files/DXF/Complete-Lib-E.dxf"
        // "D:/T/1mm 30x15x20 V2 trans.dxf" //
        //        for (int j = 0; j < 50; ++j) {
        //            QTimer::singleShot(++i * 100, [this] { serviceMenu->actions()[4]->triggered(); });
        //        }
        //        QTimer::singleShot(++i * 500, [this] { loadFile("P:/ELEMER/SSR/SSR_V4/Board_outline.gbr"); });
        //        QTimer::singleShot(++i * 500, [this] { selectAll(); });
        //        QTimer::singleShot(++i * 100, [this] { toolpathActions[GCode::Thermal]->triggered(); });
        //QTimer::singleShot(++i * 100, [this] { m_dockWidget->findChild<QPushButton*>("pbCreate")->click(); });
    }
}

MainWindow::~MainWindow()
{
    parserThread.quit();
    parserThread.wait();
    App::m_app->m_mainWindow = nullptr;
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
    createActionsGraphics();
    // helpMenu
    createActionsHelp();

    statusBar()->showMessage(tr("Ready"));
}

void MainWindow::createActionsFile()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    fileMenu->setObjectName(QStringLiteral("fileMenu"));

    fileToolBar = addToolBar(tr("File"));
    fileToolBar->setContextMenuPolicy(Qt::CustomContextMenu);
    fileToolBar->setObjectName(QStringLiteral("fileToolBar"));
    fileToolBar->setToolTip(tr("File"));

    // New
    QAction* action = fileMenu->addAction(QIcon::fromTheme("project-development-new-template"), tr("&New project"),
        this, &MainWindow::newFile);
    action->setShortcuts(QKeySequence::New);
    action->setStatusTip(tr("Create a new file"));
    fileToolBar->addAction(action);

    // Open
    action = fileMenu->addAction(QIcon::fromTheme("document-open"), tr("&Open..."),
        this, &MainWindow::open);
    action->setShortcuts(QKeySequence::Open);
    action->setStatusTip(tr("Open an existing file"));
    fileToolBar->addAction(action);
    // Save
    action = fileMenu->addAction(QIcon::fromTheme("document-save"), tr("&Save project"),
        this, &MainWindow::save);
    action->setShortcuts(QKeySequence::Save);
    action->setStatusTip(tr("Save the document to disk"));
    fileToolBar->addAction(action);
    // Save As
    action = fileMenu->addAction(QIcon::fromTheme("document-save-as"), tr("Save project &As..."),
        this, &MainWindow::saveAs);
    action->setShortcuts(QKeySequence::SaveAs);
    action->setStatusTip(tr("Save the document under a new name"));
    fileToolBar->addAction(action);
    // Save Selected Tool Paths
    action = fileMenu->addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."),
        this, &MainWindow::saveSelectedGCodeFiles);
    action->setStatusTip(tr("Save selected toolpaths"));
    fileToolBar->addAction(action);
    // Export PDF
    action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), App::scene(), &Scene::RenderPdf);
    action->setStatusTip(tr("Export to PDF file"));
    fileToolBar->addAction(action);

    fileMenu->addSeparator();
    fileMenu->addSeparator();

    recentFiles.createMenu(fileMenu, tr("Recent Files..."));
    recentProjects.createMenu(fileMenu, tr("Recent Projects..."));

    m_closeAllAct = fileMenu->addAction(QIcon::fromTheme("document-close"), tr("&Close project \"%1\""), this, &MainWindow::closeProject);
    m_closeAllAct->setShortcuts(QKeySequence::Close);
    m_closeAllAct->setStatusTip(tr("Close project"));
    // m_closeAllAct->setEnabled(false);
    fileToolBar->addAction(m_closeAllAct);

    fileMenu->addSeparator();
    action = fileMenu->addAction(QIcon::fromTheme("document-print"), tr("P&rint"), this, &MainWindow::printDialog);
    action->setShortcuts(QKeySequence::Print);
    action->setStatusTip(tr("Print"));
    fileMenu->addSeparator();

    action = fileMenu->addAction(QIcon::fromTheme("application-exit"), tr("E&xit"), qApp, &QApplication::closeAllWindows);
    action->setShortcuts(QKeySequence::Quit);
    action->setStatusTip(tr("Exit the application"));
}

void MainWindow::createActionsEdit()
{
    QMenu* editMenu = menuBar()->addMenu(tr("&Edit"));
    editMenu->setObjectName(QStringLiteral("editMenu"));
    QAction* action = editMenu->addAction(QIcon::fromTheme("edit-select-all"),
        tr("Select all"),
        this,
        &MainWindow::selectAll);
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

    //    QToolBar* toolBar = addToolBar(tr("Selection"));
    //    toolBar->setObjectName(QStringLiteral("s"));
    // s->setMovable(false);
    //    QAction* action = toolBar->addAction(QIcon::fromTheme("edit-select-all"),
    //        tr("Select all"),
    //        this,
    //        &MainWindow::selectAll);
    //    action->setShortcut(QKeySequence::SelectAll);

    // action = toolBar->addAction(QIcon::fromTheme("document-close"), tr("Redo"), this, &MainWindow::redo);
    // action->setShortcut(QKeySequence::Redo);
    // action = s->addAction(QIcon::fromTheme("layer-delete"), tr("Delete selected"), [this]() {
    // QList<QGraphicsItem*> list;
    // for (QGraphicsItem* item : MyApp::scene()->123->items())
    // if (item->isSelected() && item->type() != DrillItemType)
    // list << item;
    // if (list.size() && QMessageBox::question(this,
    // "", "Do you really want to delete the selected items?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
    // for (QGraphicsItem* item : list)
    // if (item->isSelected() && item->type() != DrillItemType)
    // delete item;
    // MyApp::scene()->123->setSceneRect(MyApp::scene()->123->itemsBoundingRect());
    // MyApp::scene()->123->update();
    // MainWindow::123->zero()->resetPos();
    // MainWindow::123->home()->resetPos();
    // Pin::shtifts()[0]->resetPos();
    // }
    // });
    // action->setShortcut(QKeySequence::Delete);
}

void MainWindow::createActionsService()
{
    serviceMenu = menuBar()->addMenu(tr("&Service"));
    QAction* action = serviceMenu->addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Settings"), [this] {
        SettingsDialog(this).exec();
    });
    action->setStatusTip(tr("Show the application's settings box"));

    toolpathToolBar = addToolBar(tr("Service"));
    toolpathToolBar->setObjectName("tbService");
    toolpathToolBar->setToolTip(tr("Service"));

    //toolpathToolBar->addSeparator();
    serviceMenu->addSeparator();
    {
        action = toolpathToolBar->addAction(QIcon::fromTheme("node"), tr("&G-Code Properties"), [this] {
            createDockWidget<GCodePropertiesForm>(/*new GCodePropertiesForm(dockWidget),*/ GCode::GCodeProperties);
        });
        action->setShortcut(QKeySequence("Ctrl+Shift+G"));
        serviceMenu->addAction(action);
        toolpathActions[GCode::GCodeProperties] = action;
    }

    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"), [this] {
        ToolDatabase tdb(this, {});
        tdb.exec();
    }));
    toolpathToolBar->addSeparator();
    serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Autoplace All Refpoints"), [this] {
        if (updateRect()) {
            Pin::resetPos(false);
            Marker::get(Marker::Home)->resetPos(false);
            Marker::get(Marker::Zero)->resetPos(false);
        }
        ui->graphicsView->zoomFit();
    }));

    serviceMenu->addSeparator();
    toolpathToolBar->addSeparator();
    serviceMenu->addAction(action = toolpathToolBar->addAction(QIcon::fromTheme("snap-to-grid"), tr("Snap to grid"), [](bool checked) { App::settings().setSnap(checked); }));
    action->setCheckable(true);

    if (qApp->applicationDirPath().contains("GERBER_X3/bin")) { // (need for debug)
        serviceMenu->addSeparator();
        serviceMenu->addAction(toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Resize"), [this] {
            auto r(geometry());
            r.setSize({ 1280, 720 });
            setGeometry(r);
        }));
    }
}

void MainWindow::createActionsHelp()
{
    helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* action = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    action->setStatusTip(tr("Show the application's About box"));

    action = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    action->setStatusTip(tr("Show the Qt library's About box"));

    helpMenu->addSeparator();

    action = helpMenu->addAction(tr("About &Plugins…"), [this] {
        DialogAboutPlugins(this).exec();
        //        PluginDialog dialog(qApp->applicationDirPath(), {}, this);
        //        dialog.exec();
    });
    action->setStatusTip(tr("Show loaded plugins…"));
}

void MainWindow::createActionsZoom()
{
    auto vievMenu = menuBar()->addMenu(tr("&Viev"));
    vievMenu->setObjectName("vievMenu");
    zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    zoomToolBar->setToolTip(tr("Zoom ToolBar"));
    // Fit best
    auto action = zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), ui->graphicsView, &GraphicsView::zoomFit);
    action->setShortcut(QKeySequence::FullScreen);
    vievMenu->addAction(action);
    // 100%
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), ui->graphicsView, &GraphicsView::zoom100);
    action->setShortcut(tr("Ctrl+0"));
    vievMenu->addAction(action);
    // Zoom in
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), ui->graphicsView, &GraphicsView::zoomIn);
    action->setShortcut(QKeySequence::ZoomIn);
    vievMenu->addAction(action);
    // Zoom out
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), ui->graphicsView, &GraphicsView::zoomOut);
    action->setShortcut(QKeySequence::ZoomOut);
    vievMenu->addAction(action);
    // Separator
    zoomToolBar->addSeparator();
    vievMenu->addSeparator();
    // Zoom to selected
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-to-selected"), tr("Zoom to selected"), ui->graphicsView, &GraphicsView::zoomToSelected);
    action->setShortcut(QKeySequence("F12"));
    vievMenu->addAction(action);
}

void MainWindow::createActionsToolPath()
{
    QMenu* menu = menuBar()->addMenu(tr("&Paths"));

    toolpathToolBar = addToolBar(tr("Toolpath"));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));
    toolpathToolBar->setToolTip(tr("Toolpath"));

    connect(m_dockWidget, &DockWidget::visibilityChanged, [this](bool visible) { if (!visible) resetToolPathsActions(); });
    addDockWidget(Qt::RightDockWidgetArea, m_dockWidget);

    {
        toolpathActions[GCode::Profile] = toolpathToolBar->addAction(QIcon::fromTheme("profile-path"), tr("Pro&file"), [this] {
            createDockWidget<ProfileForm>(GCode::Profile); //createDockWidget(new ProfileForm(dockWidget), GCode::Profile);
            toolpathActions[GCode::Profile]->setChecked(true);
        });
        toolpathActions[GCode::Profile]->setShortcut(QKeySequence("Ctrl+Shift+F"));
        menu->addAction(toolpathActions[GCode::Profile]);
    }

    {
        toolpathActions[GCode::Pocket] = toolpathToolBar->addAction(QIcon::fromTheme("pocket-path"), tr("&Pocket"), [this] {
            createDockWidget<PocketOffsetForm>(GCode::Pocket); //createDockWidget(new PocketOffsetForm(dockWidget), GCode::Pocket);
            toolpathActions[GCode::Pocket]->setChecked(true);
        });
        toolpathActions[GCode::Pocket]->setShortcut(QKeySequence("Ctrl+Shift+P"));
        menu->addAction(toolpathActions[GCode::Pocket]);
    }

    {
        toolpathActions[GCode::Raster] = toolpathToolBar->addAction(QIcon::fromTheme("raster-path"), tr("&PocketR"), [this] { ////////////////
            createDockWidget<PocketRasterForm>(GCode::Raster); //createDockWidget(new PocketRasterForm(dockWidget), GCode::Raster);
            toolpathActions[GCode::Raster]->setChecked(true);
        });
        toolpathActions[GCode::Raster]->setShortcut(QKeySequence("Ctrl+Shift+R"));
        menu->addAction(toolpathActions[GCode::Raster]);
    }

    {
        toolpathActions[GCode::Voronoi] = toolpathToolBar->addAction(QIcon::fromTheme("voronoi-path"), tr("&Voronoi"), [this] {
            createDockWidget<VoronoiForm>(GCode::Voronoi); //createDockWidget(new VoronoiForm(dockWidget), GCode::Voronoi);
            toolpathActions[GCode::Voronoi]->setChecked(true);
        });
        toolpathActions[GCode::Voronoi]->setShortcut(QKeySequence("Ctrl+Shift+V"));
        menu->addAction(toolpathActions[GCode::Voronoi]);
    }

    {
        toolpathActions[GCode::Thermal] = toolpathToolBar->addAction(QIcon::fromTheme("thermal-path"), tr("&Thermal Insulation"), [this] {
            if (ThermalForm::canToShow()) {
                createDockWidget<ThermalForm>(GCode::Thermal); //createDockWidget(new ThermalForm(dockWidget), GCode::Thermal);
                toolpathActions[GCode::Thermal]->setChecked(true);
            } else
                toolpathActions[GCode::Thermal]->setChecked(false);
        });
        toolpathActions[GCode::Thermal]->setShortcut(QKeySequence("Ctrl+Shift+T"));
        menu->addAction(toolpathActions[GCode::Thermal]);
    }

    {
        toolpathActions[GCode::Drill] = toolpathToolBar->addAction(QIcon::fromTheme("drill-path"), tr("&Drilling"), [this] {
            if (DrillForm::canToShow()) {
                createDockWidget<DrillForm>(GCode::Drill); //createDockWidget(new DrillForm(dockWidget), GCode::Drill);
                toolpathActions[GCode::Drill]->setChecked(true);
            } else
                toolpathActions[GCode::Drill]->setChecked(false);
        });
        toolpathActions[GCode::Drill]->setShortcut(QKeySequence("Ctrl+Shift+D"));
        menu->addAction(toolpathActions[GCode::Drill]);
    }

    for (QAction* action_ : toolpathActions)
        action_->setCheckable(true);
}

void MainWindow::createActionsGraphics()
{

    QToolBar* tb = addToolBar(tr("Graphics Items"));
    tb->setObjectName("GraphicsItemsToolBar");
    tb->setToolTip(tr("Graphics Items"));
    // tb->setEnabled(false);
    QAction* action = nullptr;
    //    {
    //        action = tb->addAction(QIcon::fromTheme("draw-rectangle"), tr("Rect"));
    //        action->setCheckable(true);
    //        connect(action, &QAction::triggered, [action](bool checked) {
    //            Shapes::Constructor::setType(checked ? static_cast<int>(GiType::ShapeR) : 0, checked ? action : nullptr);
    //        });
    //    }

    for (auto& [type, pair] : App::shapeInterfaces()) {
        auto& [shInt, pobj] = pair;
        action = tb->addAction(QIcon::fromTheme("draw-ellipse"), shInt->info().value("Name").toString());
        action->setCheckable(true);
        connect(pobj, SIGNAL(actionUncheck(bool)), action, SLOT(setChecked(bool)));
        connect(action, &QAction::toggled, [shInt = shInt](bool checked) {
            checked ? ShapePluginInterface::setShapePI(shInt)
                    : ShapePluginInterface::finalizeShape_();
        });
    }

    //    {
    //        action = tb->addAction(QIcon::fromTheme("draw-line"), tr("Line"));
    //        action->setCheckable(true);
    //        connect(action, &QAction::triggered, [action](bool checked) {
    //            Shapes::Constructor::setType(checked ? static_cast<int>(GiType::ShapeL) : 0, checked ? action : nullptr);
    //        });
    //    }
    //    {
    //        action = tb->addAction(QIcon::fromTheme("draw-ellipse-arc"), tr("Arc"));
    //        action->setCheckable(true);
    //        connect(action, &QAction::triggered, [action](bool checked) {
    //            Shapes::Constructor::setType(checked ? static_cast<int>(GiType::ShapeA) : 0, checked ? action : nullptr);
    //        });
    //    }
    //    {
    //        action = tb->addAction(QIcon::fromTheme("draw-text"), tr("Text"));
    //        action->setCheckable(true);
    //        connect(action, &QAction::triggered, [action](bool checked) {
    //            Shapes::Constructor::setType(checked ? static_cast<int>(GiType::ShapeT) : 0, checked ? action : nullptr);
    //        });
    //    }
    //    // tb->addAction(QIcon::fromTheme("draw-line"), tr("line"), [this] {  ui->graphicsView->setPt(Line); });
    //    // tb->addAction(QIcon::fromTheme("draw-ellipse-arc"), tr("Arc"), [this] {  ui->graphicsView->setPt(ArcPT); });
    //    // tb->addAction(QIcon::fromTheme("draw-text"), tr("Text"), [this] {  ui->graphicsView->setPt(Text); });
    //    tb->addSeparator();

    //    auto ex = [](ClipType type) {
    //        QList<QGraphicsItem*> si = App::scene()->selectedItems();
    //        QList<GraphicsItem*> rmi;
    //        for (QGraphicsItem* item : si) {
    //            if (static_cast<GiType>(item->type()) == GiType::Gerber) {
    //                DataSolidItem* gitem = static_cast<DataSolidItem*>(item);
    //                Clipper clipper;
    //                clipper.AddPaths(gitem->paths(), ptSubject, true);
    //                for (QGraphicsItem* clipItem : si) {
    //                    if (static_cast<GiType>(clipItem->type()) >= GiType::ShapeC) {
    //                        clipper.AddPaths(static_cast<GraphicsItem*>(clipItem)->paths(), ptClip, true);
    //                    }
    //                }
    //                clipper.Execute(type, *gitem->rPaths(), pftEvenOdd, pftPositive);
    //                if (gitem->rPaths()->isEmpty()) {
    //                    rmi.push_back(gitem);
    //                } else {
    //                    ReversePaths(*gitem->rPaths());
    //                    gitem->redraw();
    //                }
    //            }
    //        }
    //        for (GraphicsItem* item : rmi) {
    //            delete item->file()->itemGroup()->takeAt(item->file()->itemGroup()->indexOf(item));
    //        }
    //    };
    //    tb->addAction(QIcon::fromTheme("path-union"), tr("Union"), [ex] { ex(ctUnion); });
    //    tb->addAction(QIcon::fromTheme("path-difference"), tr("Difference"), [ex] { ex(ctDifference); });
    //    tb->addAction(QIcon::fromTheme("path-exclusion"), tr("Exclusion"), [ex] { ex(ctXor); });
    //    tb->addAction(QIcon::fromTheme("path-intersection"), tr("Intersection"), [ex] { ex(ctIntersection); });
}

void MainWindow::saveGCodeFile(int id)
{
    qDebug() << __FUNCTION__;
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
    qDebug() << __FUNCTION__;
}

void MainWindow::saveSelectedGCodeFiles()
{
    qDebug() << __FUNCTION__;
    if (m_project->pinsPlacedMessage())
        return;

    mvector<GCode::File*> gcFiles(m_project->files<GCode::File>());
    for (size_t i = 0; i < gcFiles.size(); ++i) {
        if (!gcFiles[i]->itemGroup()->isVisible())
            gcFiles.remove(i--);
    }

    using Key = QPair<uint, Side>;

    QMap<Key, QList<GCode::File*>> mm;
    for (GCode::File* file : gcFiles)
        mm[{ file->getTool().hash(), file->side() }].push_back(file);

    for (const Key& key : mm.keys()) {
        QList<GCode::File*> files(mm.value(key));
        if (files.size() < 2) {
            for (GCode::File* file : files) {
                QString name(GCode::GCUtils::getLastDir().append(file->shortName()));
                if (!name.endsWith("tap"))
                    name += QStringList({ "_TS", "_BS" })[file->side()];
                name = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save GCode file"), name, QObject::tr("GCode (*.%1)").arg(GCode::Settings::fileExtension()));
                if (name.isEmpty())
                    return;
                file->save(name);
                file->itemGroup()->setVisible(false);
            }
        } else {
            QString name(GCode::GCUtils::getLastDir().append(files.first()->getTool().nameEnc()));
            if (!name.endsWith("tap"))
                name += QStringList({ "_TS", "_BS" })[files.first()->side()];
            name = QFileDialog::getSaveFileName(nullptr, QObject::tr("Save GCode file"), name, QObject::tr("GCode (*.%1)").arg(GCode::Settings::fileExtension()));
            if (name.isEmpty())
                return;
            mvector<QString> sl;
            for (int i = 0; i < files.size(); ++i) {
                GCode::File* file = files[i];
                file->itemGroup()->setVisible(false);
                file->initSave();
                if (i == 0)
                    file->statFile();
                file->addInfo(true);
                file->genGcodeAndTile();
                if (i == (files.size() - 1))
                    file->endFile();
                sl.push_back(file->gCodeText());
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

    if (mm.isEmpty())
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
    SettingsDialog().readSettings();
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    restoreState(settings.value("state", QByteArray()).toByteArray());
    lastPath = settings.value("lastPath").toString();
    if (qApp->applicationDirPath().contains("GERBER_X3/bin")) {
        loadFile(settings.value("project").toString());
    }
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("project", m_project->name());
    settings.endGroup();
}

void MainWindow::selectAll()
{
    if (/*  */ toolpathActions.contains(GCode::Thermal)
        && toolpathActions[GCode::Thermal]->isChecked()) {
        for (QGraphicsItem* item : App::scene()->items()) {
            item->setSelected(static_cast<GiType>(item->type()) == GiType::PrThermal);
        }
    } else if (toolpathActions.contains(GCode::Drill)
        && toolpathActions[GCode::Drill]->isChecked()) {
        for (QGraphicsItem* item : App::scene()->items()) {
            const auto type(static_cast<GiType>(item->type()));
            item->setSelected( //
                type == GiType::PrApetrure || //
                type == GiType::PrDrill || //
                type == GiType::PrSlot);
        }
    } else {
        for (QGraphicsItem* item : App::scene()->items())
            if (item->isVisible())
                item->setSelected(true);
    }
}

void MainWindow::deSelectAll()
{
    qDebug() << __FUNCTION__;
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
    for (QAction* action : toolpathActions)
        action->setChecked(false);
}

void MainWindow::documentWasModified()
{
    setWindowModified(m_project->isModified());
}

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
    qDebug() << __FUNCTION__ << file;
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
    if (dynamic_cast<T*>(m_dockWidget->widget()))
        return;

    auto dwContent = new T(m_dockWidget);
    dwContent->setObjectName(typeid(T).name());

    for (QAction* action : qAsConst(toolpathActions))
        action->setChecked(false);

    toolpathActions[type]->setChecked(true);

    m_dockWidget->pop();
    m_dockWidget->push(dwContent);

    //    if (m_dockWidget->widget()) {
    //        //if(dynamic_cast<>(m_dockWidget->widget()))
    //        if (m_dockWidget->widget()->objectName() == "ErrorDialog") {
    //            m_dockWidget->widget()->close();
    //            QTimer::singleShot(10, [widget = m_dockWidget->widget()] { delete widget; });
    //        } else
    //            delete m_dockWidget->widget();
    //    }
    //    m_dockWidget->setWidget(dwContent);

    m_dockWidget->show();
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    QMainWindow::contextMenuEvent(event);
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

    qDebug() << __FUNCTION__ << qtTranslator.load(qtTr, trFolder);
    qDebug() << __FUNCTION__ << appTranslator.load(appTr, trFolder);

    qApp->installTranslator(&qtTranslator);
    qApp->installTranslator(&appTranslator);
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
        }
    } else {
        if (m_project->contains(fileName) != -1
            && QMessageBox::warning(this, tr("Warning"), QString(tr("Do you want to reload file %1?")).arg(QFileInfo(fileName).fileName()), QMessageBox::Ok | QMessageBox::Cancel)
                == QMessageBox::Cancel) {
            return;
        }
        qDebug() << __FUNCTION__;
        for (auto& [type, tuple] : App::fileInterfaces()) {
            auto& [parser, pobj] = tuple;
            qDebug() << __FUNCTION__ << pobj;
            if (parser->thisIsIt(fileName)) {
                qDebug() << __FUNCTION__;
                emit parseFile(fileName, int(type));
                //emit /**/ (this->*parseSignal[int(type)])(fileName);
                return;
            }
        }
    }

    /* else if (excellonParser->isDrillFile(fileName)) {
            emit parseExcellonFile(fileName);
        } else if (dxfParser->isDxfFile(fileName)) {
            emit parseDxfFile(fileName);
        } else
            emit parseGerberFile(fileName);*/
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

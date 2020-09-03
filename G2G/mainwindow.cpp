// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "mainwindow.h"
#include "aboutform.h"
#include "forms/drillform/drillform.h"
#include "forms/gcodepropertiesform.h"
#include "forms/pocketoffsetform.h"
#include "forms/pocketrasterform.h"
#include "forms/profileform.h"
#include "forms/thermal/thermalform.h"
#include "forms/voronoiform.h"
#include "gi/bridgeitem.h"
#include "project.h"
#include "settingsdialog.h"
#include "shheaders.h"
#include "tooldatabase/tooldatabase.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QOperatingSystemVersion>
#include <QPrintPreviewDialog>
#include <QPrinter>
#include <QProgressDialog>
#include <QTableView>
#include <QToolBar>
#include <excellondialog.h>
#include <exparser.h>
#include <gbrnode.h>
#include <gbrparser.h>
#include <gcode.h>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , recentFiles(this, "recentFileList")
    , recentProjects(this, "recentProjectsList")
    , gerberParser(new Gerber::Parser)
    , excellonParser(new Excellon::Parser)
    , m_project(new Project)

{
    setupUi(this);

    scene = reinterpret_cast<Scene*>(graphicsView->scene());

    initWidgets();

    {
        new Marker(Marker::Home);
        new Marker(Marker::Zero);
        new Pin(scene);
        new Pin(scene);
        new Pin(scene);
        new Pin(scene);
        new LayoutFrames();
    }

    gerberParser->moveToThread(&parserThread);
    connect(this, &MainWindow::parseGerberFile, gerberParser, &FileParser::parseFile, Qt::QueuedConnection);
    connect(gerberParser, &FileParser::fileReady, this, &MainWindow::addFileToPro, Qt::QueuedConnection);
    connect(gerberParser, &FileParser::fileProgress, this, &MainWindow::fileProgress);
    connect(gerberParser, &FileParser::fileError, this, &MainWindow::fileError);

    excellonParser->moveToThread(&parserThread);
    connect(this, &MainWindow::parseExcellonFile, excellonParser, &FileParser::parseFile, Qt::QueuedConnection);
    connect(excellonParser, &FileParser::fileReady, this, &MainWindow::addFileToPro, Qt::QueuedConnection);
    connect(excellonParser, &FileParser::fileProgress, this, &MainWindow::fileProgress);
    connect(excellonParser, &Gerber::Parser::fileError, this, &MainWindow::fileError);

    connect(&parserThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    connect(&parserThread, &QThread::finished, excellonParser, &QObject::deleteLater);

    parserThread.start(QThread::HighestPriority);

    connect(graphicsView, &GraphicsView::fileDroped, this, &MainWindow::loadFile);
    connect(graphicsView, &GraphicsView::customContextMenuRequested, this, &MainWindow::onCustomContextMenuRequested);

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
    ToolHolder::readTools();
    setCurrentFile(QString());

    {
        GCodePropertiesForm(nullptr); // init vars;
    }

    readSettings();

    if constexpr (0) { // (need for debug)
        QTimer::singleShot(120, [this] { selectAll(); });
        QTimer::singleShot(150, [this] { toolpathActionList[GCode::Pocket]->triggered(); });
        QTimer::singleShot(170, [this] { dockWidget->findChild<QPushButton*>("pbCreate")->click(); });
    }
    App::m_mainWindow = this;
}

MainWindow::~MainWindow()
{
    parserThread.quit();
    parserThread.wait();
    App::m_mainWindow = nullptr;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
    if (qApp->applicationDirPath().contains("GERBER_X2/bin") || maybeSave()) {
        writeSettings();
        dockWidget->close();
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
        dockWidget->close();
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
    connect(m_project, &Project::changed, this, &MainWindow::documentWasModified);
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::createActions()
{
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
        m_project, &Project::saveSelectedToolpaths);
    action->setStatusTip(tr("Save selected toolpaths"));
    fileToolBar->addAction(action);
    // Export PDF
    action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."),
        scene, &Scene::RenderPdf);
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

    QToolBar* toolpathToolBar = addToolBar(tr("Service"));
    toolpathToolBar->setObjectName("tbService");
    //toolpathToolBar->addSeparator();
    serviceMenu->addSeparator();
    {
        action = toolpathToolBar->addAction(QIcon::fromTheme("node"), tr("&G-Code Properties"), [this] {
            createDockWidget<GCodePropertiesForm>(/*new GCodePropertiesForm(dockWidget),*/ GCode::GCodeProperties);
        });
        action->setShortcut(QKeySequence("Ctrl+Shift+G"));
        serviceMenu->addAction(action);
        toolpathActionList[GCode::GCodeProperties] = action;
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
        graphicsView->zoomFit();
    }));

    serviceMenu->addSeparator();
    toolpathToolBar->addSeparator();
    serviceMenu->addAction(action = toolpathToolBar->addAction(QIcon::fromTheme("snap-to-grid"), tr("Snap to grid"), [](bool checked) { GlobalSettings::setSnap(checked); }));
    action->setCheckable(true);
}

void MainWindow::createActionsHelp()
{
    helpMenu = menuBar()->addMenu(tr("&Help"));
    QAction* action = helpMenu->addAction(tr("&About"), this, &MainWindow::about);
    action->setStatusTip(tr("Show the application's About box"));

    action = helpMenu->addAction(tr("About &Qt"), qApp, &QApplication::aboutQt);
    action->setStatusTip(tr("Show the Qt library's About box"));
}

void MainWindow::createActionsZoom()
{
    auto vievMenu = menuBar()->addMenu(tr("&Viev"));
    vievMenu->setObjectName("vievMenu");
    zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    // Fit best
    auto action = zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), graphicsView, &GraphicsView::zoomFit);
    action->setShortcut(QKeySequence::FullScreen);
    vievMenu->addAction(action);
    // 100%
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), graphicsView, &GraphicsView::zoom100);
    action->setShortcut(tr("Ctrl+0"));
    vievMenu->addAction(action);
    // Zoom in
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), graphicsView, &GraphicsView::zoomIn);
    action->setShortcut(QKeySequence::ZoomIn);
    vievMenu->addAction(action);
    // Zoom out
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), graphicsView, &GraphicsView::zoomOut);
    action->setShortcut(QKeySequence::ZoomOut);
    vievMenu->addAction(action);
    // Separator
    zoomToolBar->addSeparator();
    vievMenu->addSeparator();
    // Zoom to selected
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-to-selected"), tr("Zoom to selected"), graphicsView, &GraphicsView::zoomToSelected);
    action->setShortcut(QKeySequence("F12"));
    vievMenu->addAction(action);
}

void MainWindow::createActionsToolPath()
{
    QMenu* menu = menuBar()->addMenu(tr("&Paths"));

    toolpathToolBar = addToolBar(tr("Toolpath"));
    toolpathToolBar->setObjectName(QStringLiteral("toolpathToolBar"));

    dockWidget = new DockWidget(this);
    dockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    dockWidget->setObjectName(QStringLiteral("dwCreatePath"));

    connect(dockWidget, &DockWidget::visibilityChanged, [this](bool visible) { if (!visible) resetToolPathsActions(); });
    addDockWidget(Qt::RightDockWidgetArea, dockWidget);

    {
        toolpathActionList[GCode::Profile] = toolpathToolBar->addAction(QIcon::fromTheme("profile-path"), tr("Pro&file"), [this] {
            createDockWidget<ProfileForm>(GCode::Profile); //createDockWidget(new ProfileForm(dockWidget), GCode::Profile);
            toolpathActionList[GCode::Profile]->setChecked(true);
        });
        toolpathActionList[GCode::Profile]->setShortcut(QKeySequence("Ctrl+Shift+F"));
        menu->addAction(toolpathActionList[GCode::Profile]);
    }
    {
        toolpathActionList[GCode::Pocket] = toolpathToolBar->addAction(QIcon::fromTheme("pocket-path"), tr("&Pocket"), [this] {
            createDockWidget<PocketOffsetForm>(GCode::Pocket); //createDockWidget(new PocketOffsetForm(dockWidget), GCode::Pocket);
            toolpathActionList[GCode::Pocket]->setChecked(true);
        });
        toolpathActionList[GCode::Pocket]->setShortcut(QKeySequence("Ctrl+Shift+P"));
        menu->addAction(toolpathActionList[GCode::Pocket]);
    }
    {
        toolpathActionList[GCode::Raster] = toolpathToolBar->addAction(QIcon::fromTheme("raster-path"), tr("&PocketR"), [this] { ////////////////
            createDockWidget<PocketRasterForm>(GCode::Raster); //createDockWidget(new PocketRasterForm(dockWidget), GCode::Raster);
            toolpathActionList[GCode::Raster]->setChecked(true);
        });
        toolpathActionList[GCode::Raster]->setShortcut(QKeySequence("Ctrl+Shift+R"));
        menu->addAction(toolpathActionList[GCode::Raster]);
    }
    {
        toolpathActionList[GCode::Voronoi] = toolpathToolBar->addAction(QIcon::fromTheme("voronoi-path"), tr("&Voronoi"), [this] {
            createDockWidget<VoronoiForm>(GCode::Voronoi); //createDockWidget(new VoronoiForm(dockWidget), GCode::Voronoi);
            toolpathActionList[GCode::Voronoi]->setChecked(true);
        });
        toolpathActionList[GCode::Voronoi]->setShortcut(QKeySequence("Ctrl+Shift+V"));
        menu->addAction(toolpathActionList[GCode::Voronoi]);
    }

    {
        toolpathActionList[GCode::Thermal] = toolpathToolBar->addAction(QIcon::fromTheme("thermal-path"), tr("&Thermal Insulation"), [this] {
            if (ThermalForm::canToShow()) {
                createDockWidget<ThermalForm>(GCode::Thermal); //createDockWidget(new ThermalForm(dockWidget), GCode::Thermal);
                toolpathActionList[GCode::Thermal]->setChecked(true);
            } else
                toolpathActionList[GCode::Thermal]->setChecked(false);
        });
        toolpathActionList[GCode::Thermal]->setShortcut(QKeySequence("Ctrl+Shift+T"));
        menu->addAction(toolpathActionList[GCode::Thermal]);
    }

    {
        toolpathActionList[GCode::Drill] = toolpathToolBar->addAction(QIcon::fromTheme("drill-path"), tr("&Drilling"), [this] {
            if (DrillForm::canToShow()) {
                createDockWidget<DrillForm>(GCode::Drill); //createDockWidget(new DrillForm(dockWidget), GCode::Drill);
                toolpathActionList[GCode::Drill]->setChecked(true);
            } else
                toolpathActionList[GCode::Drill]->setChecked(false);
        });
        toolpathActionList[GCode::Drill]->setShortcut(QKeySequence("Ctrl+Shift+D"));
        menu->addAction(toolpathActionList[GCode::Drill]);
    }

    for (QAction* action_ : toolpathActionList)
        action_->setCheckable(true);
}

void MainWindow::createActionsGraphics()
{
    QToolBar* tb = addToolBar(tr("Graphics Items"));
    tb->setObjectName("GraphicsItemsToolBar");
    // tb->setEnabled(false);
    QAction* action = nullptr;
    {
        action = tb->addAction(QIcon::fromTheme("draw-rectangle"), tr("Rect"));
        action->setCheckable(true);
        connect(action, &QAction::triggered, [action](bool checked) {
            Shapes::Constructor::setType(checked ? GiShapeR : 0, checked ? action : nullptr);
        });
    }
    {
        action = tb->addAction(QIcon::fromTheme("draw-ellipse"), tr("Elipse"));
        action->setCheckable(true);
        connect(action, &QAction::triggered, [action](bool checked) {
            Shapes::Constructor::setType(checked ? GiShapeC : 0, checked ? action : nullptr);
        });
    }
    {
        action = tb->addAction(QIcon::fromTheme("draw-line"), tr("Line"));
        action->setCheckable(true);
        connect(action, &QAction::triggered, [action](bool checked) {
            Shapes::Constructor::setType(checked ? GiShapeL : 0, checked ? action : nullptr);
        });
    }
    {
        action = tb->addAction(QIcon::fromTheme("draw-ellipse-arc"), tr("Arc"));
        action->setCheckable(true);
        connect(action, &QAction::triggered, [action](bool checked) {
            Shapes::Constructor::setType(checked ? GiShapeA : 0, checked ? action : nullptr);
        });
    }
    {
        action = tb->addAction(QIcon::fromTheme("draw-text"), tr("Text"));
        action->setCheckable(true);
        connect(action, &QAction::triggered, [action](bool checked) {
            Shapes::Constructor::setType(checked ? GiShapeT : 0, checked ? action : nullptr);
        });
    }
    // tb->addAction(QIcon::fromTheme("draw-line"), tr("line"), [this] { graphicsView->setPt(Line); });
    // tb->addAction(QIcon::fromTheme("draw-ellipse-arc"), tr("Arc"), [this] { graphicsView->setPt(ArcPT); });
    // tb->addAction(QIcon::fromTheme("draw-text"), tr("Text"), [this] { graphicsView->setPt(Text); });
    tb->addSeparator();

    auto ex = [](ClipType type) {
        QList<QGraphicsItem*> si = App::scene()->selectedItems();
        QList<GraphicsItem*> rmi;
        for (QGraphicsItem* item : si) {
            if (item->type() == GiGerber) {
                GerberItem* gitem = reinterpret_cast<GerberItem*>(item);
                Clipper clipper;
                clipper.AddPaths(gitem->paths(), ptSubject, true);
                for (QGraphicsItem* clipItem : si)
                    if (clipItem->type() >= GiShapeC)
                        clipper.AddPaths(reinterpret_cast<GraphicsItem*>(clipItem)->paths(), ptClip, true);
                clipper.Execute(type, *gitem->rPaths(), pftEvenOdd, pftPositive);
                if (gitem->rPaths()->isEmpty()) {
                    rmi.append(gitem);
                } else {
                    ReversePaths(*gitem->rPaths());
                    gitem->redraw();
                }
            }
        }
        for (GraphicsItem* item : rmi) {
            delete item->file()->itemGroup()->takeAt(item->file()->itemGroup()->indexOf(item));
        }
    };
    tb->addAction(QIcon::fromTheme("path-union"), tr("Union"), [ex] { ex(ctUnion); });
    tb->addAction(QIcon::fromTheme("path-difference"), tr("Difference"), [ex] { ex(ctDifference); });
    tb->addAction(QIcon::fromTheme("path-exclusion"), tr("Exclusion"), [ex] { ex(ctXor); });
    tb->addAction(QIcon::fromTheme("path-intersection"), tr("Intersection"), [ex] { ex(ctIntersection); });
}

void MainWindow::createPinsPath()
{
    ToolDatabase tdb(this, { Tool::Drill, Tool::EndMill });
    if (tdb.exec()) {
        Tool tool(tdb.tool());

        QPolygonF dst;

        for (Pin* item : Pin::pins()) {
            item->setFlag(QGraphicsItem::ItemIsMovable, false);
            QPointF point(item->pos());
            if (dst.contains(point))
                continue;
            dst.append(point);
        }

        qDebug() << dst.size();

        QSettings settings;
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
        double depth = QInputDialog::getDouble(this, "", tr("Set Depth"), settings.value("Pin/depth").toDouble(), 0, 100, 2);
#else
        bool ok;
        double depth = QInputDialog::getDouble(this, "", tr("Set Depth"), settings.value("Pin/depth").toDouble(), 0, 20, 1, &ok, Qt::WindowFlags(), 1);
        if (!ok)
            return;
#endif

        if (depth == 0.0)
            return;
        settings.setValue("Pin/depth", depth);

        GCode::GCodeParams gcp(tool, depth, GCode::Drill);

        gcp.params[GCode::GCodeParams::NotTile];

        GCode::File* gcode = new GCode::File({ { toPath(dst) } }, gcp);
        gcode->setFileName(tr("Pin_") + tool.nameEnc());
        m_project->addFile(gcode);
    }
}

void MainWindow::newFile()
{
    if (closeProject()) {
        setCurrentFile(QString());
    }
}

void MainWindow::readSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    restoreGeometry(settings.value("geometry", QByteArray()).toByteArray());
    restoreState(settings.value("state", QByteArray()).toByteArray());
    lastPath = settings.value("lastPath").toString();
    if (qApp->applicationDirPath().contains("GERBER_X2/bin")) {
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
    if (focusWidget() && focusWidget()->objectName() == "toolTable") {
        static_cast<QTableView*>(focusWidget())->selectAll();
        return;
    } else {
        for (QGraphicsItem* item : App::scene()->items())
            if (item->isVisible())
                item->setSelected(true);
    }
}

void MainWindow::redo() { }

void MainWindow::printDialog()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, [this](QPrinter* pPrinter) {
        scene->m_drawPdf = true;
        QRectF rect;
        for (QGraphicsItem* item : App::scene()->items())
            if (item->isVisible() && !item->boundingRect().isNull())
                rect |= item->boundingRect();
        QSizeF size(rect.size());
        pPrinter->setMargins({ 10, 10, 10, 10 });
        pPrinter->setPageSizeMM(size + QSizeF(pPrinter->margins().left + pPrinter->margins().right, pPrinter->margins().top + pPrinter->margins().bottom));
        pPrinter->setResolution(4800);

        QPainter painter(pPrinter);
#if QT_DEPRECATED_SINCE(5, 14)
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
#else
        painter.setRenderHint(QPainter::Antialiasing);
#endif
        painter.setTransform(QTransform().scale(1.0, -1.0));
        painter.translate(0, -(pPrinter->resolution() / 25.4) * size.height());
        scene->render(&painter,
            QRectF(0, 0, pPrinter->width(), pPrinter->height()),
            rect,
            Qt::KeepAspectRatio /*IgnoreAspectRatio*/);
        scene->m_drawPdf = false;
    });
    preview.exec();
}

void MainWindow::onCustomContextMenuRequested(const QPoint& pos)
{
    QMenu menu;
    QAction* action = nullptr;
    QGraphicsItem* item = scene->itemAt(graphicsView->mapToScene(pos), graphicsView->transform());

    if (!item)
        return;

    if (item->type() == GiPin) {
        action = menu.addAction(QIcon::fromTheme("drill-path"), tr("&Create path for Pins"), this, &MainWindow::createPinsPath);
        action = menu.addAction(tr("Fixed"), [](bool fl) {
            for (Pin* pin : Pin::pins())
                pin->setFlag(QGraphicsItem::ItemIsMovable, !fl);
        });
        action->setCheckable(true);
        action->setChecked(!(Pin::pins()[0]->flags() & QGraphicsItem::ItemIsMovable));
    } else if (dynamic_cast<Marker*>(item)) {
        action = menu.addAction(tr("Fixed"),
            [=](bool fl) { item->setFlag(QGraphicsItem::ItemIsMovable, !fl); });
        action->setCheckable(true);
        action->setChecked(!(item->flags() & QGraphicsItem::ItemIsMovable));
    }
    //    else if (item->type() == GiThermalPr) {
    //        if (item->flags() & QGraphicsItem::ItemIsSelectable)
    //            a = menu.addAction(QIcon::fromTheme("list-remove"), tr("Exclude from the calculation"), [=] {
    //                reinterpret_cast<ThermalPreviewItem*>(item)->node()->disable();
    //            });
    //        else
    //            a = menu.addAction(QIcon::fromTheme("list-add"), tr("Include in the calculation"), [=] {
    //                reinterpret_cast<ThermalPreviewItem*>(item)->node()->enable();
    //            });
    //    }
    if (action)
        menu.exec(graphicsView->mapToGlobal(pos + QPoint(24, 0)));
}

void MainWindow::fileProgress(const QString& fileName, int max, int value)
{
    if (max && !value) {
        QProgressDialog* pd = new QProgressDialog(this);
        pd->setCancelButton(nullptr);
        pd->setLabelText(fileName);
        pd->setMaximum(max);
        pd->setModal(true);
        pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
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
    QMessageBox::critical(this, fileName, error);
}

void MainWindow::resetToolPathsActions()
{
    for (QAction* action : toolpathActionList)
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
        toolpathActionList[GCode::Profile]->triggered();
        reinterpret_cast<FormsUtil*>(dockWidget->widget())->editFile(file);
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

void MainWindow::loadFile(const QString& fileName)
{
    if (m_project->contains(fileName) != -1
        && QMessageBox::warning(this, tr("Warning"), QString(tr("Do you want to reload file %1?")).arg(QFileInfo(fileName).fileName()), QMessageBox::Ok | QMessageBox::Cancel)
            == QMessageBox::Cancel) {
        return;
    }
    QFile file(fileName);
    if (!file.exists())
        return;
    if (file.open(QFile::ReadOnly)) {
        lastPath = QFileInfo(fileName).absolutePath();
        if (fileName.endsWith(".g2g")) {
            if (closeProject()) {
                m_project->open(file);
                setCurrentFile(fileName);
                QTimer::singleShot(100, Qt::CoarseTimer, graphicsView, &GraphicsView::zoomFit);
            }
        } else if (excellonParser->isDrillFile(fileName)) {
            emit parseExcellonFile(fileName);
        } else
            emit parseGerberFile(fileName);
        return;
    }
    QMessageBox::warning(this, tr("Warning"), tr("Cannot read file %1:\n%2.").arg(QDir::toNativeSeparators(fileName), file.errorString()));
}

bool MainWindow::saveFile(const QString& fileName)
{
    QFile file(fileName);
    if (file.open(QFile::WriteOnly)) {
        QApplication::setOverrideCursor(Qt::WaitCursor);
        m_project->save(file);
        QApplication::restoreOverrideCursor();
        setCurrentFile(fileName);
        statusBar()->showMessage(tr("File saved"), 2000);
        return true;
    }
    QMessageBox::warning(this,
        tr("Warning"),
        tr("Cannot write file %1:\n%2.")
            .arg(QDir::toNativeSeparators(fileName), file.errorString()));
    return false;
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

void MainWindow::addFileToPro(AbstractFile* file)
{
    if (m_project->isUntitled()) {
        QString name(QFileInfo(file->name()).path());
        setCurrentFile(name + "/" + name.split('/').last() + ".g2g");
    }
    m_project->addFile(file);
    recentFiles.prependToRecentFiles(file->name());
    if (file->type() == FileType::Gerber)
        Gerber::Node::repaintTimer()->start();
    graphicsView->zoomFit();
}

QString MainWindow::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

template <class T>
void MainWindow::createDockWidget(int type)
{
    if (dynamic_cast<T*>(dockWidget->widget()))
        return;

    auto dwContent = new T(dockWidget);
    dwContent->setObjectName(QStringLiteral("dwContents"));

    for (QAction* action : toolpathActionList)
        action->setChecked(false);

    toolpathActionList[type]->setChecked(true);

    if (dockWidget->widget())
        delete dockWidget->widget();
    dockWidget->setWidget(dwContent);
    dockWidget->show();
}

void MainWindow::contextMenuEvent(QContextMenuEvent* event)
{
    QMainWindow::contextMenuEvent(event);
}

QMenu* MainWindow::createPopupMenu()
{
    QMenu* menu = QMainWindow::createPopupMenu();
    menu->removeAction(dockWidget->toggleViewAction());
    menu->removeAction(toolpathToolBar->toggleViewAction());
    menu->removeAction(treeDockWidget->toggleViewAction());
    menu->addAction(tr("Icon size = 24"), [this]() { setIconSize(QSize(24, 24)); });
    menu->addAction(tr("Icon size = 48"), [this]() { setIconSize(QSize(48, 48)); });
    menu->addAction(tr("Icon size = 72"), [this]() { setIconSize(QSize(72, 72)); });
    return menu;
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

    if (files.filter(QRegExp(".+g2g$")).size()) {
        loadFile(files.at(files.indexOf(QRegExp(".+g2g$"))));
        return;
    } else {
        for (QString& fileName : files) {
            loadFile(fileName);
        }
    }
    //    QString name(QFileInfo(files.first()).path());
    //    setCurrentFile(name + "/" + name.split('/').last() + ".g2g");
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

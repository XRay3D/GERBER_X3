#include "mainwindow.h"
#include "aboutform.h"
#include "forms/drillform/drillform.h"
#include "forms/gcodepropertiesform.h"
#include "forms/pocketform.h"
#include "forms/profileform.h"
#include "forms/thermal/thermalform.h"
#include "forms/voronoiform.h"
#include "gi/bridgeitem.h"
#include "project.h"
#include "settingsdialog.h"
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
#include <filetree/gerbernode.h>
#include <gbrparser.h>
#include <gcfile.h>
#include <sh/constructor.h>

MainWindow* MainWindow::self = nullptr;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , gerberParser(new Gerber::Parser)
    , excellonParser(new Excellon::Parser)
    , pro(new Project)
{
    setupUi(this);

    scene = reinterpret_cast<Scene*>(graphicsView->scene());

    initWidgets();

    GCodePropertiesForm::zeroPoint = new Marker(Marker::Zero);
    GCodePropertiesForm::homePoint = new Marker(Marker::Home);
    new Pin[4];

    gerberParser->moveToThread(&parserThread);
    connect(this, &MainWindow::parseGerberFile, gerberParser, &FileParser::parseFile, Qt::QueuedConnection);
    connect(gerberParser, &FileParser::fileReady, this, &MainWindow::addFileToPro, Qt::QueuedConnection);
    connect(gerberParser, &FileParser::fileProgress, this, &MainWindow::fileProgress);
    connect(gerberParser, &FileParser::fileError, this, &MainWindow::fileError);

    excellonParser->moveToThread(&parserThread);
    connect(this, &MainWindow::parseExcellonFile, excellonParser, &FileParser::parseFile, Qt::QueuedConnection);
    connect(excellonParser, &FileParser::fileReady, this, &MainWindow::addFileToPro, Qt::QueuedConnection);
    connect(excellonParser, &Gerber::Parser::fileError, this, &MainWindow::fileError);

    connect(&parserThread, &QThread::finished, gerberParser, &QObject::deleteLater);
    connect(&parserThread, &QThread::finished, excellonParser, &QObject::deleteLater);

    parserThread.start(QThread::HighestPriority);

    connect(graphicsView, &GraphicsView::fileDroped, this, &MainWindow::loadFile);
    connect(graphicsView, &GraphicsView::customContextMenuRequested, this, &MainWindow::onCustomContextMenuRequested);

    if (QOperatingSystemVersion::currentType() == QOperatingSystemVersion::Windows) {
        if (QOperatingSystemVersion::current().majorVersion() > 7) {
            setStyleSheet("QGroupBox, .QFrame {"
                          "background-color: white;"
                          "border: 1px solid gray;"
                          "}"
                          "QGroupBox {"
                          "margin-top: 3ex;" /* leave space at the top for the title */
                          "}"
                          "QGroupBox::title {"
                          "subcontrol-origin: margin;"
                          "subcontrol-position: top center;" /* position at the top center */
                          "}");

        } else {
            setStyleSheet("QGroupBox, .QFrame {"
                          "background-color: white;"
                          "border: 1px solid gray;"
                          "border-radius: 3px;" // Win 7
                          "}"
                          "QGroupBox {"
                          "margin-top: 3ex;" /* leave space at the top for the title */
                          "}"
                          "QGroupBox::title {"
                          "subcontrol-origin: margin;"
                          "subcontrol-position: top center;" /* position at the top center */
                          "}");
        }
    } else {
        setStyleSheet("QGroupBox, .QFrame {"
                      "background-color: white;"
                      "border: 1px solid gray;"
                      "border-radius: 3px;" // other
                      "}"
                      "QGroupBox {"
                      "margin-top: 3ex;" /* leave space at the top for the title */
                      "}"
                      "QGroupBox::title {"
                      "subcontrol-origin: margin;"
                      "subcontrol-position: top center;" /* position at the top center */
                      "}");
    }

    ToolHolder::readTools();
    setCurrentFile(QString());
    readSettings();
    GCodePropertiesForm(); // init vars;
    self = this;
}

MainWindow::~MainWindow()
{
    parserThread.quit();
    parserThread.wait();
    self = nullptr;
}

void MainWindow::closeEvent(QCloseEvent* event)
{
#ifdef QT_DEBUG
    writeSettings();
    event->accept();
    return;
#endif
#ifndef QT_DEBUG
    if (maybeSave()) {
        writeSettings();
        dockWidget->close();
        FileModel::closeProject();
        qApp->closeAllWindows();
        event->accept();
    } else {
        event->ignore();
    }
#endif
}

bool MainWindow::closeProject()
{
    if (maybeSave()) {
        dockWidget->close();
        FileModel::closeProject();
        setCurrentFile(QString());
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
    connect(pro, &Project::changed, this, &MainWindow::documentWasModified);
    setUnifiedTitleAndToolBarOnMac(true);
}

void MainWindow::createActions()
{
    // fileMenu
    createActionsFile();
    // serviceMenu
    createActionsService();
    // zoomToolBar
    createActionsZoom();
    // Selection / Delete selected
    createActionsSDS();
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
    QAction* action = fileMenu->addAction(QIcon::fromTheme("project-development-new-template"),
        tr("&New project"),
        this,
        &MainWindow::newFile);
    action->setShortcuts(QKeySequence::New);
    action->setStatusTip(tr("Create a new file"));
    fileToolBar->addAction(action);
    // Open
    action = fileMenu->addAction(QIcon::fromTheme("document-open"), tr("&Open..."), this, &MainWindow::open);
    action->setShortcuts(QKeySequence::Open);
    action->setStatusTip(tr("Open an existing file"));
    fileToolBar->addAction(action);
    // Save
    action = fileMenu->addAction(QIcon::fromTheme("document-save"), tr("&Save project"), this, &MainWindow::save);
    action->setShortcuts(QKeySequence::Save);
    action->setStatusTip(tr("Save the document to disk"));
    fileToolBar->addAction(action);
    // Save As
    action = fileMenu->addAction(QIcon::fromTheme("document-save-as"),
        tr("Save project &As..."),
        this,
        &MainWindow::saveAs);
    action->setShortcuts(QKeySequence::SaveAs);
    action->setStatusTip(tr("Save the document under a new name"));
    fileToolBar->addAction(action);
    // Save Selected Tool Paths
    action = fileMenu->addAction(QIcon::fromTheme("document-save-all"), tr("&Save Selected Tool Paths..."), pro, &Project::saveSelectedToolpaths);
    action->setStatusTip(tr("Save selected toolpaths"));
    fileToolBar->addAction(action);
    // Export PDF
    action = fileMenu->addAction(QIcon::fromTheme("acrobat"), tr("&Export PDF..."), scene, &Scene::RenderPdf);
    action->setStatusTip(tr("Export to PDF file"));
    fileToolBar->addAction(action);

    fileMenu->addSeparator();
    fileMenu->addSeparator();

    recentMenu = fileMenu->addMenu(tr("Recent..."));
    connect(recentMenu, &QMenu::aboutToShow, this, &MainWindow::updateRecentFileActions);
    recentFileSubMenuAct = recentMenu->menuAction();

    for (int i = 0; i < MaxRecentFiles; ++i) {
        recentFileActs[i] = recentMenu->addAction(QString(), this, &MainWindow::openRecentFile);
        recentFileActs[i]->setVisible(false);
    }
    recentMenu->addSeparator();
    recentFileActs[MaxRecentFiles] = recentMenu->addAction(tr("Clear Recent Files"), [this] {
        QSettings settings;
        writeRecentFiles({}, settings);
        updateRecentFileActions();
        setRecentFilesVisible(MainWindow::hasRecentFiles());
    });

    recentFileSeparator = fileMenu->addSeparator();
    setRecentFilesVisible(MainWindow::hasRecentFiles());

    m_closeAllAct = fileMenu->addAction(QIcon::fromTheme("document-close"),
        tr("&Close project \"%1\""),
        this,
        &MainWindow::closeProject);
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

void MainWindow::createActionsService()
{
    serviceMenu = menuBar()->addMenu(tr("&Service"));
    QAction* action = serviceMenu->addAction(QIcon::fromTheme("configure-shortcuts"), tr("&Settings"), [this] {
        SettingsDialog(this).exec();
    });
    action->setStatusTip(tr("Show the application's settings box"));
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
    zoomToolBar = addToolBar(tr("Zoom ToolBar"));
    //zoomToolBar->setIconSize(QSize(22, 22));
    zoomToolBar->setObjectName(QStringLiteral("zoomToolBar"));
    // zoomToolBar->setMovable(false);
    QAction* action = zoomToolBar->addAction(QIcon::fromTheme("zoom-fit-best"), tr("Fit best"), [this]() {
        graphicsView->zoomFit();
    });
    action->setShortcut(QKeySequence::FullScreen);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-original"), tr("100%"), [this]() {
        graphicsView->zoom100();
    });
    action->setShortcut(tr("Ctrl+0"));
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-in"), tr("Zoom in"), [this]() {
        graphicsView->zoomIn();
    });
    action->setShortcut(QKeySequence::ZoomIn);
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-out"), tr("Zoom out"), [this]() {
        graphicsView->zoomOut();
    });
    action->setShortcut(QKeySequence::ZoomOut);
    zoomToolBar->addSeparator();
    action = zoomToolBar->addAction(QIcon::fromTheme("zoom-to-selected"), tr("Zoom to selected"), [this]() {
        graphicsView->zoomToSelected();
    });
}

void MainWindow::createActionsSDS()
{
    QToolBar* toolBar = addToolBar(tr("Selection"));
    toolBar->setObjectName(QStringLiteral("s"));
    // s->setMovable(false);
    QAction* action = toolBar->addAction(QIcon::fromTheme("edit-select-all"),
        tr("Select all"),
        this,
        &MainWindow::selectAll);
    action->setShortcut(QKeySequence::SelectAll);

    // action = toolBar->addAction(QIcon::fromTheme("document-close"), tr("Redo"), this, &MainWindow::redo);
    // action->setShortcut(QKeySequence::Redo);
    // action = s->addAction(QIcon::fromTheme("layer-delete"), tr("Delete selected"), [this]() {
    // QList<QGraphicsItem*> list;
    // for (QGraphicsItem* item : MyScene::self->items())
    // if (item->isSelected() && item->type() != DrillItemType)
    // list << item;
    // if (list.size() && QMessageBox::question(this,
    //"", "Do you really want to delete the selected items?", QMessageBox::Yes, QMessageBox::No) == QMessageBox::Yes) {
    // for (QGraphicsItem* item : list)
    // if (item->isSelected() && item->type() != DrillItemType)
    // delete item;
    // MyScene::self->setSceneRect(MyScene::self->itemsBoundingRect());
    // MyScene::self->update();
    // MainWindow::self->zero()->resetPos();
    // MainWindow::self->home()->resetPos();
    // Pin::shtifts()[0]->resetPos();
    // }
    // });
    // action->setShortcut(QKeySequence::Delete);
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

    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("profile-path"), tr("Pro&file"), [this] {
        createDockWidget(new ProfileForm(dockWidget), GCode::Profile);
    }));
    toolpathActionList.last()->setShortcut(QKeySequence("Ctrl+Shift+F"));
    menu->addAction(toolpathActionList.last());

    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("pocket-path"), tr("&Pocket"), [this] {
        createDockWidget(new PocketForm(dockWidget), GCode::Pocket);
    }));
    toolpathActionList.last()->setShortcut(QKeySequence("Ctrl+Shift+P"));
    menu->addAction(toolpathActionList.last());

    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("voronoi-path"), tr("&Voronoi"), [this] {
        createDockWidget(new VoronoiForm(dockWidget), GCode::Voronoi);
    }));
    toolpathActionList.last()->setShortcut(QKeySequence("Ctrl+Shift+V"));
    menu->addAction(toolpathActionList.last());

    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("thermal-path"), tr("&Thermal Insulation"), [this] {
        if (ThermalForm::canToShow())
            createDockWidget(new ThermalForm(dockWidget), GCode::Thermal);
        else
            toolpathActionList[GCode::Thermal]->setChecked(false);
    }));
    toolpathActionList.last()->setShortcut(QKeySequence("Ctrl+Shift+T"));
    menu->addAction(toolpathActionList.last());

    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("drill-path"), tr("&Drilling"), [this] {
        if (DrillForm::canToShow())
            createDockWidget(new DrillForm(dockWidget), GCode::Drill);
        else
            toolpathActionList[GCode::Drill]->setChecked(false);
    }));
    toolpathActionList.last()->setShortcut(QKeySequence("Ctrl+Shift+D"));
    menu->addAction(toolpathActionList.last());

    toolpathToolBar->addSeparator();
    toolpathActionList.append(toolpathToolBar->addAction(QIcon::fromTheme("node"), tr("&G-Code Properties"), [this] {
        createDockWidget(new GCodePropertiesForm(dockWidget), GCode::GCodeProperties);
    }));
    toolpathActionList.last()->setShortcut(QKeySequence("Ctrl+Shift+G"));
    menu->addSeparator();
    menu->addAction(toolpathActionList.last());

    toolpathToolBar->addSeparator();
    for (QAction* action : toolpathActionList)
        action->setCheckable(true);

    toolpathToolBar->addAction(QIcon::fromTheme("view-form"), tr("Tool Base"), [this] {
        ToolDatabase tdb(this, {});
        tdb.exec();
    });
    toolpathToolBar->addSeparator();
    toolpathToolBar->addAction(QIcon::fromTheme("snap-nodes-cusp"), tr("Autoplace All Refpoints"), [this] {
        Pin::pins().first()->resetPos();
        GCodePropertiesForm::homePoint->resetPos(false);
        GCodePropertiesForm::zeroPoint->resetPos(false);
        graphicsView->zoomFit();
    });
}

void MainWindow::createActionsGraphics()
{
    QToolBar* tb = addToolBar(tr("Graphics Items"));
    tb->setObjectName("GraphicsItemsToolBar");
    // tb->setEnabled(false);
    QAction* action = tb->addAction(QIcon::fromTheme("draw-rectangle"), tr("Rect"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, [action](bool checked) {
        ShapePr::Constructor::setType(checked ? ShapePr::Rect : ShapePr::NullPT, checked ? action : nullptr);
    });
    action = tb->addAction(QIcon::fromTheme("draw-ellipse"), tr("Elipse"));
    action->setCheckable(true);
    connect(action, &QAction::triggered, [action](bool checked) {
        ShapePr::Constructor::setType(checked ? ShapePr::Elipse : ShapePr::NullPT, checked ? action : nullptr);
    });

    // tb->addAction(QIcon::fromTheme("draw-line"), tr("line"), [this] { graphicsView->setPt(Line); });
    // tb->addAction(QIcon::fromTheme("draw-ellipse-arc"), tr("Arc"), [this] { graphicsView->setPt(ArcPT); });
    // tb->addAction(QIcon::fromTheme("draw-text"), tr("Text"), [this] { graphicsView->setPt(Text); });
    tb->addSeparator();

    auto ex = [](ClipType type) {
        QList<QGraphicsItem*> si = Scene::selectedItems();
        QList<GraphicsItem*> rmi;
        for (QGraphicsItem* item : si) {
            if (item->type() == GiGerber) {
                GerberItem* gitem = reinterpret_cast<GerberItem*>(item);
                Clipper clipper;
                clipper.AddPaths(gitem->paths(), ptSubject, true);
                for (QGraphicsItem* item : si)
                    if (item->type() == GiShapeC)
                        clipper.AddPaths(reinterpret_cast<GraphicsItem*>(item)->paths(), ptClip, true);
                clipper.Execute(type, gitem->rPaths(), pftEvenOdd, pftPositive);
                if (gitem->rPaths().isEmpty()) {
                    rmi.append(gitem);
                } else {
                    ReversePaths(gitem->rPaths());
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
    tb->addSeparator();
    tb->addAction(tr("Undo"))->setEnabled(false);
    tb->addAction(tr("Redo"))->setEnabled(false);
    tb->addSeparator();
    tb->addAction(QIcon::fromTheme("snap-to-grid"), tr("Snap to grid"), [](bool checked) { ShapePr::Constructor::setSnap(checked); })->setCheckable(true);
}

void MainWindow::createPinsPath()
{
    ToolDatabase tdb(this, { Tool::Drill });
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
        double depth = QInputDialog::getDouble(this, "", tr("Set Depth"), settings.value("Pin/depth").toDouble(), 0, 100, 2);
        if (depth == 0.0)
            return;
        settings.setValue("Pin/depth", depth);

        GCode::File* gcode = new GCode::File({ { toPath(dst) } }, tool, depth, GCode::Drill);
        gcode->setFileName("Pin (Tool Id " + QString::number(tool.id()) + ")");
        Project::addFile(gcode);
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
    pro->setName(settings.value("project").toString());
    loadFile(pro->name());

    SettingsDialog().readSettings();
    settings.endGroup();
}

void MainWindow::writeSettings()
{
    QSettings settings;
    settings.beginGroup("MainWindow");
    settings.setValue("geometry", saveGeometry());
    settings.setValue("state", saveState());
    settings.setValue("lastPath", lastPath);
    settings.setValue("project", pro->name());
    settings.endGroup();

    settings.beginGroup("Pin");
    for (int i = 0; i < Pin::pins().size(); ++i) {
        settings.setValue(QString("pos%1").arg(i), Pin::pins()[i]->pos());
    }
    settings.setValue("fixed", bool(Pin::pins()[0]->flags() & QGraphicsItem::ItemIsMovable));
    settings.setValue("worckRect", Pin::pins()[0]->worckRect);
    settings.endGroup();
}

void MainWindow::selectAll()
{
    if (focusWidget() && focusWidget()->objectName() == "toolTable") {
        static_cast<QTableView*>(focusWidget())->selectAll();
        return;
    } else {
        for (QGraphicsItem* item : Scene::items())
            if (item->isVisible())
                item->setSelected(true);
    }
}

void MainWindow::redo() {}

void MainWindow::printDialog()
{
    QPrinter printer(QPrinter::HighResolution);
    QPrintPreviewDialog preview(&printer, this);
    connect(&preview, &QPrintPreviewDialog::paintRequested, [this](QPrinter* printer) {
        scene->m_drawPdf = true;
        QRectF rect;
        for (QGraphicsItem* item : Scene::items())
            if (item->isVisible() && !item->boundingRect().isNull())
                rect |= item->boundingRect();
        QSizeF size(rect.size());
        printer->setMargins({ 10, 10, 10, 10 });
        printer->setPageSizeMM(size
            + QSizeF(printer->margins().left + printer->margins().right,
                  printer->margins().top + printer->margins().bottom));
        printer->setResolution(4800);

        QPainter painter(printer);
        painter.setRenderHint(QPainter::HighQualityAntialiasing);
        painter.setTransform(QTransform().scale(1.0, -1.0));
        painter.translate(0, -(printer->resolution() / 25.4) * size.height());
        scene->render(&painter,
            QRectF(0, 0, printer->width(), printer->height()),
            rect,
            Qt::KeepAspectRatio /*IgnoreAspectRatio*/);
        scene->m_drawPdf = false;
    });
    preview.exec();
}

void MainWindow::onCustomContextMenuRequested(const QPoint& pos)
{
    QMenu menu;
    QAction* a = nullptr;
    QGraphicsItem* item = scene->itemAt(graphicsView->mapToScene(pos), graphicsView->transform());

    if (!item)
        return;

    if (item->type() == GiPin) {
        a = menu.addAction(QIcon::fromTheme("drill-path"), tr("&Create path for Pins"), this, &MainWindow::createPinsPath);
        a = menu.addAction(tr("Fixed"), [](bool fl) {
            for (Pin* item : Pin::pins())
                item->setFlag(QGraphicsItem::ItemIsMovable, !fl);
        });
        a->setCheckable(true);
        a->setChecked(!(Pin::pins()[0]->flags() & QGraphicsItem::ItemIsMovable));
    } else if (dynamic_cast<Marker*>(item)) {
        a = menu.addAction(tr("Fixed"),
            [=](bool fl) { item->setFlag(QGraphicsItem::ItemIsMovable, !fl); });
        a->setCheckable(true);
        a->setChecked(!(item->flags() & QGraphicsItem::ItemIsMovable));
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
    if (a)
        menu.exec(graphicsView->mapToGlobal(pos + QPoint(24, 0)));
}

void MainWindow::fileProgress(const QString& fileName, int max, int value)
{
    static QMap<QString, QProgressDialog*> progress;
    if (max && !value) {
        QProgressDialog* pd = new QProgressDialog(this);
        pd->setCancelButton(nullptr);
        pd->setLabelText(fileName);
        pd->setMaximum(max);
        pd->setModal(true);
        pd->setWindowFlag(Qt::WindowCloseButtonHint, false);
        pd->show();
        progress[fileName] = pd;
    } else if (max == 1 && value == 1) {
        progress[fileName]->hide();
        progress[fileName]->deleteLater();
        progress.remove(fileName);
    } else
        progress[fileName]->setValue(value);
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

void MainWindow::setRecentFilesVisible(bool visible)
{
    recentFileSubMenuAct->setVisible(visible);
    recentFileSeparator->setVisible(visible);
}

QStringList MainWindow::readRecentFiles(QSettings& settings)
{
    QStringList result;
    const int count = settings.beginReadArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        result.append(settings.value(fileKey()).toString());
    }
    settings.endArray();
    return result;
}

void MainWindow::writeRecentFiles(const QStringList& files, QSettings& settings)
{
    const int count = files.size();
    settings.beginWriteArray(recentFilesKey());
    for (int i = 0; i < count; ++i) {
        settings.setArrayIndex(i);
        settings.setValue(fileKey(), files.at(i));
    }
    settings.endArray();
}

bool MainWindow::hasRecentFiles()
{
    QSettings settings;
    const int count = settings.beginReadArray(recentFilesKey());
    settings.endArray();
    return count > 0;
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
    if (pro->isUntitled())
        return saveAs();
    else
        return saveFile(pro->name());
}

bool MainWindow::saveAs()
{
    QString file(
        QFileDialog::getSaveFileName(this, tr("Open File"), pro->name(), tr("Project (*.g2g)")));
    if (file.isEmpty())
        return false;
    return saveFile(file);
}

void MainWindow::documentWasModified()
{
    setWindowModified(pro->isModified());
}

bool MainWindow::maybeSave()
{
    if (!pro->isModified() && pro->size()) {
        return QMessageBox::warning(this, tr("Warning"), tr("Do you want to close this project?"), QMessageBox::Ok | QMessageBox::Cancel) == QMessageBox::Ok;
    } else if (!pro->size()) {
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

void MainWindow::loadFile(const QString& fileName)
{
    if (Project::contains(fileName) != -1
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
                Project::open(file);
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
        pro->save(file);
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
    pro->setName(fileName);
    pro->setModified(false);
    setWindowModified(false);
    if (!pro->isUntitled())
        prependToRecentFiles(pro->name());
    m_closeAllAct->setText(tr("&Close project \"%1\"").arg(strippedName(pro->name())));
    setWindowFilePath(pro->name());
    setWindowFilePath(pro->name());
    setWindowFilePath(pro->name());
}

void MainWindow::addFileToPro(AbstractFile* file)
{
    if (Project::isUntitled()) {
        QString name(QFileInfo(file->name()).path());
        setCurrentFile(name + "/" + name.split('/').last() + ".g2g");
    }
    Project::addFile(file);
    prependToRecentFiles(file->name());
    if (file->type() == FileType::Gerber)
        GerberNode::repaintTimer()->start();
}

void MainWindow::prependToRecentFiles(const QString& fileName)
{
    QSettings settings;
    const QStringList oldRecentFiles = readRecentFiles(settings);
    QStringList recentFiles = oldRecentFiles;
    recentFiles.removeAll(fileName);
    recentFiles.prepend(fileName);
    if (oldRecentFiles != recentFiles)
        writeRecentFiles(recentFiles, settings);
    setRecentFilesVisible(!recentFiles.isEmpty());
    documentWasModified();
}

void MainWindow::updateRecentFileActions()
{
    QSettings settings;

    const QStringList recentFiles = readRecentFiles(settings);
    const int count = qMin(int(MaxRecentFiles), recentFiles.size());
    int i = 0;
    for (; i < count; ++i) {
        const QString fileName = MainWindow::strippedName(recentFiles.at(i));
        recentFileActs[i]->setText(tr("&%1 %2").arg(i + 1).arg(fileName));
        recentFileActs[i]->setData(recentFiles.at(i));
        recentFileActs[i]->setVisible(true);
    }
    for (; i < MaxRecentFiles; ++i)
        recentFileActs[i]->setVisible(false);

    recentFileActs[MaxRecentFiles]->setVisible(count);
}

void MainWindow::openRecentFile()
{
    if (const QAction* action = qobject_cast<const QAction*>(sender()))
        loadFile(action->data().toString());
}

QString MainWindow::strippedName(const QString& fullFileName)
{
    return QFileInfo(fullFileName).fileName();
}

void MainWindow::createDockWidget(QWidget* dwContent, int type)
{
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

QString MainWindow::fileKey()
{
    return QStringLiteral("file");
}

QString MainWindow::recentFilesKey()
{
    return QStringLiteral("recentFileList");
}

void MainWindow::showEvent(QShowEvent* event)
{
    toolpathActionList[GCode::GCodeProperties]->trigger();
    QMainWindow::showEvent(event);
}

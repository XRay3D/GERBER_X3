// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "drillform.h"
#include "ui_drillform.h"

#include "drillmodel.h"
#include "drillpreviewgi.h"
#include "excellon.h"
#include "gbraperture.h"
#include "gbrfile.h"
#include "graphicsview.h"
#include "point.h"
#include "scene.h"
#include "settings.h"
#include "tooldatabase/tooldatabase.h"
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QTimer>

#include "leakdetector.h"

enum { IconSize = 24 };

Paths offset(const Path& path, double offset, bool fl = false)
{
    Paths tmpPpaths;
    ClipperOffset cpOffset;
    if (fl)
        cpOffset.AddPath(path, jtRound, etClosedLine);
    else
        cpOffset.AddPath(path, jtRound, etOpenRound);

    cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);

    for (Path& tmpPath : tmpPpaths)
        tmpPath.append(tmpPath.first());

    return tmpPpaths;
}

/////////////////////////////////////////////
/// \brief draw
/// \param aperture
/// \return
///
QIcon drawApertureIcon(Gerber::AbstractAperture* aperture)
{
    QPainterPath painterPath;

    for (QPolygonF& polygon : toQPolygons(aperture->draw(Gerber::State())))
        painterPath.addPolygon(polygon);

    painterPath.addEllipse(QPointF(0, 0), aperture->drillDiameter() * 0.5, aperture->drillDiameter() * 0.5);

    const QRectF rect = painterPath.boundingRect();

    qreal scale = static_cast<double>(IconSize) / qMax(rect.width(), rect.height());

    double ky = -rect.top() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (static_cast<double>(IconSize) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(IconSize) - rect.width() * scale) / 2;

    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.translate(-kx, ky);
    painter.scale(scale, scale);
    painter.drawPath(painterPath);
    return QIcon(pixmap);
}

QIcon drawDrillIcon()
{
    QPixmap pixmap(IconSize, IconSize);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QRect(0, 0, IconSize - 1, IconSize - 1));
    return QIcon(pixmap);
}

/////////////////////////////////////////////
/// \brief DrillForm::DrillForm
/// \param parent
///
DrillForm::DrillForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DrillForm)
{
    if (App::m_drillForm) {
        QMessageBox::critical(nullptr, "Err", "You cannot create class DrillForm more than 2 times!!!");
        exit(1);
    }
    ui->setupUi(this);
    {
        ui->toolTable->setIconSize(QSize(IconSize, IconSize));
        ui->toolTable->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->toolTable->setWordWrap(false);
        ui->toolTable->horizontalHeader()->setMinimumHeight(ui->toolTable->verticalHeader()->defaultSectionSize());

        connect(ui->toolTable, &QTableView::customContextMenuRequested, this, &DrillForm::on_customContextMenuRequested);
        connect(ui->toolTable, &QTableView::doubleClicked, this, &DrillForm::on_doubleClicked);
        connect(ui->toolTable, &QTableView::clicked, this, &DrillForm::on_clicked);
    }

    {
        auto cornerButton = ui->toolTable->findChild<QAbstractButton*>();
        header = new Header(Qt::Vertical, ui->toolTable);
        ui->toolTable->setVerticalHeader(header);
        if (cornerButton) {
            checkBox = new QCheckBox(cornerButton);
            checkBox->setFocusPolicy(Qt::NoFocus);
            checkBox->setGeometry(Header::getRect(cornerButton->rect()) /*.translated(1, -4)*/);
            connect(checkBox, &QCheckBox::clicked, [this](bool checked) { header->setAll(checked); });
            connect(header, &Header::onChecked, [this](int idx) {
                int fl = 0;
                for (int i = 0; i < model->rowCount(); ++i) {
                    if (model->useForCalc(i))
                        ++fl;

                    if (idx < 0 || idx == i)
                        for (auto item : m_giPeview[model->apertureId(i)])
                            item->changeColor();
                }

                checkBox->setCheckState(!fl ? Qt::Unchecked : (fl == model->rowCount() ? Qt::Checked : Qt::PartiallyChecked));
                ui->pbCreate->setEnabled(fl);
            });
        }
    }
    ui->pbCreate->setEnabled(false);
    ui->rb_drilling->setChecked(true);

    auto updateState = [this] {
        m_worckType = ui->rb_drilling->isChecked() ? GCode::Drill : (ui->rb_profile->isChecked() ? GCode::Profile : GCode::Pocket);
        ui->grbxSide->setEnabled(m_worckType == GCode::Profile);
        ui->grbxDirection->setEnabled(m_worckType == GCode::Profile || m_worckType == GCode::Pocket);
        m_side = ui->rb_on->isChecked() ? GCode::On : (ui->rb_out->isChecked() ? GCode::Outer : GCode::Inner);
    };

    {
        MySettings settings;
        settings.beginGroup("DrillForm");
        if (settings.value("rbClimb").toBool())
            ui->rbClimb->setChecked(true);
        if (settings.value("rbConventional", true).toBool())
            ui->rbConventional->setChecked(true);
        if (settings.value("rb_drilling", true).toBool())
            ui->rb_drilling->setChecked(true);
        if (settings.value("rb_in", true).toBool())
            ui->rb_in->setChecked(true);
        if (settings.value("rb_on").toBool())
            ui->rb_on->setChecked(true);
        if (settings.value("rb_out").toBool())
            ui->rb_out->setChecked(true);
        if (settings.value("rb_pocket").toBool())
            ui->rb_pocket->setChecked(true);
        if (settings.value("rb_profile").toBool())
            ui->rb_profile->setChecked(true);
        settings.getValue(ui->chbxZoomToSelected);
        settings.endGroup();
    }

    connect(ui->rb_drilling, &QRadioButton::clicked, updateState);
    connect(ui->rb_in, &QRadioButton::clicked, updateState);
    connect(ui->rb_on, &QRadioButton::clicked, updateState);
    connect(ui->rb_out, &QRadioButton::clicked, updateState);
    connect(ui->rb_pocket, &QRadioButton::clicked, updateState);
    connect(ui->rb_profile, &QRadioButton::clicked, updateState);

    connect(ui->dsbxDepth, &DepthForm::valueChanged, [](double val) { Row::depth = val; }); //////////////////////////////////////////////////////////////////////////////////////////

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    updateState();

    updateFiles();

    parent->setWindowTitle(ui->label->text());

    App::m_drillForm = this;
}

DrillForm::~DrillForm()
{
    App::m_drillForm = nullptr;
    MySettings settings;
    settings.beginGroup("DrillForm");
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rb_drilling);
    settings.setValue(ui->rb_in);
    settings.setValue(ui->rb_on);
    settings.setValue(ui->rb_out);
    settings.setValue(ui->rb_pocket);
    settings.setValue(ui->rb_profile);
    settings.setValue(ui->chbxZoomToSelected);
    settings.endGroup();
    clear();
    delete ui;
}

void DrillForm::setApertures(const Gerber::ApertureMap* value)
{
    m_type = tAperture;
    clear();
    m_apertures = *value;

    uint count = 0;
    for (auto [_, aperture] : m_apertures)
        if (aperture->isFlashed())
            ++count;

    const Gerber::File* gbrFile = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>());

    std::map<int, QVector<const Gerber::GraphicObject*>> cacheApertures;
    for (const Gerber::GraphicObject& go : *gbrFile)
        if (go.state().dCode() == Gerber::D03)
            cacheApertures[go.state().aperture()].append(&go);

    assert(count == cacheApertures.size()); // assert on != - false

    model = new DrillModel(m_type, count, this);

    for (auto [dCode, aperture] : m_apertures) {
        if (aperture && aperture->isFlashed()) {
            double drillDiameter;
            QString name(aperture->name());
            if (aperture->withHole()) {
                drillDiameter = aperture->drillDiameter();
                name += tr(", drill Ø%1mm").arg(drillDiameter);
            } else if (aperture->type() == Gerber::Circle) {
                drillDiameter = aperture->apertureSize();
            }

            Row& row = model->appendRow(name, drawApertureIcon(aperture.data()), dCode);
            for (const Gerber::GraphicObject* go : cacheApertures[dCode]) {
                DrillPrGI* item = new DrillPrGI(go, dCode, row);
                m_giPeview[dCode].append(QSharedPointer<DrillPrGI>(item));
                App::scene()->addItem(item);
            }

            if (drillDiameter != 0.0)
                pickUpTool(dCode, drillDiameter);
        }
    }

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    connect(model, &DrillModel::set, header, &Header::set);
    //    header->onCheckedV(header->checked());
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::setExcellonTools(const Excellon::Tools& value)
{
    m_type = tTool;
    clear();

    m_tools = value;
    model = new DrillModel(m_type, static_cast<int>(m_tools.size()), this);

    const Excellon::File* exFile = static_cast<Excellon::File*>(ui->cbxFile->currentData().value<void*>());

    std::map<int, QVector<const Excellon::Hole*>> cacheHoles;
    for (const Excellon::Hole& hole : *exFile)
        cacheHoles[hole.state.tCode] << &hole;

    for (auto [toolNum, diameter] : m_tools) {
        //for (toolIt = m_tools.cbegin(); toolIt != m_tools.cend(); ++toolIt) {
        QString name(tr("Tool Ø%1mm").arg(diameter));
        Row& row = model->appendRow(name, drawDrillIcon(), toolNum); ///->
        bool isSlot = false;
        for (const Excellon::Hole* hole : cacheHoles[toolNum]) {
            if (!hole->state.path.isEmpty())
                isSlot = true;

            auto* item = new DrillPrGI(hole, row);
            m_giPeview[toolNum].append(QSharedPointer<DrillPrGI>(item));
            App::scene()->addItem(item);
        }
        model->setSlot(model->rowCount() - 1, isSlot); ///<-
        if (diameter > 0.0)
            pickUpTool(toolNum, diameter, isSlot);
    }

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    connect(model, &DrillModel::set, header, &Header::set);
    //    header->onCheckedV(header->checked());
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::updateFiles()
{

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    disconnect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#else
    disconnect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#endif

    ui->cbxFile->clear();

    for (Excellon::File* exFile : App::project()->files<Excellon::File>()) {
        ui->cbxFile->addItem(exFile->shortName(), QVariant::fromValue(static_cast<void*>(exFile)));
        ui->cbxFile->setItemIcon(ui->cbxFile->count() - 1, QIcon::fromTheme("drill-path"));
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
    }

    for (Gerber::File* gbrFile : App::project()->files<Gerber::File>()) {
        if (gbrFile->flashedApertures()) {
            ui->cbxFile->addItem(gbrFile->shortName(), QVariant::fromValue(static_cast<void*>(gbrFile)));
            QPixmap pixmap(IconSize, IconSize);
            QColor color(gbrFile->color());
            color.setAlpha(255);
            pixmap.fill(color);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon(pixmap), Qt::DecorationRole);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
        }
    }

    on_cbxFileCurrentIndexChanged(0);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#else
    connect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#endif
}

bool DrillForm::canToShow()
{
    if (App::project()->files<Excellon::File>().size() > 0)
        return true;

    for (Gerber::File* file : App::project()->files<Gerber::File>())
        if (file->flashedApertures())
            return true;

    QMessageBox::information(nullptr, "", tr("No data to process."));
    return false;
}

void DrillForm::on_pbClose_clicked()
{
    if (parent())
        dynamic_cast<QWidget*>(parent())->close();
}

void DrillForm::on_pbCreate_clicked()
{
    { //   slots only
        struct data {
            Paths paths;
            QVector<int> toolsApertures;
        };

        std::map<int, data> pathsMap;

        for (int row = 0; row < model->rowCount(); ++row) {
            int usedToolId = model->toolId(row);
            if (usedToolId != -1 && model->useForCalc(row)) {
                int apertureId = model->apertureId(row);
                pathsMap[usedToolId].toolsApertures.append(apertureId);
                for (QSharedPointer<DrillPrGI>& item : m_giPeview[apertureId]) {
                    if (item->type() == GiSlotPr) {
                        if (item->fit(ui->dsbxDepth->value())) {
                            for (Path& path : offset(item->paths().first(), item->sourceDiameter() - ToolHolder::tool(item->toolId()).diameter())) {
                                pathsMap[usedToolId].paths.append(path);
                            }
                            model->setCreate(row, false);
                        } else {
                            pathsMap[usedToolId].paths.append(item->paths().first());
                            model->setCreate(row, false);
                        }
                    }
                }
                for (QSharedPointer<DrillPrGI>& item : m_giPeview[apertureId]) {
                    if (item->type() != GiSlotPr)
                        model->setCreate(row, true);
                }
            }
        }
        for (auto [usedToolId, _] : pathsMap) {
            QString indexes;
            for (int id : pathsMap[usedToolId].toolsApertures) {
                if (indexes.size())
                    indexes += ",";
                indexes += QString::number(id);
            }
            if (!pathsMap[usedToolId].paths.isEmpty()) {
                GCode::File* gcode = new GCode::File({ pathsMap[usedToolId].paths }, { ToolHolder::tool(usedToolId), ui->dsbxDepth->value(), GCode::Profile });
                gcode->setFileName(ToolHolder::tool(usedToolId).nameEnc() + "_T" + indexes);
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
        }
    }

    { //   other
        struct data {
            Path drillPath;
            Paths paths;
            QVector<int> toolsApertures;
        };
        QMap<int, data> pathsMap;
        for (int row = 0; row < model->rowCount(); ++row) {
            int toolId = model->toolId(row);
            if (toolId != -1 && model->useForCalc(row)) {
                const int apertureId = model->apertureId(row);
                pathsMap[toolId].toolsApertures.append(apertureId);
                for (QSharedPointer<DrillPrGI>& item : m_giPeview[apertureId]) {
                    if (item->type() == GiSlotPr)
                        continue;
                    switch (m_worckType) {
                    case GCode::Profile:
                        if (ToolHolder::tool(toolId).type() != Tool::Drill) {
                            switch (m_side) {
                            case GCode::On:
                            case GCode::Outer:
                                pathsMap[toolId].paths.append(item->paths());
                                model->setCreate(row, false);
                                break;
                            case GCode::Inner:
                                if (item->fit(ui->dsbxDepth->value())) {
                                    pathsMap[toolId].paths.append(item->paths());
                                    model->setCreate(row, false);
                                }
                                break;
                            }
                        }
                        break;
                    case GCode::Pocket:
                        if (ToolHolder::tool(toolId).type() != Tool::Drill && item->fit(ui->dsbxDepth->value())) {
                            pathsMap[toolId].paths.append(item->paths());
                            model->setCreate(row, false);
                        }
                        break;
                    case GCode::Drill:
                        if (ToolHolder::tool(toolId).type() != Tool::Engraver || ToolHolder::tool(toolId).type() != Tool::Laser) {
                            pathsMap[toolId].drillPath.append(item->pos());
                            model->setCreate(row, false);
                        }
                        break;
                    default:;
                    }
                }
            }
        }

        QMap<int, data>::iterator iterator;
        for (iterator = pathsMap.begin(); iterator != pathsMap.end(); ++iterator) {
            const int toolId = iterator.key();

            QString indexes;
            for (int id : pathsMap[toolId].toolsApertures) {
                if (indexes.size())
                    indexes += ",";
                indexes += QString::number(id);
            }

            if (!pathsMap[toolId].drillPath.isEmpty()) {
                Path& path = pathsMap[toolId].drillPath;
                IntPoint point1((Marker::get(Marker::Home)->pos()));
                { // sort by distance
                    int counter = 0;
                    while (counter < path.size()) {
                        int selector = 0;
                        double length = std::numeric_limits<double>::max();
                        for (int i = counter, end = path.size(); i < end; ++i) {
                            double length2 = Length(point1, path[i]);
                            if (length > length2) {
                                length = length2;
                                selector = i;
                            }
                        }
                        qSwap(path[counter], path[selector]);
                        point1 = path[counter++];
                    }
                }
                GCode::File* gcode = new GCode::File({ { path } }, { ToolHolder::tool(toolId), ui->dsbxDepth->value(), GCode::Drill });
                gcode->setFileName(ToolHolder::tool(toolId).nameEnc() + (m_type ? "_T" : "_D") + indexes);
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
            if (!pathsMap[toolId].paths.isEmpty()) {
                Clipper clipper;
                clipper.AddPaths(pathsMap[toolId].paths, ptSubject, true);
                clipper.Execute(ctUnion, pathsMap[toolId].paths, pftPositive);
                ReversePaths(pathsMap[toolId].paths);
                GCode::File* gcode = nullptr;
                switch (m_worckType) {
                case GCode::Profile: {
                    GCode::GCodeParams gcp;
                    gcp.setConvent(ui->rbConventional->isChecked());
                    gcp.setSide(m_side);
                    gcp.tools.append(ToolHolder::tool(toolId));
                    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
                    GCode::ProfileCreator tpc;
                    tpc.addPaths(pathsMap[toolId].paths);
                    tpc.createGc(gcp);
                    gcode = tpc.file();
                } break;
                case GCode::Pocket: {
                    GCode::GCodeParams gcp;
                    gcp.setConvent(ui->rbConventional->isChecked());
                    gcp.setSide(GCode::Inner);
                    gcp.tools.append(ToolHolder::tool(toolId));
                    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
                    gcp.params[GCode::GCodeParams::Pass] = 0;
                    gcp.params[GCode::GCodeParams::UseRaster] = 0;
                    gcp.params[GCode::GCodeParams::Steps] = 0;
                    GCode::PocketCreator tpc;
                    tpc.setGcp(gcp);
                    tpc.addPaths(pathsMap[toolId].paths);
                    tpc.createGc(gcp);
                    gcode = tpc.file();
                } break;
                default:
                    continue;
                }
                if (!gcode)
                    continue;
                gcode->setFileName(ToolHolder::tool(toolId).nameEnc() + "_T" + indexes);
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
        }
    }
    QTimer::singleShot(1, Qt::CoarseTimer, [this] { header->onChecked(); });
}

void DrillForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    file = static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>());
    if (file && file->type() == FileType::Gerber)
        setApertures(static_cast<Gerber::File*>(file)->apertures());
    else
        setExcellonTools(static_cast<Excellon::File*>(file)->tools());
    header->onChecked();
}

void DrillForm::on_clicked(const QModelIndex& index)
{
    int apertureId = model->apertureId(index.row());
    deselectAll();
    setSelected(apertureId, true);
    zoonToSelected();
}

void DrillForm::on_doubleClicked(const QModelIndex& current)
{
    if (current.column() == 1) {
        QVector<Tool::Type> tools;
        tools = model->isSlot(current.row())
            ? QVector<Tool::Type> { Tool::EndMill }
            : ((m_worckType == GCode::Profile || m_worckType == GCode::Pocket)
                    ? QVector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser }
                    : QVector<Tool::Type> { Tool::Drill, Tool::EndMill });
        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id());
            updateToolsOnGi(apertureId);
        }
    }
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
    qDebug(Q_FUNC_INFO);
    deselectAll();
    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(previous.row());
        setSelected(apertureId, false);
    }

    //    if (0) {
    //        for (int row = 0; row < model->rowCount(); ++row) {
    //            int apertureId = model->apertureId(row);
    //            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
    //                item->setSelected(false);
    //            }
    //        }
    //        const QModelIndexList selectedIndexes(ui->toolTable->selectionModel()->selectedIndexes());
    //        for (const QModelIndex& index : selectedIndexes) {
    //            int apertureId = model->apertureId(index.row());
    //            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
    //                item->setSelected(true);
    //            }
    //        }
    //    }

    if (previous.isValid() && previous.row() != current.row()) {
        int apertureId = model->apertureId(current.row());
        setSelected(apertureId, true);
    }
    zoonToSelected();
}

void DrillForm::on_customContextMenuRequested(const QPoint& pos)
{
    if (ui->toolTable->selectionModel()->selectedIndexes().isEmpty())
        return;
    QMenu menu;
    menu.addAction(QIcon::fromTheme("view-form"), tr("&Select Tool"), [this] {
        bool fl = true;
        for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
            fl = model->isSlot(current.row());
            if (!fl)
                break;
        }

        QVector<Tool::Type> tools;
        if (fl)
            tools = QVector<Tool::Type> { Tool::EndMill };
        else
            tools = (m_worckType == GCode::Drill)
                ? QVector<Tool::Type> { Tool::Drill, Tool::EndMill }
                : QVector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser };

        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            const Tool tool(tdb.tool());
            for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
                if (model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id());
                    updateToolsOnGi(model->apertureId(current.row()));
                } else if (model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                    QMessageBox::information(this, "", "\"" + tool.name() + tr("\" not suitable for T") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + "-" + model->data(current.sibling(current.row(), 0)).toString() + "-");
                } else if (!model->isSlot(current.row())) {
                    if (model->toolId(current.row()) > -1 && !model->useForCalc(current.row()))
                        continue;
                    model->setToolId(current.row(), tool.id());
                    updateToolsOnGi(model->apertureId(current.row()));
                }
            }
        }
    });

    for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
        if (model->toolId(current.row()) != -1) {
            menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool"), [this] {
                for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
                    model->setToolId(current.row(), -1);
                    updateToolsOnGi(model->apertureId(current.row()));
                }
                for (int i = 0; i < model->rowCount(); ++i) {
                    if (model->toolId(i) != -1)
                        return;
                }
            });
            break;
        }
    }

    menu.exec(ui->toolTable->mapToGlobal(pos /*+ QPoint(24, 24)*/));
}

void DrillForm::updateToolsOnGi(int toolId)
{
    for (const QSharedPointer<DrillPrGI>& item : m_giPeview[toolId])
        item->updateTool();
}

void DrillForm::pickUpTool(int apertureId, double diameter, bool isSlot)
{
    const double k = 0.05; // 5%
    const double drillDiameterMin = diameter * (1.0 - k);
    const double drillDiameterMax = diameter * (1.0 + k);
    QMap<int, Tool>::const_iterator toolIt;
    for (toolIt = ToolHolder::tools().cbegin(); !isSlot && toolIt != ToolHolder::tools().cend(); ++toolIt) {
        const auto& tool = toolIt.value();
        if (tool.type() == Tool::Drill && drillDiameterMin <= tool.diameter() && drillDiameterMax >= tool.diameter()) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            updateToolsOnGi(apertureId);
            for (QSharedPointer<DrillPrGI>& item : m_giPeview[apertureId])
                item->updateTool();
            return;
        }
    }
    for (toolIt = ToolHolder::tools().cbegin(); toolIt != ToolHolder::tools().cend(); ++toolIt) {
        const auto& tool = toolIt.value();
        if (tool.type() == Tool::EndMill && drillDiameterMin <= tool.diameter() && drillDiameterMax >= tool.diameter()) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            updateToolsOnGi(apertureId);
            for (QSharedPointer<DrillPrGI>& item : m_giPeview[apertureId])
                item->updateTool();
            return;
        }
    }
}

void DrillForm::setSelected(int id, bool fl)
{
    for (QSharedPointer<DrillPrGI>& item : m_giPeview[id])
        item->setSelected(fl);
}

void DrillForm::zoonToSelected()
{
    if (ui->chbxZoomToSelected->isChecked())
        App::graphicsView()->zoomToSelected();
}

void DrillForm::deselectAll()
{
    for (QGraphicsItem* item : App::scene()->selectedItems())
        item->setSelected(false);
}

void DrillForm::clear()
{
    m_giPeview.clear();
}

//////////////////////////////////////////////////////////////////////////
/// \brief Header::Header
/// \param orientation
/// \param parent
///
Header::Header(Qt::Orientation orientation, QWidget* parent)
    : QHeaderView(orientation, parent)

{
    connect(this, &QHeaderView::sectionCountChanged, [this](int /*oldCount*/, int newCount) {
        m_checkRect.resize(newCount);
    });
    setSectionsClickable(true);
    setHighlightSections(true);
}

Header::~Header() { }

void Header::setAll(bool ch)
{
    for (int i = 0; i < count(); ++i) {
        if (checked(i) != ch) {
            setChecked(i, ch);
            updateSection(i);
        }
    }
    emit onChecked();
}

void Header::togle(int index)
{
    setChecked(index, !checked(index));
    updateSection(index);
    emit onChecked(index);
}

void Header::set(int index, bool ch)
{
    setChecked(index, ch);
    updateSection(index);
    emit onChecked(index);
}

QRect Header::getRect(const QRect& rect)
{
    return QRect(
        rect.left() + XOffset,
        rect.top() + (rect.height() - DelegateSize) / 2,
        DelegateSize,
        DelegateSize);
}

void Header::mouseMoveEvent(QMouseEvent* event)
{
    static int index = 0;
    do {

        if (index == logicalIndexAt(event->pos()))
            break;
        index = logicalIndexAt(event->pos());
        if (index < 0)
            break;
        if (event->buttons() != Qt::RightButton)
            break;
        if (orientation() == Qt::Horizontal) {
            //setSingle(index);
        } else
            togle(index);
        event->accept();
        return;
    } while (0);
    QHeaderView::mouseMoveEvent(event);
}

void Header::mousePressEvent(QMouseEvent* event)
{
    int index = logicalIndexAt(event->pos());
    do {
        if (index < 0)
            break;
        if (!m_checkRect[index].contains(event->pos()) && event->buttons() != Qt::RightButton)
            break;
        togle(index);
        event->accept();
        return;
    } while (0);
    QHeaderView::mousePressEvent(event);
}

void Header::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    QStyleOptionButton option;
    m_checkRect[logicalIndex] = option.rect = getRect(rect);

    option.state = checked(logicalIndex)
        ? QStyle::State_On
        : QStyle::State_Off;

    option.state |= model()->toolId(logicalIndex) != -1 && isEnabled()
        ? QStyle::State_Enabled
        : QStyle::State_None;

    if (orientation() == Qt::Horizontal)
        style()->drawPrimitive(QStyle::PE_IndicatorRadioButton, &option, painter);
    else
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
}

void Header::setChecked(int index, bool ch) { model()->setCreate(index, ch); }

bool Header::checked(int index) const { return model()->useForCalc(index); }

DrillModel* Header::model() const { return static_cast<DrillModel*>(static_cast<QTableView*>(parent())->model()); }

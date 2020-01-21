#include "drillform.h"
#include "ui_drillform.h"

#include "../gcodepropertiesform.h"
#include "drillmodel.h"
#include "drillpreviewgi.h"
#include "project.h"
#include "tooldatabase/tooldatabase.h"
#include <QCheckBox>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSettings>
#include <QTimer>
#include <gcpocket.h>
#include <gcprofile.h>
#include <gctypes.h>
#include <graphicsview.h>

DrillForm* DrillForm::self = nullptr;

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

    for (Path& path : tmpPpaths)
        path.append(path.first());

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
    ui->setupUi(this);
    ui->toolTable->setIconSize(QSize(IconSize, IconSize));
    ui->toolTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->toolTable->setWordWrap(false);

    connect(ui->toolTable, &QTableView::customContextMenuRequested, this, &DrillForm::on_customContextMenuRequested);
    connect(ui->toolTable, &QTableView::doubleClicked, this, &DrillForm::on_doubleClicked);
    connect(ui->toolTable, &QTableView::clicked, this, &DrillForm::on_clicked);

    {
        auto cornerButton = ui->toolTable->findChild<QAbstractButton*>();
        header = new Header(Qt::Vertical, ui->toolTable);
        ui->toolTable->setVerticalHeader(header);
        if (cornerButton) {
            checkBox = new QCheckBox(cornerButton);
            checkBox->setFocusPolicy(Qt::NoFocus);
            checkBox->setGeometry(Header::getRect(cornerButton->rect()).translated(1, -4));
            connect(checkBox, &QCheckBox::clicked, [this](bool checked) { header->setAll(checked); });
            connect(header, &Header::onCheckedV, [this](const QVector<bool>& v) {
                static const Qt::CheckState chState[]{
                    Qt::Unchecked,
                    Qt::Unchecked,
                    Qt::Checked,
                    Qt::PartiallyChecked
                };
                checkBox->setCheckState(chState[v.contains(true) * 2 | v.contains(false) * 1]);
                ui->pbCreate->setEnabled(checkBox->checkState() != Qt::Unchecked);
                // updateCreateButton();
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

    QSettings settings;
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
    settings.endGroup();

    connect(ui->rb_drilling, &QRadioButton::clicked, updateState);
    connect(ui->rb_in, &QRadioButton::clicked, updateState);
    connect(ui->rb_on, &QRadioButton::clicked, updateState);
    connect(ui->rb_out, &QRadioButton::clicked, updateState);
    connect(ui->rb_pocket, &QRadioButton::clicked, updateState);
    connect(ui->rb_profile, &QRadioButton::clicked, updateState);

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    updateState();

    updateFiles();

    parent->setWindowTitle(ui->label->text());

    self = this;
}

DrillForm::~DrillForm()
{
    self = nullptr;
    QSettings settings;
    settings.beginGroup("DrillForm");
    settings.setValue("rbClimb", ui->rbClimb->isChecked());
    settings.setValue("rbConventional", ui->rbConventional->isChecked());
    settings.setValue("rb_drilling", ui->rb_drilling->isChecked());
    settings.setValue("rb_in", ui->rb_in->isChecked());
    settings.setValue("rb_on", ui->rb_on->isChecked());
    settings.setValue("rb_out", ui->rb_out->isChecked());
    settings.setValue("rb_pocket", ui->rb_pocket->isChecked());
    settings.setValue("rb_profile", ui->rb_profile->isChecked());
    settings.endGroup();
    clear();
    delete ui;
}

void DrillForm::setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value)
{
    m_type = tAperture;
    clear();
    m_apertures = *value;
    model = new DrillModel(m_type, this);

    QMap<int, QSharedPointer<Gerber::AbstractAperture>>::const_iterator apertureIt;
    for (apertureIt = m_apertures.cbegin(); apertureIt != m_apertures.cend(); ++apertureIt) {
        if (apertureIt.value()->isFlashed()) {
            double drillDiameter = 0.0;
            QString name(apertureIt.value()->name());
            if (apertureIt.value()->isDrilled()) {
                drillDiameter = apertureIt.value()->drillDiameter();
                name += tr(", drill Ø%1mm").arg(drillDiameter);
            } else if (apertureIt.value()->type() == Gerber::Circle) {
                drillDiameter = apertureIt.value()->apertureSize();
            }
            model->appendRow(name, drawApertureIcon(apertureIt.value().data()), apertureIt.key());
            const Gerber::File* file = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>());
            for (const Gerber::GraphicObject& go : *file) {
                if (go.state().dCode() == Gerber::D03 && go.state().aperture() == apertureIt.key()) {
                    DrillPrGI* item = new DrillPrGI(go, apertureIt.key());
                    m_sourcePreview[apertureIt.key()].append(QSharedPointer<DrillPrGI>(item));
                    Scene::addItem(item);
                }
            }
            if (drillDiameter != 0.0)
                pickUpTool(apertureIt.key(), drillDiameter);
        }
    }
    // updateCreateButton();

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    connect(model, &DrillModel::set, header, &Header::set);
    header->onCheckedV(header->checked());
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::setHoles(const QMap<int, double>& value)
{
    m_type = tTool;
    clear();

    m_tools = value;
    model = new DrillModel(m_type, this);

    QMap<int, double>::const_iterator toolIt;
    for (toolIt = m_tools.cbegin(); toolIt != m_tools.cend(); ++toolIt) {
        QString name(tr("Tool Ø%1mm").arg(toolIt.value()));
        model->appendRow(name, drawDrillIcon(), toolIt.key());
        const Excellon::File* file = static_cast<Excellon::File*>(ui->cbxFile->currentData().value<void*>());
        bool isSlot = false;
        for (const Excellon::Hole& hole : *file) {
            if (hole.state.tCode == toolIt.key()) {
                auto* item = new DrillPrGI(hole);
                if (!hole.state.path.isEmpty())
                    isSlot = true;
                m_sourcePreview[toolIt.key()].append(QSharedPointer<DrillPrGI>(item));
                Scene::addItem(item);
            }
        }
        model->setSlot(model->rowCount() - 1, isSlot);
        if (toolIt.value() != 0.0)
            pickUpTool(toolIt.key(), toolIt.value(), isSlot);
    }

    // updateCreateButton();
    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    connect(model, &DrillModel::set, header, &Header::set);
    header->onCheckedV(header->checked());
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::updateFiles()
{
    disconnect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);

    ui->cbxFile->clear();

    for (Excellon::File* file : Project::instance()->files<Excellon::File>()) {
        ui->cbxFile->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
        ui->cbxFile->setItemIcon(ui->cbxFile->count() - 1, QIcon::fromTheme("drill-path"));
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
    }

    for (Gerber::File* file : Project::instance()->files<Gerber::File>()) {
        if (file->flashedApertures()) {
            ui->cbxFile->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
            QPixmap pixmap(IconSize, IconSize);
            QColor color(file->color());
            color.setAlpha(255);
            pixmap.fill(color);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon(pixmap), Qt::DecorationRole);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, IconSize), Qt::SizeHintRole);
        }
    }

    on_cbxFileCurrentIndexChanged(0);

    connect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
}

bool DrillForm::canToShow()
{
    if (Project::instance()->files<Excellon::File>().size() > 0)
        return true;

    for (Gerber::File* file : Project::instance()->files<Gerber::File>())
        if (file->flashedApertures())
            return true;

    QMessageBox::information(nullptr, "", tr("No data to process."));
    return false;
}

void DrillForm::on_pbClose_clicked()
{
    if (parent())
        if (auto* w = dynamic_cast<QWidget*>(parent()); w)
            w->close();
}

void DrillForm::on_pbCreate_clicked()
{
    { //   slots only

        struct data {
            Paths paths;
            QVector<int> toolsApertures;
        };

        QMap<int, data> pathsMap;
        for (int row = 0; row < model->rowCount(); ++row) {
            int selectedToolId = model->toolId(row);
            if (selectedToolId != -1 && model->create(row)) {
                int apertureId = model->apertureId(row);
                pathsMap[selectedToolId].toolsApertures.append(apertureId);
                for (QSharedPointer<DrillPrGI>& item : m_sourcePreview[apertureId]) {
                    if (item->type() == GiSlotPr) {
                        if (item->fit(ui->dsbxDepth->value())) {
                            for (Path& path : offset(item->paths().first(), item->sourceDiameter() - ToolHolder::tools[item->toolId()].diameter())) {
                                pathsMap[selectedToolId].paths.append(path);
                            }
                            model->setCreate(row, false);
                        } else {
                            pathsMap[selectedToolId].paths.append(item->paths().first());
                            model->setCreate(row, false);
                        }
                    }
                }
                for (QSharedPointer<DrillPrGI>& item : m_sourcePreview[apertureId]) {
                    if (item->type() != GiSlotPr)
                        model->setCreate(row, true);
                }
            }
        }
        QMap<int, data>::iterator iterator;
        for (iterator = pathsMap.begin(); iterator != pathsMap.end(); ++iterator) {
            int selectedToolId = iterator.key();
            QString indexes;
            QVector<int>& v = pathsMap[selectedToolId].toolsApertures;
            for (int id : v)
                indexes += QString::number(id) + (id != v.last() ? "," : "");
            if (!pathsMap[selectedToolId].paths.isEmpty()) {
                GCode::File* gcode = new GCode::File({ pathsMap[selectedToolId].paths }, ToolHolder::tools[selectedToolId], ui->dsbxDepth->value(), GCode::Profile);
                gcode->setFileName(/*"Slot Drill " +*/ ToolHolder::tools[selectedToolId].name() + " - T(" + indexes + ')');
                gcode->setSide(file->side());
                Project::instance()->addFile(gcode);
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
            if (toolId != -1 && model->create(row)) {
                const int apertureId = model->apertureId(row);
                pathsMap[toolId].toolsApertures.append(apertureId);
                for (QSharedPointer<DrillPrGI>& item : m_sourcePreview[apertureId]) {
                    if (item->type() == GiSlotPr)
                        continue;
                    switch (m_worckType) {
                    case GCode::Profile:
                        if (ToolHolder::tools[toolId].type() != Tool::Drill) {
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
                        if (ToolHolder::tools[toolId].type() != Tool::Drill && item->fit(ui->dsbxDepth->value())) {
                            pathsMap[toolId].paths.append(item->paths());
                            model->setCreate(row, false);
                        }
                        break;
                    case GCode::Drill:
                        if (ToolHolder::tools[toolId].type() != Tool::Engraving || ToolHolder::tools[toolId].type() != Tool::Laser) {
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
            QVector<int>& v = pathsMap[toolId].toolsApertures;
            for (int id : v)
                indexes += QString::number(id) + (id != v.last() ? "," : "");
            if (!pathsMap[toolId].drillPath.isEmpty()) {
                Path& path = pathsMap[toolId].drillPath;
                IntPoint point1(toIntPoint(Marker::get(Marker::Home)->pos()));
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
                GCode::File* gcode = new GCode::File({ { path } }, ToolHolder::tools[toolId], ui->dsbxDepth->value(), GCode::Drill);
                gcode->setFileName(/*"Drill " +*/ ToolHolder::tools[toolId].name() + (m_type ? " - T(" : " - D(") + indexes + ')');
                gcode->setSide(file->side());
                Project::instance()->addFile(gcode);
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
                    gcp.convent = ui->rbConventional->isChecked();
                    gcp.side = m_side;
                    gcp.tool.append(ToolHolder::tools[toolId]);
                    gcp.dParam[GCode::Depth] = ui->dsbxDepth->value();
                    GCode::ProfileCreator tpc;
                    tpc.addPaths(pathsMap[toolId].paths);
                    tpc.createGc(gcp);
                    gcode = tpc.file();
                } break;
                case GCode::Pocket: {
                    GCode::GCodeParams gcp;
                    gcp.convent = ui->rbConventional->isChecked();
                    gcp.side = GCode::Inner;
                    gcp.tool.append(ToolHolder::tools[toolId]);
                    gcp.dParam[GCode::Depth] = ui->dsbxDepth->value();
                    gcp.dParam[GCode::Pass] = 0;
                    gcp.dParam[GCode::UseRaster] = 0;
                    gcp.dParam[GCode::Steps] = 0;
                    gcp.dParam[GCode::TwoTools] = 0;
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
                gcode->setFileName(/*"Slot Drill " +*/ ToolHolder::tools[toolId].name() + " - T(" + indexes + ')');
                gcode->setSide(file->side());
                Project::instance()->addFile(gcode);
            }
        }
    }
    header->onCheckedV(header->checked());
    // updateCreateButton();
    QTimer::singleShot(500, Qt::CoarseTimer, [this] { ui->pbCreate->update(); });
}

void DrillForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    file = static_cast<AbstractFile*>(ui->cbxFile->currentData().value<void*>());
    if (file && file->type() == FileType::Gerber)
        setApertures(static_cast<Gerber::File*>(file)->apertures());
    else
        setHoles(static_cast<Excellon::File*>(file)->tools());
}

void DrillForm::on_clicked(const QModelIndex& index)
{
    int apertureId = model->apertureId(index.row());
    deselectAll();
    setSelected(apertureId, true);
    zoonToSelected();
    // updateCreateButton();
}

void DrillForm::on_doubleClicked(const QModelIndex& current)
{
    if (current.column() == 1) {
        QVector<Tool::Type> tools;
        tools = model->isSlot(current.row())
            ? QVector<Tool::Type>{ Tool::EndMill }
            : ((m_worckType == GCode::Profile || m_worckType == GCode::Pocket)
                      ? QVector<Tool::Type>{ Tool::Drill, Tool::EndMill, Tool::Engraving, Tool::Laser }
                      : QVector<Tool::Type>{ Tool::Drill, Tool::EndMill });
        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id());
            createHoles(apertureId, tool.id());
            // updateCreateButton();
        }
    }
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& previous)
{
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
    // updateCreateButton();
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
            tools = QVector<Tool::Type>{ Tool::EndMill };
        else
            tools = (m_worckType == GCode::Drill)
                ? QVector<Tool::Type>{ Tool::Drill, Tool::EndMill }
                : QVector<Tool::Type>{ Tool::Drill, Tool::EndMill, Tool::Engraving, Tool::Laser };

        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            const Tool tool(tdb.tool());
            for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
                if (model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id());
                    createHoles(model->apertureId(current.row()), tool.id());
                } else if (model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                    QMessageBox::information(this, "", "\"" + tool.name() + tr("\" not suitable for T") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + "(" + model->data(current.sibling(current.row(), 0)).toString() + ")");
                } else if (!model->isSlot(current.row())) {
                    if (model->toolId(current.row()) > -1 && !model->create(current.row()))
                        continue;
                    model->setToolId(current.row(), tool.id());
                    createHoles(model->apertureId(current.row()), tool.id());
                }
            }
            // updateCreateButton();
        }
    });

    for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
        if (model->toolId(current.row()) != -1) {
            menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool"), [this] {
                for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
                    model->setToolId(current.row(), -1);
                    createHoles(model->apertureId(current.row()), -1);
                }
                for (int i = 0; i < model->rowCount(); ++i) {
                    if (model->toolId(i) != -1)
                        return;
                }
                // updateCreateButton();
            });
            break;
        }
    }

    menu.exec(ui->toolTable->mapToGlobal(pos /*+ QPoint(24, 24)*/));
}

void DrillForm::createHoles(int toolId, int toolIdSelected)
{
    for (const QSharedPointer<DrillPrGI>& item : m_sourcePreview[toolId])
        item->setToolId(toolIdSelected);
}

void DrillForm::pickUpTool(int apertureId, double diameter, bool isSlot)
{
    const double k = 0.05; // 5%
    const double drillDiameterMin = diameter * (1.0 - k);
    const double drillDiameterMax = diameter * (1.0 + k);
    QMap<int, Tool>::const_iterator toolIt;
    for (toolIt = ToolHolder::tools.cbegin(); !isSlot && toolIt != ToolHolder::tools.cend(); ++toolIt) {
        if (toolIt.value().type() == Tool::Drill && drillDiameterMin <= toolIt.value().diameter() && drillDiameterMax >= toolIt.value().diameter()) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().id());
            for (QSharedPointer<DrillPrGI>& item : m_sourcePreview[apertureId]) {
                item->setToolId(toolIt.value().id());
            }
            return;
        }
    }
    for (toolIt = ToolHolder::tools.cbegin(); toolIt != ToolHolder::tools.cend(); ++toolIt) {
        if (toolIt.value().type() == Tool::EndMill && drillDiameterMin <= toolIt.value().diameter() && drillDiameterMax >= toolIt.value().diameter()) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().id());
            for (QSharedPointer<DrillPrGI>& item : m_sourcePreview[apertureId]) {
                item->setToolId(toolIt.value().id());
            }
            return;
        }
    }
}

//void DrillForm::updateCreateButton()
//{
//    for (int row = 0; row < model->rowCount(); ++row) {
//        if (model->create(row)) {
//            ui->pbCreate->setEnabled(true);
//            return;
//        }
//    }
//    checkBox->setChecked(false);
//    header->setAll(false);
//    ui->pbCreate->setEnabled(false);
//}

void DrillForm::setSelected(int id, bool fl)
{
    for (QSharedPointer<DrillPrGI>& item : m_sourcePreview[id])
        item->setSelected(fl);
}

void DrillForm::zoonToSelected()
{
    if (ui->chbxZoomToSelected->isChecked())
        GraphicsView::zoomToSelected();
}

void DrillForm::deselectAll()
{
    for (QGraphicsItem* item : Scene::selectedItems())
        item->setSelected(false);
}

void DrillForm::clear()
{
    m_sourcePreview.clear();
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
        m_checked.resize(newCount);
        m_checkRect.resize(newCount);
        emit onCheckedV(m_checked);
    });
    setSectionsClickable(true);
    setHighlightSections(true);
}

Header::~Header() {}

void Header::setAll(bool ch)
{
    for (int i = 0; i < count(); ++i) {
        if (checked(i) != ch) {
            setChecked(i, ch);
            emit onChecked(i);
            updateSection(i);
        }
    }
    emit onCheckedV(m_checked);
}

void Header::togle(int index)
{
    setChecked(index, !checked(index));
    updateSection(index);
    emit onCheckedV(m_checked);
    emit onChecked(index);
}

void Header::set(int index, bool ch)
{
    setChecked(index, ch);
    updateSection(index);
    emit onCheckedV(m_checked);
    emit onChecked(index);
}

QVector<bool> Header::checked()
{
    for (int index = 0; index < m_checked.size(); ++index) {
        m_checked[index] = static_cast<DrillModel*>(static_cast<QTableView*>(parent())->model())->create(index);
        updateSection(index);
    }
    return m_checked;
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

    option.state |= static_cast<DrillModel*>(static_cast<QTableView*>(parent())->model())->toolId(logicalIndex) != -1 && isEnabled()
        ? QStyle::State_Enabled
        : QStyle::State_None;

    if (orientation() == Qt::Horizontal)
        style()->drawPrimitive(QStyle::PE_IndicatorRadioButton, &option, painter);
    else
        style()->drawPrimitive(QStyle::PE_IndicatorCheckBox, &option, painter);
}

void Header::setChecked(int index, bool ch)
{
    static_cast<DrillModel*>(static_cast<QTableView*>(parent())->model())->setCreate(index, ch);
    m_checked[index] = static_cast<DrillModel*>(static_cast<QTableView*>(parent())->model())->create(index);
}

bool Header::checked(int index) const
{
    return m_checked[index] = static_cast<DrillModel*>(static_cast<QTableView*>(parent())->model())->create(index);
}

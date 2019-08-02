#include "drillform.h"
#include "ui_drillform.h"

#include "drillmodel.h"
#include "gcodepropertiesform.h"
#include "previewitem.h"
#include "project.h"
#include "tooldatabase/tooldatabase.h"
#include <QCheckBox>
#include <QMenu>
#include <QMouseEvent>
#include <QPainter>
#include <QSettings>
#include <QTimer>
#include <graphicsview.h>

DrillForm* DrillForm::self = nullptr;

enum { Size = 24 };
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

    qreal scale = static_cast<double>(Size / qMax(rect.width(), rect.height()));

    double ky = -rect.top() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (Size - rect.height() * scale) / 2;
    else
        kx -= (Size - rect.width() * scale) / 2;

    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, scale);
    painter.drawPath(painterPath);
    return QIcon(pixmap);
}

QIcon drawDrillIcon()
{
    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    painter.drawEllipse(QRect(0, 0, Size - 1, Size - 1));
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
    ui->toolTable->setIconSize(QSize(Size, Size));
    ui->dsbxDepth->setValue(GCodePropertiesForm::thickness);

    ui->toolTable->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->toolTable, &QTableView::customContextMenuRequested, this, &DrillForm::on_customContextMenuRequested);
    connect(ui->toolTable, &QTableView::doubleClicked, this, &DrillForm::on_doubleClicked);
    connect(ui->toolTable, &QTableView::clicked, this, &DrillForm::on_clicked);
    //    MyHeader* header = new MyHeader(Qt::Horizontal /*, ui->toolTable*/);
    //    ui->toolTable->setHorizontalHeader(header);

    lay = new QGridLayout(ui->toolTable->horizontalHeader());
    cbx = new QCheckBox("", ui->toolTable);
    lay->addWidget(cbx, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    lay->setContentsMargins(3, 0, 0, 0);
    cbx->setMinimumHeight(ui->toolTable->horizontalHeader()->height() - 4);
    connect(cbx, &QCheckBox::toggled, [this](bool checked) {
        model->setCreate(checked);
        updateCreateButton();
    });

    ui->toolTable->setWordWrap(false);
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

    ui->dsbxDepth->setValue(settings.value("dsbxDepth").toDouble());
    if (settings.value("rbBoard").toBool())
        ui->dsbxDepth->rbBoard->setChecked(true);
    if (settings.value("rbCopper").toBool())
        ui->dsbxDepth->rbCopper->setChecked(true);

    settings.endGroup();

    connect(ui->rb_drilling, &QRadioButton::clicked, updateState);
    connect(ui->rb_in, &QRadioButton::clicked, updateState);
    connect(ui->rb_on, &QRadioButton::clicked, updateState);
    connect(ui->rb_out, &QRadioButton::clicked, updateState);
    connect(ui->rb_pocket, &QRadioButton::clicked, updateState);
    connect(ui->rb_profile, &QRadioButton::clicked, updateState);

    ui->pbClose->setIcon(Icon(ButtonCloseIcon));
    ui->pbCreate->setIcon(Icon(ButtonCreateIcon));
    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }
    updateState();

    updateFiles();
    self = this;
    parent->setWindowTitle(ui->label->text());
}

DrillForm::~DrillForm()
{
    self = nullptr;

    qDebug("~DrillForm()");

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

    settings.setValue("dsbxDepth", ui->dsbxDepth->value(true));
    settings.setValue("rbBoard", ui->dsbxDepth->rbBoard->isChecked());
    settings.setValue("rbCopper", ui->dsbxDepth->rbCopper->isChecked());
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
                    PreviewItem* item = new PreviewItem(go, apertureIt.key());
                    m_sourcePreview[apertureIt.key()].append(QSharedPointer<PreviewItem>(item));
                    Scene::addItem(item);
                }
            }
            if (drillDiameter != 0.0)
                pickUpTool(apertureIt.key(), drillDiameter);
        }
    }
    updateCreateButton();

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
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
                auto* item = new PreviewItem(hole);
                if (!hole.state.path.isEmpty())
                    isSlot = true;
                m_sourcePreview[toolIt.key()].append(QSharedPointer<PreviewItem>(item));
                Scene::addItem(item);
            }
        }
        model->setSlot(model->rowCount() - 1, isSlot);
        if (toolIt.value() != 0.0)
            pickUpTool(toolIt.key(), toolIt.value(), isSlot);
    }
    updateCreateButton();

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);
}

void DrillForm::updateFiles()
{
    disconnect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);

    ui->cbxFile->clear();

    for (Excellon::File* file : Project::files<Excellon::File>()) {
        ui->cbxFile->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, Icon(PathDrillIcon), Qt::DecorationRole);
        ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
    }

    for (Gerber::File* file : Project::files<Gerber::File>()) {
        if (file->flashedApertures()) {
            ui->cbxFile->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
            QPixmap pixmap(Size, Size);
            QColor color(file->color());
            color.setAlpha(255);
            pixmap.fill(color);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon(pixmap), Qt::DecorationRole);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
        }
    }

    if (!ui->cbxFile->count()) {
        QMessageBox::information(this, "", tr("No data to process."));
        QTimer::singleShot(1, Qt::CoarseTimer, [this] { on_pbClose_clicked(); });
    } else
        on_cbxFileCurrentIndexChanged(0);
    connect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
}

void DrillForm::on_pbClose_clicked()
{
    if (parent())
        if (auto* w = dynamic_cast<QWidget*>(parent()); w)
            w->close();
}

void DrillForm::on_pbCreate_clicked()
{
    //    { //   slots only

    //        struct data {
    //            Paths paths;
    //            QVector<int> toolsApertures;
    //        };

    //        QMap<int, data> pathsMap;
    //        for (int row = 0; row < model->rowCount(); ++row) {
    //            int selectedToolId = model->toolId(row);
    //            if (selectedToolId != -1 && model->create(row)) {
    //                int apertureId = model->apertureId(row);
    //                pathsMap[selectedToolId].toolsApertures.append(apertureId);
    //                for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
    //                    if (item->type() == PreviewItemType::SlotType) {
    //                        if (item->fit(ui->dsbxDepth->value())) {
    //                            for (Path& path : offset(item->paths().first(), item->sourceDiameter() - ToolHolder::tools[item->toolId()].diameter())) {
    //                                pathsMap[selectedToolId].paths.append(path);
    //                            }
    //                            model->setCreate(row, false);
    //                        } else {
    //                            pathsMap[selectedToolId].paths.append(item->paths().first());
    //                            model->setCreate(row, false);
    //                        }
    //                    }
    //                }
    //                for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
    //                    if (item->type() != PreviewItemType::SlotType)
    //                        model->setCreate(row, true);
    //                }
    //            }
    //        }
    //        QMap<int, data>::iterator iterator;
    //        for (iterator = pathsMap.begin(); iterator != pathsMap.end(); ++iterator) {
    //            int selectedToolId = iterator.key();
    //            QString indexes;
    //            QVector<int>& v = pathsMap[selectedToolId].toolsApertures;
    //            for (int id : v)
    //                indexes += QString::number(id) + (id != v.last() ? "," : "");
    //            if (!pathsMap[selectedToolId].paths.isEmpty()) {
    //                GCode::File* gcode = new GCode::File(pathsMap[selectedToolId].paths, ToolHolder::tools[selectedToolId], ui->dsbxDepth->value(), GCode::Profile);
    //                gcode->setFileName(/*"Slot Drill " +*/ ToolHolder::tools[selectedToolId].name() + " - T(" + indexes + ')');
    //                gcode->setSide(file->side());
    //                Project::addFile(gcode);
    //            }
    //        }
    //    }

    //    { //   other
    //        struct data {
    //            Path drillPath;
    //            Paths paths;
    //            QVector<int> toolsApertures;
    //        };
    //        QMap<int, data> pathsMap;
    //        for (int row = 0; row < model->rowCount(); ++row) {
    //            int toolId = model->toolId(row);
    //            if (toolId != -1 && model->create(row)) {
    //                const int apertureId = model->apertureId(row);
    //                pathsMap[toolId].toolsApertures.append(apertureId);
    //                for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
    //                    if (item->type() == PreviewItemType::SlotType)
    //                        continue;
    //                    switch (m_worckType) {
    //                    case GCode::Profile:
    //                        if (ToolHolder::tools[toolId].type() != Tool::Drill) {
    //                            switch (m_side) {
    //                            case GCode::On:
    //                            case GCode::Outer:
    //                                pathsMap[toolId].paths.append(item->paths());
    //                                model->setCreate(row, false);
    //                                break;
    //                            case GCode::Inner:
    //                                if (item->fit(ui->dsbxDepth->value())) {
    //                                    pathsMap[toolId].paths.append(item->paths());
    //                                    model->setCreate(row, false);
    //                                }
    //                                break;
    //                            }
    //                        }
    //                        break;
    //                    case GCode::Pocket:
    //                        if (ToolHolder::tools[toolId].type() != Tool::Drill && item->fit(ui->dsbxDepth->value())) {
    //                            pathsMap[toolId].paths.append(item->paths());
    //                            model->setCreate(row, false);
    //                        }
    //                        break;
    //                    case GCode::Drill:
    //                        if (ToolHolder::tools[toolId].type() != Tool::Engraving) {
    //                            pathsMap[toolId].drillPath.append(item->pos());
    //                            model->setCreate(row, false);
    //                        }
    //                        break;
    //                    default:;
    //                    }
    //                }
    //            }
    //        }

    //        QMap<int, data>::iterator iterator;
    //        for (iterator = pathsMap.begin(); iterator != pathsMap.end(); ++iterator) {
    //            const int toolId = iterator.key();
    //            QString indexes;
    //            QVector<int>& v = pathsMap[toolId].toolsApertures;
    //            for (int id : v)
    //                indexes += QString::number(id) + (id != v.last() ? "," : "");
    //            if (!pathsMap[toolId].drillPath.isEmpty()) {
    //                Path& path = pathsMap[toolId].drillPath;
    //                IntPoint point1(toIntPoint(GCodePropertiesForm::homePoint->pos()));
    //                int counter = 0;
    //                { // sort by distance
    //                    while (counter < path.size()) {
    //                        int selector = 0;
    //                        double length = std::numeric_limits<double>::max();
    //                        for (int i = counter, end = path.size(); i < end; ++i) {
    //                            double length2 = Length(point1, path[i]);
    //                            if (length > length2) {
    //                                length = length2;
    //                                selector = i;
    //                            }
    //                        }
    //                        qSwap(path[counter], path[selector]);
    //                        point1 = path[counter++];
    //                    }
    //                }
    //                GCode::File* gcode = new GCode::File({ path }, ToolHolder::tools[toolId], ui->dsbxDepth->value(), GCode::Drill);
    //                gcode->setFileName(/*"Drill " +*/ ToolHolder::tools[toolId].name() + (m_type ? " - T(" : " - D(") + indexes + ')');
    //                gcode->setSide(file->side());
    //                Project::addFile(gcode);
    //            }
    //            if (!pathsMap[toolId].paths.isEmpty()) {
    //                Clipper clipper;
    //                clipper.AddPaths(pathsMap[toolId].paths, ptSubject, true);
    //                clipper.Execute(ctUnion, pathsMap[toolId].paths, pftPositive);
    //                ReversePaths(pathsMap[toolId].paths);
    //                GCode::File* gcode = nullptr;
    //                switch (m_worckType) {
    //                case GCode::Profile: {
    //                    GCode::Creator tpc(pathsMap[toolId].paths, ui->rbConventional->isChecked(), m_side);
    //                    tpc.createProfile(ToolHolder::tools[toolId], ui->dsbxDepth->value());
    //                    gcode = tpc.file();
    //                } break;
    //                case GCode::Pocket: {
    //                    GCode::Creator tpc(pathsMap[toolId].paths, ui->rbConventional->isChecked(), GCode::Inner);
    //                    tpc.createPocket(ToolHolder::tools[toolId], ui->dsbxDepth->value(), 0);
    //                    gcode = tpc.file();
    //                } break;
    //                default:
    //                    continue;
    //                }
    //                if (!gcode)
    //                    continue;
    //                gcode->setFileName(/*"Slot Drill " +*/ ToolHolder::tools[toolId].name() + " - T(" + indexes + ')');
    //                gcode->setSide(file->side());
    //                Project::addFile(gcode);
    //            }
    //        }
    //    }

    //    updateCreateButton();
    //    QTimer::singleShot(100, Qt::CoarseTimer, [this] { ui->pbCreate->update(); });
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
    updateCreateButton();
}

void DrillForm::on_doubleClicked(const QModelIndex& current)
{
    if (current.column() == 1) {
        QVector<Tool::Type> tools;
        tools = model->isSlot(current.row())
            ? QVector<Tool::Type>{ Tool::EndMill }
            : ((m_worckType == GCode::Profile || m_worckType == GCode::Pocket)
                      ? QVector<Tool::Type>{ Tool::Drill, Tool::EndMill, Tool::Engraving }
                      : QVector<Tool::Type>{ Tool::Drill, Tool::EndMill });
        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id());
            createHoles(apertureId, tool.id());
            updateCreateButton();
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
    updateCreateButton();
}

void DrillForm::on_customContextMenuRequested(const QPoint& pos)
{
    if (ui->toolTable->selectionModel()->selectedIndexes().isEmpty())
        return;
    QMenu menu;
    menu.addAction(Icon(ToolDatabaseIcon), tr("&Select Tool"), [this] {
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
            tools = (m_worckType == GCode::Profile || m_worckType == GCode::Pocket)
                ? QVector<Tool::Type>{ Tool::Drill, Tool::EndMill, Tool::Engraving }
                : QVector<Tool::Type>{ Tool::Drill, Tool::EndMill };

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
            updateCreateButton();
        }
    });

    for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
        if (model->toolId(current.row()) != -1) {
            menu.addAction(Icon(RemoveIcon), tr("&Remove Tool"), [this] {
                for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
                    model->setToolId(current.row(), -1);
                    createHoles(model->apertureId(current.row()), -1);
                }
                for (int i = 0; i < model->rowCount(); ++i) {
                    if (model->toolId(i) != -1)
                        return;
                }
                updateCreateButton();
            });
            break;
        }
    }

    menu.exec(ui->toolTable->mapToGlobal(pos /*+ QPoint(24, 24)*/));
}

void DrillForm::createHoles(int toolId, int toolIdSelected)
{
    for (const QSharedPointer<PreviewItem>& item : m_sourcePreview[toolId])
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
            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                item->setToolId(toolIt.value().id());
            }
            return;
        }
    }
    for (toolIt = ToolHolder::tools.cbegin(); toolIt != ToolHolder::tools.cend(); ++toolIt) {
        if (toolIt.value().type() == Tool::EndMill && drillDiameterMin <= toolIt.value().diameter() && drillDiameterMax >= toolIt.value().diameter()) {
            model->setToolId(model->rowCount() - 1, toolIt.key());
            createHoles(apertureId, toolIt.value().id());
            for (QSharedPointer<PreviewItem>& item : m_sourcePreview[apertureId]) {
                item->setToolId(toolIt.value().id());
            }
            return;
        }
    }
}

void DrillForm::updateCreateButton()
{
    for (int row = 0; row < model->rowCount(); ++row) {
        if (model->create(row)) {
            ui->pbCreate->setEnabled(true);
            return;
        }
    }
    cbx->setChecked(false);
    ui->pbCreate->setEnabled(false);
}

void DrillForm::setSelected(int id, bool fl)
{
    for (QSharedPointer<PreviewItem>& item : m_sourcePreview[id])
        item->setSelected(fl);
}

void DrillForm::zoonToSelected()
{
    if (ui->chbxZoomToSelected->isChecked())
        GraphicsView::self->zoomToSelected();
}

void DrillForm::deselectAll()
{
    for (QGraphicsItem* item : GraphicsView::self->scene()->selectedItems())
        item->setSelected(false);
}

void DrillForm::clear()
{
    m_sourcePreview.clear();
}

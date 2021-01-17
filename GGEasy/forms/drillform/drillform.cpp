// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "drillform.h"
#include "ui_drillform.h"

#include "graphicsview.h"
#include "scene.h"
#include "settings.h"

#include "drillmodel.h"
#include "drillpreviewgi.h"
#include "point.h"
#include "project.h"
#include "tooldatabase.h"

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
        tmpPath.push_back(tmpPath.front());

    return tmpPpaths;
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
                if (model) {
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
                }
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
        settings.getValue(ui->rbConventional, true);
        settings.getValue(ui->rb_drilling, true);
        settings.getValue(ui->rb_in, true);
        settings.getValue(ui->rb_on);
        settings.getValue(ui->rb_out);
        settings.getValue(ui->rb_pocket);
        settings.getValue(ui->rb_profile);
        settings.getValue(ui->rbClimb);
        settings.getValue(ui->chbxZoomToSelected);
        settings.endGroup();
    }

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

    App::setDrillForm(this);
}

DrillForm::~DrillForm()
{
    App::setDrillForm(nullptr);

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

void DrillForm::updateFiles()
{

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    disconnect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#else
    disconnect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#endif

    ui->cbxFile->clear();
    for (auto file : App::project()->files({ FileType::Gerber, FileType::Excellon }))
        App::fileInterface(int(file->type()))->addToDrillForm(file, ui->cbxFile);

    on_cbxFileCurrentIndexChanged(0);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#else
    connect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#endif
}

bool DrillForm::canToShow()
{
    if (App::project()->files(FileType::Excellon).size() > 0)
        return true;

    QComboBox cbx;
    for (auto file : App::project()->files(FileType::Gerber)) {
        App::fileInterface(int(file->type()))->addToDrillForm(file, &cbx);
        if (cbx.count())
            return true;
    }

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
            mvector<int> toolsApertures;
        };

        std::map<int, data> pathsMap;

        for (int row = 0; row < model->rowCount(); ++row) {
            int usedToolId = model->toolId(row);
            if (usedToolId != -1 && model->useForCalc(row)) {
                int apertureId = model->apertureId(row);
                pathsMap[usedToolId].toolsApertures.push_back(apertureId);
                for (auto& item : m_giPeview[apertureId]) {
                    if (static_cast<GiType>(item->type()) == GiType::PrSlot) {
                        if (item->fit(ui->dsbxDepth->value())) {
                            for (Path& path : offset(item->paths().front(), item->sourceDiameter() - App::toolHolder().tool(item->toolId()).diameter())) {
                                pathsMap[usedToolId].paths.push_back(path);
                            }
                            model->setCreate(row, false);
                        } else {
                            pathsMap[usedToolId].paths.push_back(item->paths().front());
                            model->setCreate(row, false);
                        }
                    }
                }
                for (auto& item : m_giPeview[apertureId]) {
                    if (static_cast<GiType>(item->type()) != GiType::PrSlot)
                        model->setCreate(row, true);
                }
            }
        }
        for (auto [usedToolId, _] : pathsMap) {
            (void)_;
            QString indexes;
            for (int id : pathsMap[usedToolId].toolsApertures) {
                if (indexes.size())
                    indexes += ",";
                indexes += QString::number(id);
            }
            if (!pathsMap[usedToolId].paths.empty()) {
                GCode::File* gcode = new GCode::File({ pathsMap[usedToolId].paths }, { App::toolHolder().tool(usedToolId), ui->dsbxDepth->value(), GCode::Profile });
                gcode->setFileName(App::toolHolder().tool(usedToolId).nameEnc() + "_T" + indexes);
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
        }
    }

    { //   other
        struct data {
            Path drillPath;
            Paths paths;
            mvector<int> toolsApertures;
        };
        QMap<int, data> pathsMap;
        for (int row = 0; row < model->rowCount(); ++row) {
            int toolId = model->toolId(row);
            if (toolId != -1 && model->useForCalc(row)) {
                const int apertureId = model->apertureId(row);
                pathsMap[toolId].toolsApertures.push_back(apertureId);
                for (auto& item : m_giPeview[apertureId]) {
                    if (static_cast<GiType>(item->type()) == GiType::PrSlot)
                        continue;
                    switch (m_worckType) {
                    case GCode::Profile:
                        if (App::toolHolder().tool(toolId).type() != Tool::Drill) {
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
                        if (App::toolHolder().tool(toolId).type() != Tool::Drill && item->fit(ui->dsbxDepth->value())) {
                            pathsMap[toolId].paths.append(item->paths());
                            model->setCreate(row, false);
                        }
                        break;
                    case GCode::Drill:
                        if (App::toolHolder().tool(toolId).type() != Tool::Engraver || App::toolHolder().tool(toolId).type() != Tool::Laser) {
                            pathsMap[toolId].drillPath.push_back(item->pos());
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

            if (!pathsMap[toolId].drillPath.empty()) {
                Path& path = pathsMap[toolId].drillPath;
                Point64 point1((Marker::get(Marker::Home)->pos()));
                { // sort by distance
                    size_t counter = 0;
                    while (counter < path.size()) {
                        size_t selector = 0;
                        double length = std::numeric_limits<double>::max();
                        for (size_t i = counter, end = path.size(); i < end; ++i) {
                            double length2 = point1.distTo(path[i]);
                            if (length > length2) {
                                length = length2;
                                selector = i;
                            }
                        }
                        qSwap(path[counter], path[selector]);
                        point1 = path[counter++];
                    }
                }
                GCode::File* gcode = new GCode::File({ { path } }, { App::toolHolder().tool(toolId), ui->dsbxDepth->value(), GCode::Drill });
                gcode->setFileName(App::toolHolder().tool(toolId).nameEnc() + (m_type ? "_T" : "_D") + indexes);
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
            if (!pathsMap[toolId].paths.empty()) {
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
                    gcp.tools.push_back(App::toolHolder().tool(toolId));
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
                    gcp.tools.push_back(App::toolHolder().tool(toolId));
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
                gcode->setFileName(App::toolHolder().tool(toolId).nameEnc() + "_T" + indexes);
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
        }
    }
    QTimer::singleShot(1, Qt::CoarseTimer, [this] { header->onChecked(); });
}

void DrillForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    clear();
    file = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>());

    switch (file->type()) {
    case FileType::Gerber:
        m_type = tAperture;
        break;
    case FileType::Excellon:
    default:
        m_type = tTool;
    }

    model = new DrillModel(this);
    m_giPeview = App::fileInterface(int(file->type()))->createDrillPreviewGi(file, model->data());

    for (auto& [key, val] : m_giPeview)
        for (auto& spGi : val)
            App::scene()->addItem(spGi.get());

    pickUpTool();

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    connect(model, &DrillModel::set, header, &Header::set);
    //    header->onCheckedV(header->checked());
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::currentChanged, this, &DrillForm::on_currentChanged);

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
        mvector<Tool::Type> tools;
        tools = model->isSlot(current.row())
            ? mvector<Tool::Type> { Tool::EndMill }
            : ((m_worckType == GCode::Profile || m_worckType == GCode::Pocket)
                    ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser }
                    : mvector<Tool::Type> { Tool::Drill, Tool::EndMill });
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

        mvector<Tool::Type> tools;
        if (fl)
            tools = mvector<Tool::Type> { Tool::EndMill };
        else
            tools = (m_worckType == GCode::Drill)
                ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill }
                : mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser };

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

void DrillForm::updateToolsOnGi(int apToolId)
{
    qDebug() << __FUNCTION__ << apToolId;
    for (auto& item : m_giPeview[apToolId])
        item->updateTool();
}

void DrillForm::pickUpTool(const double k)
{
    int ctr = 0;
    for (const auto& [ //
             icon,
             name,
             diameter,
             apertureId,
             isSlot,
             useForCalc,
             toolId] : model->data()) {
        const double drillDiameterMin = diameter * (1.0 - k);
        const double drillDiameterMax = diameter * (1.0 + k);
        if (!isSlot)
            for (auto& [id, tool] : App::toolHolder().tools()) {
                if (tool.type() == Tool::Drill
                    && drillDiameterMin <= tool.diameter()
                    && drillDiameterMax >= tool.diameter()) {
                    model->setToolId(ctr, id);
                    updateToolsOnGi(apertureId);
                    break;
                }
            }
        if (/*row.*/ toolId < 0)
            for (auto& [id, tool] : App::toolHolder().tools()) {
                if (tool.type() == Tool::EndMill
                    && drillDiameterMin <= tool.diameter()
                    && drillDiameterMax >= tool.diameter()) {
                    model->setToolId(ctr, id);
                    updateToolsOnGi(apertureId);
                    break;
                }
            }
        ++ctr;
    }
}

void DrillForm::setSelected(int id, bool fl)
{
    for (auto& item : m_giPeview[id])
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

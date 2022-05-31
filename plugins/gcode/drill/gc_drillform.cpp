// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#include "gc_drillform.h"
#include "gc_drillmodel.h"
#include "pocketoffset/gc_pocketoffset.h"
#include "profile/gc_profile.h"
#include "ui_drillform.h"

#include "graphicsview.h"
#include "scene.h"
#include "settings.h"

#include "drillpreviewgi.h"
#include "gc_drillmodel.h"
#include "point.h"
#include "project.h"
#include "tool_pch.h"

#include <QMessageBox>
#include <QPainter>
#include <QTimer>

Paths offset(const Path& path, double offset, bool fl = false) {
    ClipperOffset cpOffset;
    cpOffset.AddPath(path, jtRound, fl ? etClosedLine : etOpenRound);
    Paths tmpPpaths;
    cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
    for (Path& tmpPath : tmpPpaths)
        tmpPath.push_back(tmpPath.front());
    return tmpPpaths;
}

class DrillPrGI final : public AbstractDrillPrGI {
    HV& hv;
    Row& row;

public:
    explicit DrillPrGI(HV& hv, double diameter, int toolId, Row& row)
        : AbstractDrillPrGI(toolId)
        , hv { hv }
        , row { row } {
        sourceDiameter_ = diameter;
        sourcePath_ = hv.index() ? drawDrill() : drawSlot();
        type_ = hv.index() ? GiType::PrDrill : GiType::PrSlot;
        update();
    }

    //    explicit DrillPrGI(const Excellon::Hole* hole, Row& row)
    //        : AbstractDrillPrGI(row)
    //        , hole(hole) {
    //        m_sourceDiameter = hole->state.currentToolDiameter();
    //        m_sourcePath = hole->state.path.isEmpty() ? drawDrill() : drawSlot();
    //        m_type = hole->state.path.isEmpty() ? GiType::PrDrill : GiType::PrSlot;
    //    }

    // AbstractDrillPrGI interface
    void updateTool() override {
        if (toolId_ > -1) {
            colorState |= Tool;
            if (type_ == GiType::PrSlot) {
                toolPath_ = {};

                auto& tool(App::toolHolder().tool(toolId_));
                const double diameter = tool.getDiameter(tool.getDepth());
                const double lineKoeff = diameter * 0.7;

                Paths tmpPpath;

                ClipperOffset offset;
                offset.AddPath(*std::get<const QPolygonF*>(hv), jtRound, etOpenRound);
                offset.Execute(tmpPpath, diameter * 0.5 * uScale);

                for (Path& path : tmpPpath) {
                    path.push_back(path.front());
                    toolPath_.addPolygon(path);
                }

                Path path(*std::get<const QPolygonF*>(hv));

                if (path.size()) {
                    for (IntPoint& pt : path) {
                        toolPath_.moveTo(pt - QPointF(0.0, lineKoeff));
                        toolPath_.lineTo(pt + QPointF(0.0, lineKoeff));
                        toolPath_.moveTo(pt - QPointF(lineKoeff, 0.0));
                        toolPath_.lineTo(pt + QPointF(lineKoeff, 0.0));
                    }
                    toolPath_.moveTo(path.front());
                    for (IntPoint& pt : path) {
                        toolPath_.lineTo(pt);
                    }
                }
            }
        } else {
            colorState &= ~Tool;
            toolPath_ = {};
        }
        changeColor();
    }

    IntPoint pos() const override { return hv.index() ? std::get<const QPointF>(hv) : QPointF {} /*hole->state.offsetedPos()*/; }

    Paths paths() const override {
        if (hv.index())
            return {};

        if (type_ == GiType::PrSlot)
            return { *std::get<const QPolygonF*>(hv) };

        Paths paths({ *std::get<const QPolygonF*>(hv) });
        return ReversePaths(paths);
    }

    bool fit(double depth) override { return sourceDiameter_ > App::toolHolder().tool(toolId()).getDiameter(depth); }

    // AbstractDrillPrGI interface
    int toolId() const override { return toolId_ < 0 ? row.toolId : toolId_; }

private:
    QPainterPath drawDrill() const {
        QPainterPath painterPath;
        const double radius = /*hole->state.currentToolDiameter()*/ sourceDiameter_ * 0.5;
        painterPath.addEllipse(std::get<const QPointF>(hv), radius, radius);
        return painterPath;
    }

    QPainterPath drawSlot() const {
        QPainterPath painterPath;

        //        std::ranges::for_each(offset(/*hole->item->paths().front()*/ *std::get<const QPolygonF*>(hv)),
        //            //            painterPath, &QPainterPath::addPolygon);
        //            [&painterPath](auto&& path) {
        //                painterPath.addPolygon(path);
        //            });
        for (auto&& path : offset(/*hole->item->paths().front()*/ *std::get<const QPolygonF*>(hv), /*hole->state.currentToolDiameter()*/ sourceDiameter_))
            painterPath.addPolygon(path);
        return painterPath;
    }

    Paths offset(const Path& path, double offset) const {
        ClipperOffset cpOffset;
        // cpOffset.AddPath(path, jtRound, etClosedLine);
        cpOffset.AddPath(path, jtRound, etOpenRound);
        Paths tmpPpaths;
        cpOffset.Execute(tmpPpaths, offset * 0.5 * uScale);
        for (Path& path : tmpPpaths)
            path.push_back(path.front());
        return tmpPpaths;
    }

    //    const Hole* hole;
};

/////////////////////////////////////////////
/// \brief DrillForm::DrillForm
/// \param parent
///
DrillForm::DrillForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DrillForm) {
    ui->setupUi(this);
    {
        ui->toolTable->setIconSize(QSize(IconSize, IconSize));
        ui->toolTable->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->toolTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->toolTable->setWordWrap(false);
        ui->toolTable->horizontalHeader()->setMinimumHeight(ui->toolTable->verticalHeader()->defaultSectionSize());

        connect(ui->toolTable, &QTableView::customContextMenuRequested, this, &DrillForm::on_customContextMenuRequested);
        connect(ui->toolTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, [this](const QPoint& pos) {
            QMenu menu;
            menu.addAction(QIcon::fromTheme("view-form"), tr("&Choose a Tool for everyone"), [this] {
                ui->toolTable->selectAll();
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
                    tools = (worckType_ == GCode::Drill) ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill } : mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser };
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
            menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool for everyone"), [this] {
                ui->toolTable->selectAll();
                for (QModelIndex current : ui->toolTable->selectionModel()->selectedIndexes()) {
                    model->setToolId(current.row(), -1);
                    updateToolsOnGi(model->apertureId(current.row()));
                }
                for (int i = 0; i < model->rowCount(); ++i) {
                    if (model->toolId(i) != -1)
                        return;
                }
            });
            menu.exec(ui->toolTable->horizontalHeader()->mapToGlobal(pos));
        });
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

                        if (idx < 0 || idx == i) {
                            for (auto& item : giPeview_[model->apertureId(i)])
                                item->changeColor();
                        }
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
        worckType_ = ui->rb_drilling->isChecked() ? GCode::Drill : (ui->rb_profile->isChecked() ? GCode::Profile : GCode::Pocket);
        ui->grbxSide->setEnabled(worckType_ == GCode::Profile);
        ui->grbxDirection->setEnabled(worckType_ == GCode::Profile || worckType_ == GCode::Pocket);
        side_ = ui->rb_on->isChecked() ? GCode::On : (ui->rb_out->isChecked() ? GCode::Outer : GCode::Inner);
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

    connect(ui->pbPickUpTools, &QPushButton::clicked, this, &DrillForm::pickUpTool);

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    updateState();

    updateFiles();

    setWindowTitle(ui->label->text());

    App::setDrillForm(this);
}

DrillForm::~DrillForm() {
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

void DrillForm::updateFiles() {

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    disconnect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#else
    disconnect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#endif

    ui->cbxFile->clear();
    for (auto file : App::project()->files({ FileType::Excellon, FileType::Gerber, FileType::Dxf }))
        App::filePlugin(int(file->type()))->addToDrillForm(file, ui->cbxFile);

    on_cbxFileCurrentIndexChanged(0);

#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#else
    connect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &DrillForm::on_cbxFileCurrentIndexChanged);
#endif
}

bool DrillForm::canToShow() {
    if (App::project()->files(FileType::Excellon).size() > 0)
        return true;

    QComboBox cbx;
    for (auto type : { FileType::Gerber, FileType::Dxf }) {
        for (auto file : App::project()->files(type)) {
            App::filePlugin(int(file->type()))->addToDrillForm(file, &cbx);
            if (cbx.count())
                return true;
        }
    }

    QMessageBox::information(nullptr, "", tr("No data to process."));
    return false;
}

void DrillForm::on_pbClose_clicked() {
    if (parent())
        dynamic_cast<QWidget*>(parent())->close();
}

void DrillForm::on_pbCreate_clicked() {
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
                for (auto& item : giPeview_[apertureId]) {
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
                for (auto& item : giPeview_[apertureId]) {
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
                for (auto& item : giPeview_[apertureId]) {
                    if (static_cast<GiType>(item->type()) == GiType::PrSlot)
                        continue;
                    switch (worckType_) {
                    case GCode::Profile:
                        if (App::toolHolder().tool(toolId).type() != Tool::Drill) {
                            switch (side_) {
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
                IntPoint point1((App::home()->pos()));
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
                gcode->setFileName(App::toolHolder().tool(toolId).nameEnc() + (type_ ? "_T" : "_D") + indexes);
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
            if (!pathsMap[toolId].paths.empty()) {
                Clipper clipper;
                clipper.AddPaths(pathsMap[toolId].paths, ptSubject, true);
                clipper.Execute(ctUnion, pathsMap[toolId].paths, pftPositive);
                ReversePaths(pathsMap[toolId].paths);
                GCode::File* gcode = nullptr;
                switch (worckType_) {
                case GCode::Profile: {
                    // FIXME GCode                   GCode::GCodeParams gcp;
                    //                    gcp.setConvent(ui->rbConventional->isChecked());
                    //                    gcp.setSide(m_side);
                    //                    gcp.tools.push_back(App::toolHolder().tool(toolId));
                    //                    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
                    //                    GCode::ProfileCreator tpc;
                    //                    tpc.setGcp(gcp);
                    //                    tpc.addPaths(pathsMap[toolId].paths);
                    //                    tpc.createGc();
                    //                    gcode = tpc.file();
                } break;
                case GCode::Pocket: {
                    // FIXME GCode                  GCode::GCodeParams gcp;
                    //                    gcp.setConvent(ui->rbConventional->isChecked());
                    //                    gcp.setSide(GCode::Inner);
                    //                    gcp.tools.push_back(App::toolHolder().tool(toolId));
                    //                    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
                    //                    gcp.params[GCode::GCodeParams::Pass] = 0;
                    //                    gcp.params[GCode::GCodeParams::UseRaster] = 0;
                    //                    gcp.params[GCode::GCodeParams::Steps] = 0;
                    //                    GCode::PocketCreator tpc;
                    //                    tpc.setGcp(gcp);
                    //                    tpc.addPaths(pathsMap[toolId].paths);
                    //                    tpc.createGc();
                    //                    gcode = tpc.file();
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

void DrillForm::on_cbxFileCurrentIndexChanged(int /*index*/) {
    clear();
    file = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>());
    if (!file)
        return;
    switch (file->type()) {
    case FileType::Gerber:
    case FileType::Dxf:
        type_ = tAperture;
        break;
    case FileType::Excellon:
    default:
        type_ = tTool;
    }

    try {
        peview_ = std::any_cast<HoleMap>(App::filePlugin(int(file->type()))->createPreviewGi(file, /*model->data()*/ nullptr));

        model = new DrillModel(1, peview_.size(), ui->toolTable);
        auto& data = model->data();
        data.reserve(peview_.size());
        for (auto& [key, val] : peview_) {
            auto [id, diametr, isSlot] = key;
            data.emplace_back(QString::number(diametr), drawDrillIcon(isSlot ? Qt::red : Qt::black), id, diametr);
            qDebug() << id << diametr << isSlot;
            for (auto& val : val) {
                auto gi = new DrillPrGI(val, diametr, data.back().toolId, data.back());
                giPeview_[id].emplace_back(gi);
                App::scene()->addItem(gi);
            }
            //                        giPeview_ = App::filePlugin(int(file->type()))->createDrillPreviewGi(file, model->data());
        }
        //        giPeview_ = App::filePlugin(int(file->type()))->createDrillPreviewGi(file, model->data());

        //    for (auto& [key, val] : giPeview_)
        //        for (auto& spGi : val)
        //            App::scene()->addItem(spGi.get());
        App::scene()->update();
    } catch (...) {
        delete ui->toolTable->model();
        return;
    }

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

void DrillForm::on_clicked(const QModelIndex& index) {
    int apertureId = model->apertureId(index.row());
    deselectAll();
    setSelected(apertureId, true);
    zoonToSelected();
}

void DrillForm::on_doubleClicked(const QModelIndex& current) {
    if (current.column() == 1) {
        mvector<Tool::Type> tools;
        tools = model->isSlot(current.row()) ? mvector<Tool::Type> { Tool::EndMill } : ((worckType_ == GCode::Profile || worckType_ == GCode::Pocket) ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser } : mvector<Tool::Type> { Tool::Drill, Tool::EndMill });
        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id());
            updateToolsOnGi(apertureId);
        }
    }
}

void DrillForm::on_currentChanged(const QModelIndex& current, const QModelIndex& previous) {
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

void DrillForm::on_customContextMenuRequested(const QPoint& pos) {
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
            tools = (worckType_ == GCode::Drill) ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill } : mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser };

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

    menu.exec(ui->toolTable->viewport()->mapToGlobal(pos));
}

void DrillForm::updateToolsOnGi(int apToolId) {
    qDebug() << apToolId;
    for (auto& item : giPeview_[apToolId])
        item->updateTool();
}

void DrillForm::pickUpTool() {
    if (!model)
        return;
    const double k = 0.05;
    int ctr = 0;
    for (const auto& row : model->data()) {
        const double drillDiameterMin = row.diameter * (1.0 - k);
        const double drillDiameterMax = row.diameter * (1.0 + k);
        if (!row.isSlot)
            for (auto& [id, tool] : App::toolHolder().tools()) {
                if (tool.type() == Tool::Drill
                    && drillDiameterMin <= tool.diameter()
                    && drillDiameterMax >= tool.diameter()) {
                    model->setToolId(ctr, id);
                    updateToolsOnGi(row.apertureId);
                    break;
                }
            }
        if (row.toolId < 0)
            for (auto& [id, tool] : App::toolHolder().tools()) {
                if (tool.type() == Tool::EndMill
                    && drillDiameterMin <= tool.diameter()
                    && drillDiameterMax >= tool.diameter()) {
                    model->setToolId(ctr, id);
                    updateToolsOnGi(row.apertureId);
                    break;
                }
            }
        ++ctr;
    }
}

void DrillForm::setSelected(int id, bool fl) {
    for (auto& item : giPeview_[id])
        item->setSelected(fl);
}

void DrillForm::zoonToSelected() {
    if (ui->chbxZoomToSelected->isChecked())
        App::graphicsView()->zoomToSelected();
}

void DrillForm::deselectAll() {
    for (QGraphicsItem* item : App::scene()->selectedItems())
        item->setSelected(false);
}

void DrillForm::clear() {
    giPeview_.clear();
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

void Header::setAll(bool ch) {
    for (int i = 0; i < count(); ++i) {
        if (checked(i) != ch) {
            setChecked(i, ch);
            updateSection(i);
        }
    }
    emit onChecked();
}

void Header::togle(int index) {
    setChecked(index, !checked(index));
    updateSection(index);
    emit onChecked(index);
}

void Header::set(int index, bool ch) {
    setChecked(index, ch);
    updateSection(index);
    emit onChecked(index);
}

QRect Header::getRect(const QRect& rect) {
    return QRect(
        rect.left() + XOffset,
        rect.top() + (rect.height() - DelegateSize) / 2,
        DelegateSize,
        DelegateSize);
}

void Header::mouseMoveEvent(QMouseEvent* event) {
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
            // setSingle(index);
        } else
            togle(index);
        event->accept();
        return;
    } while (0);
    QHeaderView::mouseMoveEvent(event);
}

void Header::mousePressEvent(QMouseEvent* event) {
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

void Header::paintSection(QPainter* painter, const QRect& rect, int logicalIndex) const {
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();

    QStyleOptionButton option;
    m_checkRect[logicalIndex] = option.rect = getRect(rect);

    option.state = checked(logicalIndex) ? QStyle::State_On : QStyle::State_Off;
    option.state |= model()->toolId(logicalIndex) != -1 && isEnabled() ? QStyle::State_Enabled : QStyle::State_None;

    style()->drawPrimitive(orientation() == Qt::Horizontal ? QStyle::PE_IndicatorRadioButton : QStyle::PE_IndicatorCheckBox, &option, painter);
}

void Header::setChecked(int index, bool ch) { model()->setCreate(index, ch); }

bool Header::checked(int index) const { return model()->useForCalc(index); }

DrillModel* Header::model() const { return static_cast<DrillModel*>(static_cast<QTableView*>(parent())->model()); }

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
 *******************************************************************************/
#include "drill_form.h"
#include "drill_header.h"
#include "drill_model.h"
#include "ui_drillform.h"

#include "gc_errordialog.h"
#include "gc_gidrillpreview.h"
#include "gi_point.h"
#include "gi_preview.h"
#include "graphicsview.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include "tool_pch.h"

#include "pocketoffset/pocketoffset.h"
#include "profile/profile.h"

#include <QMessageBox>
#include <QPainter>
#include <QThread>
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

/////////////////////////////////////////////
/// \brief DrillForm::DrillForm
/// \param parent
///
DrillForm::DrillForm(GCodePlugin* plugin, QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::DrillForm)
    , plugin { plugin } {
    ui->setupUi(this);

    {
        ui->toolTable->setIconSize(QSize(IconSize, IconSize));
        ui->toolTable->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->toolTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
        ui->toolTable->setWordWrap(false);
        ui->toolTable->horizontalHeader()->setMinimumHeight(ui->toolTable->verticalHeader()->defaultSectionSize());

        connect(ui->toolTable, &QTableView::customContextMenuRequested, this, &DrillForm::on_customContextMenuRequested);

        auto menu = [this](const QPoint& pos) {
            QMenu menu;
            menu.addAction(QIcon::fromTheme("view-form"), tr("&Choose a Tool for everyone"), [this] {
                ui->toolTable->selectAll();
                bool fl = true;
                for (QModelIndex current : selectedIndexes()) {
                    fl = model->isSlot(current.row());
                    if (!fl)
                        break;
                }
                mvector<Tool::Type> tools;
                if (fl)
                    tools = mvector<Tool::Type> { Tool::EndMill };
                else
                    tools = (worckType == GCode::Drill) ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill } : mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser };
                ToolDatabase tdb(this, tools);
                if (tdb.exec()) {
                    const Tool tool(tdb.tool());
                    for (QModelIndex current : selectedIndexes()) {
                        if (model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                            model->setToolId(current.row(), tool.id());
                        } else if (model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                            QMessageBox::information(this, "", "\"" + tool.name() + tr("\" not suitable for T") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + "-" + model->data(current.sibling(current.row(), 0)).toString() + "-");
                        } else if (!model->isSlot(current.row())) {
                            if (model->toolId(current.row()) > -1 && !model->useForCalc(current.row()))
                                continue;
                            model->setToolId(current.row(), tool.id());
                        }
                    }
                }
            });
            menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool for everyone"), [this] {
                ui->toolTable->selectAll();
                for (QModelIndex current : selectedIndexes()) {
                    model->setToolId(current.row(), -1);
                }
                for (int i = 0; i < model->rowCount(); ++i) {
                    if (model->toolId(i) != -1)
                        return;
                }
            });
            menu.exec(ui->toolTable->horizontalHeader()->mapToGlobal(pos));
        };

        connect(ui->toolTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, menu);
        connect(ui->toolTable, &QTableView::doubleClicked, this, &DrillForm::on_doubleClicked);
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
                    int fl {};
                    for (int i {}; i < model->rowCount(); ++i)
                        if (model->useForCalc(i))
                            ++fl;

                    checkBox->setCheckState(!fl ? Qt::Unchecked : (fl == model->rowCount() ? Qt::Checked : Qt::PartiallyChecked));
                    ui->pbCreate->setEnabled(fl);
                }
            });
        }
    }
    ui->pbCreate->setEnabled(false);
    ui->rb_drilling->setChecked(true);

    auto updateState = [this] {
        worckType = ui->rb_drilling->isChecked() ? GCode::Drill : (ui->rb_profile->isChecked() ? GCode::Profile : GCode::Pocket);
        ui->grbxSide->setEnabled(worckType == GCode::Profile);
        ui->grbxDirection->setEnabled(worckType == GCode::Profile || worckType == GCode::Pocket);
        side = ui->rb_on->isChecked() ? GCode::On : (ui->rb_out->isChecked() ? GCode::Outer : GCode::Inner);
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

    connect(ui->pbClose, &QPushButton::clicked, this, &QObject::deleteLater);

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

    auto indexes = [](const auto& range) {
        QString indexes;
        for (int id : range) {
            if (indexes.size())
                indexes += ",";
            indexes += QString::number(id);
        }
        return indexes;
    };

    if (worckType == GCode::Drill) { //   slots only
        struct Data {
            Paths paths;
            mvector<int> toolsApertures;
        };

        std::map<int, Data> pathsMap;

        for (int i {}; auto&& row : *model) {
            if (row.toolId > -1 && row.useForCalc && row.isSlot) {
                pathsMap[row.toolId].toolsApertures.push_back(row.apertureId);
                for (auto& item : row.items) {
                    if (item->fit(ui->dsbxDepth->value()))
                        for (Path& path : offset(item->paths().front(), item->sourceDiameter() - App::toolHolder().tool(item->toolId()).diameter()))
                            pathsMap[row.toolId].paths.push_back(path);
                    else
                        pathsMap[row.toolId].paths.push_back(item->paths().front());
                }
            }
            model->setCreate(i++, !pathsMap.contains(row.toolId));
        }

        for (auto [usedToolId, _] : pathsMap) {
            (void)_;
            if (pathsMap[usedToolId].paths.size()) {
                GCode::File* gcode = new GCode::File({ pathsMap[usedToolId].paths }, { App::toolHolder().tool(usedToolId), ui->dsbxDepth->value(), GCode::Profile });
                gcode->setFileName(App::toolHolder().tool(usedToolId).nameEnc() + "_T" + indexes(pathsMap[usedToolId].toolsApertures));
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
        }
    }

    { //   other
        struct Data {
            Path drillPath;
            Paths paths;
            mvector<int> toolsApertures;
        };

        std::map<int, Data> pathsMap;

        auto itemToPaths = [](auto* item) {
            return item->isSlot() ? item->offset() : item->paths();
        };

        for (int i {}; auto&& row : *model) {
            bool created {};
            if (row.toolId > -1 && row.useForCalc) {
                pathsMap[row.toolId].toolsApertures.push_back(row.apertureId);
                for (auto& item : row.items) {
                    //                    if (item->isSlot())
                    //                        continue;
                    switch (worckType) {
                    case GCode::Profile:
                        if (App::toolHolder().tool(row.toolId).type() != Tool::Drill) {
                            switch (side) {
                            case GCode::On:
                            case GCode::Outer:
                                pathsMap[row.toolId].paths.append(itemToPaths(item));
                                created = true;
                                break;
                            case GCode::Inner:
                                //                                if (item->fit(ui->dsbxDepth->value())) {
                                pathsMap[row.toolId].paths.append(itemToPaths(item));
                                created = true;
                                //                                }
                                break;
                            }
                        }
                        break;
                    case GCode::Pocket:
                        if (App::toolHolder().tool(row.toolId).type() != Tool::Drill) {
                            //                        if (App::toolHolder().tool(row.toolId).type() != Tool::Drill && item->fit(ui->dsbxDepth->value())) {
                            pathsMap[row.toolId].paths.append(itemToPaths(item));
                            created = true;
                            //   }
                        }
                        break;
                    case GCode::Drill:
                        if (App::toolHolder().tool(row.toolId).type() != Tool::Engraver || App::toolHolder().tool(row.toolId).type() != Tool::Laser) {
                            pathsMap[row.toolId].drillPath.push_back(item->pos());
                            created = true;
                        }
                        break;
                    default:;
                    }
                }
            }
            if (created)
                model->setCreate(i, false);
            ++i;
        }

        for (auto [toolId, val] : pathsMap) {
            if (val.drillPath.size()) {
                IntPoint point1((App::home()->pos()));
                { // sort by distance
                    size_t counter = 0;
                    while (counter < val.drillPath.size()) {
                        size_t selector = 0;
                        double length = std::numeric_limits<double>::max();
                        for (size_t i = counter, end = val.drillPath.size(); i < end; ++i) {
                            double length2 = point1.distTo(val.drillPath[i]);
                            if (length > length2) {
                                length = length2;
                                selector = i;
                            }
                        }
                        qSwap(val.drillPath[counter], val.drillPath[selector]);
                        point1 = val.drillPath[counter++];
                    }
                }
                GCode::File* gcode = new GCode::File({ { val.drillPath } }, { App::toolHolder().tool(toolId), ui->dsbxDepth->value(), GCode::Drill });
                gcode->setFileName(App::toolHolder().tool(toolId).nameEnc() + type_ + indexes(val.toolsApertures));
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
            if (val.paths.size()) {
                GCode::File* gcode = nullptr;

                QThread thread;
                switch (worckType) {
                case GCode::Profile: {
                    GCode::GCodeParams gcp;
                    gcp.setConvent(ui->rbConventional->isChecked());
                    gcp.setSide(side);
                    gcp.tools = { App::toolHolder().tool(toolId) };
                    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();

                    GCode::ProfileCreator tpc;
                    tpc.moveToThread(&thread);
                    connect(&tpc, &GCode::Creator::errorOccurred, this, &DrillForm::errorOccurred);
                    thread.start();
                    tpc.setGcp(gcp);
                    tpc.addPaths(val.paths);
                    tpc.createGc();

                    gcode = tpc.file();
                } break;
                case GCode::Pocket: {
                    GCode::GCodeParams gcp;
                    gcp.setConvent(ui->rbConventional->isChecked());
                    gcp.setSide(GCode::Inner);
                    gcp.tools = { App::toolHolder().tool(toolId) };
                    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
                    gcp.params[GCode::GCodeParams::Pass] = 0;
                    gcp.params[GCode::GCodeParams::UseRaster] = 0;
                    gcp.params[GCode::GCodeParams::Steps] = 0;

                    GCode::PocketCreator tpc;
                    tpc.moveToThread(&thread);
                    connect(&tpc, &GCode::Creator::errorOccurred, this, &DrillForm::errorOccurred);
                    thread.start();
                    tpc.setGcp(gcp);
                    tpc.addPaths(val.paths);
                    tpc.createGc();
                    gcode = tpc.file();
                } break;
                default:
                    continue;
                }

                thread.quit();
                thread.wait();

                if (!gcode)
                    continue;
                gcode->setFileName(App::toolHolder().tool(toolId).nameEnc() + "_T" + indexes(val.toolsApertures));
                gcode->setSide(file->side());
                App::project()->addFile(gcode);
            }
        }
    }

    QTimer::singleShot(1, Qt::CoarseTimer, [this] { header->onChecked(); });
}

void DrillForm::on_cbxFileCurrentIndexChanged(int /*index*/) {
    file = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>());
    if (!file)
        return;
    switch (file->type()) {
    case FileType::Gerber:
        type_ = "_D";
        break;
    case FileType::Dxf:
        type_ = "_DXF";
        break;
    case FileType::Excellon:
    default:
        type_ = "_T";
    }

    try {
        auto peview = std::any_cast<Drills>(App::filePlugin(int(file->type()))->createPreviewGi(file, plugin));

        model = new DrillModel(type_, peview.size(), ui->toolTable);

        auto& data = model->data();
        data.reserve(peview.size());
        for (auto& [key, val] : peview) {
            auto [id, diametr, isSlot, name] = key;
            data.emplace_back(
                val.draw.size() ? drawIcon(val.draw) : drawDrillIcon(isSlot ? Qt::red : Qt::black),
                name + ": Ø" + QString::number(diametr),
                diametr,
                id,
                isSlot);
            for (auto&& posOrPath : val.posOrPath)
                auto gi = new GiDrillPreview(std::move(posOrPath), diametr, data.back().toolId, data.back(), val.draw);
        }

        App::scene()->update();
    } catch (const std::exception& exc) {
        qDebug("%s: %s", __FUNCTION__, exc.what());
    } catch (...) {
        exit(-99);
        // delete ui->toolTable->model();
        return;
    }

    pickUpTool();

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    connect(model, &DrillModel::set, header, &Header::set);
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &DrillForm::on_selectionChanged);

    header->onChecked();
}

void DrillForm::on_doubleClicked(const QModelIndex& current) {
    if (current.column() == 1) {
        mvector<Tool::Type> tools;
        tools = model->isSlot(current.row()) ? mvector<Tool::Type> { Tool::EndMill } : ((worckType == GCode::Profile || worckType == GCode::Pocket) ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser } : mvector<Tool::Type> { Tool::Drill, Tool::EndMill });
        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            int apertureId = model->apertureId(current.row());
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id());
        }
    }
}

void DrillForm::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    for (auto&& index : selected.indexes())
        for (auto&& item : model->data()[index.row()].items)
            item->setSelected(true);
    for (auto&& index : deselected.indexes())
        for (auto&& item : model->data()[index.row()].items)
            item->setSelected(false);
    zoomToSelected();
}

void DrillForm::on_customContextMenuRequested(const QPoint& pos) {

    if (selectedIndexes().isEmpty())
        return;
    QMenu menu;
    menu.addAction(QIcon::fromTheme("view-form"), tr("&Select Tool"), [this] {
        bool flag = true;
        for (QModelIndex current : selectedIndexes()) {
            flag = model->isSlot(current.row());
            if (!flag)
                break;
        }

        mvector<Tool::Type> tools;
        if (flag)
            tools = mvector<Tool::Type> { Tool::EndMill };
        else
            tools = (worckType == GCode::Drill) ? mvector<Tool::Type> { Tool::Drill, Tool::EndMill } : mvector<Tool::Type> { Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser };

        ToolDatabase tdb(this, tools);
        if (tdb.exec()) {
            const Tool tool(tdb.tool());
            for (QModelIndex current : selectedIndexes()) {
                if (model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id());
                } else if (model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                    QMessageBox::information(this, "", "\"" + tool.name() + tr("\" not suitable for T") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + "-" + model->data(current.sibling(current.row(), 0)).toString() + "-");
                } else if (!model->isSlot(current.row())) {
                    if (model->toolId(current.row()) > -1 && !model->useForCalc(current.row()))
                        continue;
                    model->setToolId(current.row(), tool.id());
                }
            }
        }
    });

    for (QModelIndex current : selectedIndexes()) {
        if (model->toolId(current.row()) != -1) {
            menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool"), [this] {
                for (QModelIndex current : selectedIndexes())
                    model->setToolId(current.row(), -1);
            });
            break;
        }
    }

    menu.exec(ui->toolTable->viewport()->mapToGlobal(pos));
}

void DrillForm::pickUpTool() {
    if (!model)
        return;
    const double k = 0.05; // 5%
    int ctr = 0;
    for (const auto& row : model->data()) {
        //        model->setToolId(ctr++, 3);
        //        continue;
        const double drillDiameterMin = row.diameter * (1.0 - k);
        const double drillDiameterMax = row.diameter * (1.0 + k);

        auto isFit = [&, this](auto& tool) -> bool {
            const auto diameter = tool.getDiameter(ui->dsbxDepth->value());
            return drillDiameterMin <= diameter && drillDiameterMax >= diameter;
        };

        for (auto& [id, tool] : App::toolHolder().tools()) {
            if (!row.isSlot && (tool.type() == Tool::Drill || tool.type() == Tool::EndMill) && isFit(tool)) {
                model->setToolId(ctr, id);
                break;
            }
            if (row.isSlot && tool.type() == Tool::EndMill && isFit(tool)) {
                model->setToolId(ctr, id);
                break;
            }
        }
        ++ctr;
    }
}

void DrillForm::errorOccurred() {
   
    auto tpc = (GCode::Creator*)sender();
    tpc->continueCalc(ErrorDialog(std::move(tpc->items), this).exec());
}

QModelIndexList DrillForm::selectedIndexes() const { return ui->toolTable->selectionModel()->selectedIndexes(); }

void DrillForm::zoomToSelected() {
    if (ui->chbxZoomToSelected->isChecked())
        App::graphicsView()->zoomToSelected();
}

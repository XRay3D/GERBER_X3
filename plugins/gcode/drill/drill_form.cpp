// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "drill_form.h"
#include "gi_point.h"
#include "tool_database.h"
#include "ui_drillform.h"

#include "drill_gi_preview.h"
#include "drill_header.h"
#include "drill_model.h"

// #include "file.h"
// #include "gi_point.h"
// #include "gi_preview.h"
#include "graphicsview.h"
// #include "project.h"
// #include "settings.h"
// #include "tool_pch.h"

#include "pocketoffset/pocketoffset.h"
#include "profile/profile.h"

#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QWidget>
#include <set>

namespace Drilling {

Paths offset(const Path& path, double offset, bool fl = false) {
    ClipperOffset cpOffset;
    cpOffset.AddPath(path, JoinType::Round, fl ? EndType::Round : EndType::Round);
    Paths tmpPpaths = cpOffset.Execute(offset * uScale);
    for(Path& tmpPath: tmpPpaths)
        tmpPath.push_back(tmpPath.front());
    return tmpPpaths;
}

/////////////////////////////////////////////

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, nullptr, parent)
    , ui(new Ui::DrillForm) {
    ui->setupUi(content);

    grid->setRowStretch(2, 1);
    grid->setRowStretch(7, 0);

    initToolTable();

    pbCreate->setEnabled(false);
    ui->rb_drilling->setChecked(true);

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

    //
    connect(ui->pbPickUpTools, &QPushButton::clicked, this, &Form::pickUpTool);
    connect(ui->rb_drilling, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rb_in, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rb_on, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rb_out, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rb_pocket, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rb_profile, &QRadioButton::clicked, this, &Form::updateState);

    updateButtonIconSize();
    updateState();
    updateFiles();
    setWindowTitle(tr("GCType::Drill Toolpath"));
    App::setDrillForm(this);
}

Form::~Form() {
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

    // for (auto* var : App::grView().items<Gi::DataSolid>())
    // delete var;

    delete ui;
}

using GrTy = GraphicObject::Type;
Criteria criterias[]{
    {
        std::vector{GraphicObject::FlStamp} //
    },
};

void Form::updateFiles() {
    disconnect(ui->cbxFile, &QComboBox::currentIndexChanged, this, &Form::on_cbxFileCurrentIndexChanged);
    ui->cbxFile->clear();

    for(auto file: App::project().files()) {
        auto gos = file->getDataForGC(criterias, GCType::Drill, true);
        if(gos.size())
            ui->cbxFile->addItem(file->icon(), file->shortName(), QVariant::fromValue(file));
    }

    if(ui->cbxFile->count())
        on_cbxFileCurrentIndexChanged();

    connect(ui->cbxFile, &QComboBox::currentIndexChanged, this, &Form::on_cbxFileCurrentIndexChanged);
}

bool Form::canToShow() {
    // if (App::project().files(FileType::Excellon).size() > 0)
    return true;

    // QComboBox cbx;
    // for (auto type : {FileType::Gerber_, FileType::Dxf_}) {
    // for (auto file : App::project().files(type)) {
    // App::filePlugin(int(file->type()))->addToGcForm(file, &cbx);
    // if (cbx.count())
    // return true;
    // }
    // }
    // QMessageBox::information(nullptr, "", tr("No data to process."));
    // return false;
}

void Form::initToolTable() {
    ui->toolTable->setIconSize(QSize(IconSize, IconSize));
    ui->toolTable->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->toolTable->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->toolTable->setWordWrap(false);
    ui->toolTable->horizontalHeader()->setMinimumHeight(ui->toolTable->verticalHeader()->defaultSectionSize());

    connect(ui->toolTable, &QTableView::customContextMenuRequested, this, &Form::on_customContextMenuRequested);

    connect(ui->toolTable->horizontalHeader(), &QHeaderView::customContextMenuRequested, this, &Form::customContextMenuRequested);
    connect(ui->toolTable, &QTableView::doubleClicked, this, &Form::on_doubleClicked);

    auto cornerButton = ui->toolTable->findChild<QAbstractButton*>();
    if(!cornerButton)
        exit(-67);
    header = new Header{Qt::Vertical, ui->toolTable};
    ui->toolTable->setVerticalHeader(header);
    checkBox = new QCheckBox{cornerButton};
    checkBox->setFocusPolicy(Qt::NoFocus);
    checkBox->setGeometry(Header::getRect(cornerButton->rect()) /*.translated(1, -4)*/);
    connect(checkBox, &QCheckBox::clicked, [this](bool checked) { header->setAll(checked); });
    connect(header, &Header::onChecked, [this](int idx) {
        if(model) {
            int fl{};
            for(int i{}; i < model->rowCount(); ++i)
                if(model->useForCalc(i))
                    ++fl;

            checkBox->setCheckState(!fl ? Qt::Unchecked : (fl == model->rowCount() ? Qt::Checked : Qt::PartiallyChecked));
            pbCreate->setEnabled(fl);
        }
    });
}

void Form::on_cbxFileCurrentIndexChanged() {
    file = ui->cbxFile->currentData(Qt::UserRole).value<AbstractFile*>();
    qDebug() << file << file->id();
    if(!App::project().contains(file))
        return;

    try {
        // auto peview = std::any_cast<Preview>(App::filePlugin(int(file->type()))->getDataForGC(file, plugin));
        // auto peview = ui->cbxFile->currentData().value<mvector<const GraphicObject*>>();
        // for (auto* var : peview)
        // qDebug() << var->name << var->pos;

        using Key = std::pair<QByteArray, bool>;
        using Val = mvector<const GraphicObject*>;
        std::map<Key, Val> map;

        static mvector<GraphicObject> gos;
        gos = file->getDataForGC(criterias, GCType::Drill);

        for(auto& var: gos)
            map[Key{var.name, var.path.size() > 1}].emplace_back(&var);

        model = new Model{map.size(), ui->toolTable};
        auto& data = model->data();

        QColor color{App::settings().theme() > LightRed ? Qt::white : Qt::black};

        for(int i{}; auto& [key, val]: map) {
            auto& row = data[i++];
            row.icon = !key.second ? drawIcon(val.front()->fill, color) : drawDrillIcon(key.second ? Qt::red : color);
            row.name = QString(key.first).split('|');
            row.name.back() += ": Ø" + QString::number(std::any_cast<double>(val.front()->raw));
            row.diameter = std::any_cast<double>(val.front()->raw);
            row.isSlot = key.second;
            for(auto* go: val)
                new GiPreview{
                    (go->path.size() > 1 ? Path{go->path} : Path{go->pos}),
                    row.diameter,
                    data.back().toolId,
                    row,
                    go->fill};
        }

        App::grView().scene()->update();
    } catch(const std::exception& exc) {
        qDebug("%s: %s", __FUNCTION__, exc.what());
        return;
    } catch(...) {
        exit(-99);
        // delete ui->toolTable->model();
        return;
    }

    pickUpTool();

    delete ui->toolTable->model();
    ui->toolTable->setModel(model);
    connect(model, &Model::set, header, &Header::set);
    ui->toolTable->resizeColumnsToContents();
    ui->toolTable->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    connect(ui->toolTable->selectionModel(), &QItemSelectionModel::selectionChanged, this, &Form::on_selectionChanged);

    header->onChecked();
}

void Form::on_doubleClicked(const QModelIndex& current) {
    if(current.column() == 1) {
        mvector<Tool::Type> tools;
        // FIXME       tools = model->isSlot(current.row()) ? mvector<Tool::Type> {Tool::EndMill} : ((worckType == GCType::Profile || worckType == GCType::Pocket) ? mvector<Tool::Type> {Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser} : mvector<Tool::Type> {Tool::Drill, Tool::EndMill});
        ToolDatabase tdb(this, tools);
        if(tdb.exec()) {
            const Tool tool(tdb.tool());
            model->setToolId(current.row(), tool.id());
        }
    }
}

void Form::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    for(auto&& index: selected.indexes())
        for(auto&& item: model->data()[index.row()].items)
            item->setSelected(true);
    for(auto&& index: deselected.indexes())
        for(auto&& item: model->data()[index.row()].items)
            item->setSelected(false);
    zoomToSelected();
}

void Form::on_customContextMenuRequested(const QPoint& pos) {

    if(selectedIndexes().isEmpty())
        return;
    QMenu menu;
    menu.addAction(QIcon::fromTheme("view-form"), tr("&Select Tool"), [this] {
        bool flag = true;
        for(QModelIndex current: selectedIndexes()) {
            flag = model->isSlot(current.row());
            if(!flag)
                break;
        }

        mvector<Tool::Type> tools;
        if(flag)
            tools = mvector<Tool::Type>{Tool::EndMill};
        else
            tools = (worckType == GCType::Drill) ? mvector<Tool::Type>{Tool::Drill, Tool::EndMill} : mvector<Tool::Type>{Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser};

        ToolDatabase tdb(this, tools);
        if(tdb.exec()) {
            const Tool tool(tdb.tool());
            for(QModelIndex current: selectedIndexes()) {
                if(model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id());
                } else if(model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                    QMessageBox::information(this, "", "\"" + tool.name() + tr("\" not suitable for T") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + "-" + model->data(current.sibling(current.row(), 0)).toString() + "-");
                } else if(!model->isSlot(current.row())) {
                    if(model->toolId(current.row()) > -1 && !model->useForCalc(current.row()))
                        continue;
                    model->setToolId(current.row(), tool.id());
                }
            }
        }
    });

    for(QModelIndex current: selectedIndexes()) {
        if(model->toolId(current.row()) != -1) {
            menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool"), [this] {
                for(QModelIndex current: selectedIndexes())
                    model->setToolId(current.row(), -1);
            });
            break;
        }
    }

    menu.exec(ui->toolTable->viewport()->mapToGlobal(pos));
}

void Form::customContextMenuRequested(const QPoint& pos) {
    QMenu menu;
    menu.addAction(QIcon::fromTheme("view-form"), tr("&Choose a Tool for everyone"), [this] {
        ui->toolTable->selectAll();
        bool fl = true;
        for(QModelIndex current: selectedIndexes()) {
            fl = model->isSlot(current.row());
            if(!fl)
                break;
        }
        mvector<Tool::Type> tools;
        if(fl)
            tools = mvector<Tool::Type>{Tool::EndMill};
        else
            tools = (worckType == GCType::Drill) ? mvector<Tool::Type>{Tool::Drill, Tool::EndMill} : mvector<Tool::Type>{Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser};
        ToolDatabase tdb(this, tools);
        if(tdb.exec()) {
            const Tool tool(tdb.tool());
            for(QModelIndex current: selectedIndexes()) {
                if(model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id());
                } else if(model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                    QMessageBox::information(this, "", "\"" + tool.name() + tr("\" not suitable for T") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + "-" + model->data(current.sibling(current.row(), 0)).toString() + "-");
                } else if(!model->isSlot(current.row())) {
                    if(model->toolId(current.row()) > -1 && !model->useForCalc(current.row()))
                        continue;
                    model->setToolId(current.row(), tool.id());
                }
            }
        }
    });
    menu.addAction(QIcon::fromTheme("list-remove"), tr("&Remove Tool for everyone"), [this] {
        ui->toolTable->selectAll();
        for(QModelIndex current: selectedIndexes())
            model->setToolId(current.row(), -1);
        for(int i = 0; i < model->rowCount(); ++i)
            if(model->toolId(i) != -1)
                return;
    });
    menu.exec(ui->toolTable->horizontalHeader()->mapToGlobal(pos));
}

void Form::pickUpTool() {
    if(!model)
        return;
    const double k = 0.05; // 5%
    int ctr = 0;
    for(const auto& row: model->data()) {
        // model->setToolId(ctr++, 3);
        // continue;
        const double drillDiameterMin = row.diameter * (1.0 - k);
        const double drillDiameterMax = row.diameter * (1.0 + k);

        auto isFit = [&, this](auto& tool) -> bool {
            const auto diameter = tool.getDiameter(dsbxDepth->value());
            return drillDiameterMin <= diameter && drillDiameterMax >= diameter;
        };

        for(auto& [id, tool]: App::toolHolder().tools()) {
            if(!row.isSlot && (tool.type() == Tool::Drill || tool.type() == Tool::EndMill) && isFit(tool)) {
                model->setToolId(ctr, id);
                break;
            }
            if(row.isSlot && tool.type() == Tool::EndMill && isFit(tool)) {
                model->setToolId(ctr, id);
                break;
            }
        }
        ++ctr;
    }
}

void Form::updateState() {
    worckType = ui->rb_drilling->isChecked() ? GCType::Drill : (ui->rb_profile->isChecked() ? GCType::Profile : GCType::Pocket);
    ui->grbxSide->setEnabled(worckType == GCType::Profile);
    ui->grbxDirection->setEnabled(worckType == GCType::Profile || worckType == GCType::Pocket);
    side = ui->rb_on->isChecked() ? GCode::On : (ui->rb_out->isChecked() ? GCode::Outer : GCode::Inner);
}

void Form::errorOccurred() {
    auto tpc = (GCode::Creator*)sender();
    // tpc->continueCalc(ErrorDialog(std::move(tpc->items), this).exec());
}

QModelIndexList Form::selectedIndexes() const { return ui->toolTable->selectionModel()->selectedIndexes(); }

void Form::zoomToSelected() {
    if(ui->chbxZoomToSelected->isChecked())
        App::grView().zoomToSelected();
}

void Form::computePaths() {
    auto indexes = [](const auto& range) {
        QString indexes;
        for(int32_t id: range) {
            if(indexes.size())
                indexes += ",";
            indexes += QString::number(id);
        }
        return indexes;
    };

    if(worckType == GCType::Drill) { // slots only
        struct Data {
            Paths paths;
            mvector<int> toolsApertures;
        };

        std::map<int, Data> pathsMap;

        for(int i{}; auto&& row: *model) {
            if(row.toolId > -1 && row.useForCalc && row.isSlot) {
                pathsMap[row.toolId].toolsApertures.push_back(i);
                for(auto& item: row.items) {
                    if(item->fit(dsbxDepth->value()))
                        for(Path& path: offset(item->paths().front(), item->sourceDiameter() - App::toolHolder().tool(item->toolId()).diameter()))
                            pathsMap[row.toolId].paths.push_back(path);
                    else
                        pathsMap[row.toolId].paths.push_back(item->paths().front());
                }
            }
            model->setCreate(i++, !pathsMap.contains(row.toolId));
        }

        for(auto [usedToolId, _]: pathsMap) {
            (void)_;
            if(pathsMap[usedToolId].paths.size()) {
                // GCode::File* gcode = new GCode::File({App::toolHolder().tool{usedToolId), dsbxDepth->value(), GCType::Profile}, {pathsMap[usedToolId].paths});
                // gcode->setFileName(App::toolHolder().tool(usedToolId).nameEnc() + "_T" + indexes(pathsMap[usedToolId].toolsApertures));
                // gcode->setSide(file->side());
                // App::project().addFile(gcode);
            }
        }
    }

    { // other
        struct Data {
            Path drillPath;
            Paths paths;
            mvector<int> toolsApertures;
        };

        std::map<int, Data> pathsMap;

        for(int i{}; auto&& row: *model) {
            bool created{};
            if(row.toolId > -1 && row.useForCalc) {
                pathsMap[row.toolId].toolsApertures.push_back(i);
                for(auto& item: row.items) {
                    // if (item->isSlot())
                    // continue;
                    switch(worckType) {
                    case GCType::Profile:
                        if(App::toolHolder().tool(row.toolId).type() != Tool::Drill) {
                            switch(side) {
                            case GCode::On:
                                pathsMap[row.toolId].paths.append(item->paths());
                                created = true;
                                break;
                            case GCode::Outer:
                                pathsMap[row.toolId].paths.append(item->offset());
                                created = true;
                                break;
                            case GCode::Inner:
                                // if (item->fit(dsbxDepth->value())) {
                                pathsMap[row.toolId].paths.append(item->offset());
                                created = true;
                                // }
                                break;
                            }
                        }
                        break;
                    case GCType::Pocket:
                        if(App::toolHolder().tool(row.toolId).type() != Tool::Drill) {
                            // if (App::toolHolder().tool(row.toolId).type() != Tool::Drill && item->fit(dsbxDepth->value())) {
                            pathsMap[row.toolId].paths.append(item->offset());
                            created = true;
                            // }
                        }
                        break;
                    case GCType::Drill:
                        if(App::toolHolder().tool(row.toolId).type() != Tool::Engraver || App::toolHolder().tool(row.toolId).type() != Tool::Laser) {
                            pathsMap[row.toolId].drillPath.push_back(item->pos());
                            created = true;
                        }
                        break;
                    default:;
                    }
                }
            }
            if(created)
                model->setCreate(i, false);
            ++i;
        }

        for(auto& [toolId, val]: pathsMap) {
            if(val.drillPath.size()) {
                Point point1((App::home().pos()));
                { // sort by distance
                    size_t counter = 0;
                    while(counter < val.drillPath.size()) {
                        size_t selector = 0;
                        double length = std::numeric_limits<double>::max();
                        for(size_t i = counter, end = val.drillPath.size(); i < end; ++i) {
                            double length2 = point1.distTo(val.drillPath[i]);
                            if(length > length2) {
                                length = length2;
                                selector = i;
                            }
                        }
                        qSwap(val.drillPath[counter], val.drillPath[selector]);
                        point1 = val.drillPath[counter++];
                    }
                }
                GCode::File* gcode = new File{{App::toolHolder().tool(toolId), dsbxDepth->value()}, {{val.drillPath}}};
                gcode->setFileName(App::toolHolder().tool(toolId).nameEnc() + /*type_ +*/ indexes(val.toolsApertures));
                gcode->setSide(file->side());
                App::project().addFile(gcode);
            }
            if(val.paths.size()) {
                switch(worckType) {
                case GCType::Profile: {
                    auto gcp = new GCode::Params;
                    gcp->setConvent(ui->rbConventional->isChecked());
                    gcp->setSide(side);
                    gcp->tools = {App::toolHolder().tool(toolId)};
                    gcp->params[GCode::Params::Depth] = dsbxDepth->value();
                    gcp->closedPaths = std::move(val.paths);
                    setCreator(new Profile::Creator);
                    fileCount = 1;
                    emit createToolpath(gcp);
                } break;
                case GCType::Pocket: {
                    auto gcp = new GCode::Params;
                    gcp->setConvent(ui->rbConventional->isChecked());
                    gcp->setSide(GCode::Inner);
                    gcp->tools = {App::toolHolder().tool(toolId)};
                    gcp->params[GCode::Params::Depth] = dsbxDepth->value();
                    gcp->closedPaths = std::move(val.paths);
                    setCreator(new PocketOffset::Creator);
                    fileCount = 1;
                    emit createToolpath(gcp);
                } break;
                default:
                    continue;
                }
            }
        }
    }

    QTimer::singleShot(1, Qt::CoarseTimer, [this] { header->onChecked(); });
}

void Form::updateName() { }

void Form::hideEvent(QHideEvent *event) {
    delete ui->toolTable->model();
    event->accept();
}

void Form::editFile(GCode::File* file) { }



} // namespace Drilling

#include "moc_drill_form.cpp"

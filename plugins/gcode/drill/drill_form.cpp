/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "drill_form.h"
#include "circle/shape.h"
#include "drill_gi_preview.h"
#include "drill_header.h"
#include "drill_model.h"
#include "gi_point.h"
#include "tool_database.h"
#include "ui_drillform.h"
#include <boost/graph/bellman_ford_shortest_paths.hpp>

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

// Контур паза, расширенный до нужной ширины. delta у Geo::Inflate -- ПОЛНАЯ
// ширина, а offset здесь задан как смещение кромки, отсюда удвоение. Замыкать
// контуры руками больше не нужно: из точного домена они приходят замкнутыми.
Geo::Polylines offset(const Geo::Polyline& path, double offset) {
    return Geo::Inflate(Geo::Polylines{path}, offset * 2.0).contours();
}

/////////////////////////////////////////////

Form::Form(GCode::Plugin* plugin)
    : GCode::Form{plugin, nullptr}
    , ui(new Ui::DrillForm) {
    ui->setupUi(content);

    grid->setRowStretch(2, 1);
    grid->setRowStretch(7, 0);

    initToolTable();

    pbCreate->setEnabled(false);
    ui->rb_drilling->setChecked(true);

    {
        MySettings settings;
        settings.beginGroup(u"DrillForm"_s);
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
    settings.beginGroup(u"DrillForm"_s);
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

    if(auto shapes = App::project().shapes(::Gi::Type::ShCircle); shapes.size()) {
        ui->cbxFile->addItem(shapes.front()->icon(), shapes.front()->name(), QVariant::fromValue(shapes));
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
    // QMessageBox::information(nullptr, {}, tr("No data to process."));
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
    connect(checkBox, &QCheckBox::clicked, [this](bool checked) {
        if(1) { // NOTE CheckBox behavior #112
            Qt::CheckState state = header->checkBoxState;
            header->setAll(checked ? (state == Qt::Unchecked ? 1 : 0) : 0);
        } else {
            // qWarning() << checkBox->isChecked() << checkBox->isTristate() << checkBox->checkState() << checked;
            // header->setAll((checkBox->checkState() == Qt::PartiallyChecked) ^ checked);
            // header->setAll((checkBox->checkState() == Qt::PartiallyChecked) ^ checked);
            checkBox->setChecked(!!checkBox->checkState());
            header->setAll(!checkBox->checkState());
        }
    });
    connect(header, &Header::onChecked, [this](int /*idx*/) {
        if(model) {
            int fl{};
            for(int i{}; i < model->rowCount(); ++i)
                if(model->useForCalc(i)) ++fl;
            if(1) { // NOTE CheckBox behavior #112
                header->checkBoxState = !fl ? Qt::Unchecked : (fl == model->rowCount() ? Qt::Checked : Qt::PartiallyChecked);
                checkBox->setCheckState(header->checkBoxState);
            } else {
                checkBox->setCheckState(!fl ? Qt::Unchecked
                                            : (fl == model->rowCount() ? Qt::Checked
                                                                       : Qt::PartiallyChecked));
            }
            pbCreate->setEnabled(fl);
        }
    });
}

void Form::on_cbxFileCurrentIndexChanged() {
    auto execFile = [this] {
        qDebug() << file << file->id();
        if(!App::project().contains(file)) return;
        // auto peview = std::any_cast<Preview>(App::filePlugin(int(file->type()))->getDataForGC(file, plugin));
        // auto peview = ui->cbxFile->currentData().value<std::vector<const GraphicObject*>>();
        // for (auto* var : peview)
        // qDebug() << var->name << var->pos;

        using Key = std::pair<QString, bool>;
        using Val = std::vector<const GraphicObject*>;
        std::map<Key, Val> map;

        static std::vector<GraphicObject> gos;
        gos = file->getDataForGC(criterias, GCType::Drill);

        for(auto& var: gos) {
            // if(var.raw.has_value())
            // qWarning() << std::any_cast<double>(var.raw);
            map[Key{var.name, var.path.size() > 1}].emplace_back(&var);
        }

        model = new Model{map.size(), ui->toolTable};
        auto& data = model->data();

        QColor color{App::settings().theme() == DarkTheme ? Qt::white : Qt::black};

        for(int i{}; auto& [key, val]: map) {
            auto& row = data[i++];
            row.icon = !key.second ? drawIcon(val.front()->fill, color) : drawDrillIcon(key.second ? Qt::red : color);
            row.name = QString(key.first).split(u'|');
            row.name.back() += u": Ø"_s + QString::number(std::any_cast<double>(val.front()->raw));
            row.diameter = std::any_cast<double>(val.front()->raw);
            row.isSlot = key.second;
            for(auto* go: val)
                new Gi::Preview{
                    (go->path.size() > 1 ? Geo::Polyline{go->path} : Geo::Polyline{Geo::Vertex{go->pos}}),
                    row.diameter,
                    data.back().toolId,
                    row,
                    go->fill.contours()};
        }
    };
    auto execShapes = [this] {
        auto shapes = ui->cbxFile->currentData(Qt::UserRole).value<std::vector<Shapes::AbstractShape*>>();
        qInfo() << shapes.size();

        using Key = std::pair<QString, double>;
        using Val = std::vector<Shapes::AbstractShape*>;
        std::map<Key, Val> map;

        for(auto* shape: shapes)
            if(shape->isVisible())
                map[Key{shape->name(), std::any_cast<double>(shape->getVal(ShCirc::Shape::Diameter))}].emplace_back(shape);

        model = new Model{map.size(), ui->toolTable};
        auto& data = model->data();

        QColor color{App::settings().theme() == DarkTheme ? Qt::white : Qt::black};

        for(int i{}; auto& [key, shapes]: map) {
            auto& row = data[i++];
            row.icon = shapes.front()->icon();
            row.name = QString(key.first).split(u'|');
            row.diameter = std::any_cast<double>(shapes.front()->getVal(ShCirc::Shape::Diameter));
            row.name.back() += u": Ø%1"_s.arg(row.diameter);
            row.isSlot = false;
            for(auto* shape: shapes)
                new Gi::Preview{
                    Geo::Polyline{Geo::Vertex{std::any_cast<QPointF>(shape->getVal(ShCirc::Shape::Center))}},
                    row.diameter,
                    data.back().toolId,
                    row,
                    Geo::fromPath(shape->shape())};
        }
    };
    try {
        file = ui->cbxFile->currentData(Qt::UserRole).value<AbstractFile*>();
        if(file) execFile();
        else execShapes();
        App::grView().scene()->update();
    } catch(const std::exception& exc) {
        qDebug("%s: %s", __FUNCTION__, exc.what()); //-V576
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
        std::vector<Tool::Type> tools;
        // FIXME       tools = model->isSlot(current.row()) ? std::vector<Tool::Type> {Tool::EndMill} : ((worckType == GCType::Profile || worckType == GCType::Pocket) ? std::vector<Tool::Type> {Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser} : std::vector<Tool::Type> {Tool::Drill, Tool::EndMill});
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
    menu.addAction(QIcon::fromTheme(u"view-form"_s), tr("&Select Tool"), [this] {
        bool flag = true;
        for(QModelIndex current: selectedIndexes()) {
            flag = model->isSlot(current.row());
            if(!flag)
                break;
        }

        std::vector<Tool::Type> tools;
        if(flag)
            tools = std::vector<Tool::Type>{Tool::EndMill};
        else
            tools = (worckType == GCType::Drill) ? std::vector<Tool::Type>{Tool::Drill, Tool::EndMill} : std::vector<Tool::Type>{Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser};

        ToolDatabase tdb(this, tools);
        if(tdb.exec()) {
            const Tool tool(tdb.tool());
            for(QModelIndex current: selectedIndexes()) {
                if(model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id());
                } else if(model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                    QMessageBox::information(this, {}, u"\""_s + tool.name() + tr("\" not suitable for Tu") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + u"_s-u"_s + model->data(current.sibling(current.row(), 0)).toString() + u"_s-"_s);
                } else if(!model->isSlot(current.row())) {
                    // Новый выбор заменяет старый инструмент независимо от текущей
                    // галки -- setToolId сам взводит её обратно.
                    model->setToolId(current.row(), tool.id());
                }
            }
        }
    });

    for(QModelIndex current: selectedIndexes()) {
        if(model->toolId(current.row()) != Tool::ID::Null) {
            menu.addAction(QIcon::fromTheme(u"list-remove"_s), tr("&Remove Tool"), [this] {
                for(QModelIndex current: selectedIndexes())
                    model->setToolId(current.row(), Tool::ID::Null);
            });
            break;
        }
    }

    menu.exec(ui->toolTable->viewport()->mapToGlobal(pos));
}

void Form::customContextMenuRequested(const QPoint& pos) {
    QMenu menu;
    menu.addAction(QIcon::fromTheme(u"view-form"_s), tr("&Choose a Tool for everyone"), [this] {
        ui->toolTable->selectAll();
        bool fl = true;
        for(QModelIndex current: selectedIndexes()) {
            fl = model->isSlot(current.row());
            if(!fl)
                break;
        }
        std::vector<Tool::Type> tools;
        if(fl)
            tools = std::vector<Tool::Type>{Tool::EndMill};
        else
            tools = (worckType == GCType::Drill) ? std::vector<Tool::Type>{Tool::Drill, Tool::EndMill} : std::vector<Tool::Type>{Tool::Drill, Tool::EndMill, Tool::Engraver, Tool::Laser};
        ToolDatabase tdb(this, tools);
        if(tdb.exec()) {
            const Tool tool(tdb.tool());
            for(QModelIndex current: selectedIndexes()) {
                if(model->isSlot(current.row()) && tool.type() == Tool::EndMill) {
                    model->setToolId(current.row(), tool.id());
                } else if(model->isSlot(current.row()) && tool.type() != Tool::EndMill) {
                    QMessageBox::information(this, {}, u"\""_s + tool.name() + tr("\" not suitable for Tu") + model->data(current.sibling(current.row(), 0), Qt::UserRole).toString() + u"_s-u"_s + model->data(current.sibling(current.row(), 0)).toString() + u"_s-"_s);
                } else if(!model->isSlot(current.row())) {
                    // Новый выбор заменяет старый инструмент независимо от текущей
                    // галки -- setToolId сам взводит её обратно.
                    model->setToolId(current.row(), tool.id());
                }
            }
        }
    });
    menu.addAction(QIcon::fromTheme(u"list-remove"_s), tr("&Remove Tool for everyone"), [this] {
        ui->toolTable->selectAll();
        for(QModelIndex current: selectedIndexes())
            model->setToolId(current.row(), Tool::ID::Null);
        for(int i{}; i < model->rowCount(); ++i)
            if(model->toolId(i) != Tool::ID::Null)
                return;
    });
    menu.exec(ui->toolTable->horizontalHeader()->mapToGlobal(pos));
}

void Form::pickUpTool() {
    if(!model) return;

    const double k = 0.05; // 5%
    int ctr{};
    for(const auto& row: model->data()) {
        // model->setToolId(ctr++, 3);
        // continue;
        const double drillDiameterMin = row.diameter * (1.0 - k);
        const double drillDiameterMax = row.diameter * (1.0 + k);

        auto isFit = [&, this](auto& tool) -> bool {
            const auto diameter = tool.getDiameter(dsbxDepth->value());
            return drillDiameterMin <= diameter && drillDiameterMax >= diameter;
        };
        auto set = !row.isSlot ? std::set{Tool::Drill, Tool::EndMill} : std::set{Tool::EndMill};
        auto filter = v::filter([&set](auto& t) { return set.contains(t.second.type()); });
        for(auto& [id, tool]: App::toolHolder().tools() | filter) {
            qWarning() << tool;
            if(model->toolId(ctr) < Tool::ID::Tool && isFit(tool)) {
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
    [[maybe_unused]] auto tpc = (GCode::Creator*)sender();
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
                indexes += u","_s;
            indexes += QString::number(id);
        }
        return indexes;
    };

    if(worckType == GCType::Drill) { // slots only
        struct Data {
            Geo::Polylines paths;
            std::vector<int> toolsApertures;
        };

        std::map<Tool::ID, Data> pathsMap;

        for(int i{}; auto&& row: *model) {
            if(row.toolId > Tool::ID::Null && row.useForCalc && row.isSlot) {
                pathsMap[row.toolId].toolsApertures.push_back(i);
                for(auto& item: row.items) {
                    if(!item->isUsed()) continue;
                    if(item->fit(dsbxDepth->value()))
                        for(Geo::Polyline& path: offset(item->paths().front(), item->sourceDiameter() - App::toolHolder().tool(item->toolId()).diameter()))
                            pathsMap[row.toolId].paths.push_back(path);
                    else
                        pathsMap[row.toolId].paths.push_back(item->paths().front());
                }
            }
            ++i;
        }

        for(auto [usedToolId, data]: pathsMap) {
            if(data.paths.size()) {
                auto* gcode = new File{
                    GCode::Params{
                                  App::toolHolder().tool(usedToolId),
                                  dsbxDepth->value(),
                                  std::move(data.paths),
                                  }
                };
                gcode->setRows(rowRefs(data.toolsApertures));
                gcode->setWorckType(static_cast<int>(worckType));
                gcode->setSrcFileId(file ? file->id() : -1);
                gcode->setFileName(
                    App::toolHolder().tool(usedToolId).nameEnc()
                    + u"_T"_s + indexes(pathsMap[usedToolId].toolsApertures));
                if(file) gcode->setSide(file->side());

                auto it = createdSlotFileIds_.find(usedToolId);
                if(it != createdSlotFileIds_.end() && App::project().file(it->second))
                    App::project().replaceFile(it->second, gcode);
                else
                    createdSlotFileIds_[usedToolId] = App::project().addFile(gcode);

                // УП по этим строкам реально получилась -- только теперь снимаем галки.
                for(int row: data.toolsApertures)
                    model->setCreate(row, false);
            }
        }
    }

    { // other
        struct Data {
            Geo::Polyline drillPath;
            Geo::Polylines paths;
            std::vector<int> toolsApertures;
        };

        std::map<Tool::ID, Data> pathsMap;

        for(int i{}; auto&& row: *model) {
            if(row.toolId > Tool::ID::Null && row.useForCalc) {
                pathsMap[row.toolId].toolsApertures.push_back(i);
                for(auto& item: row.items) {
                    if(!item->isUsed()) continue;
                    // if (item->isSlot())
                    // continue;
                    switch(worckType) {
                    case GCType::Profile:
                        if(App::toolHolder().tool(row.toolId).type() != Tool::Drill) {
                            switch(side) {
                            case GCode::On:
                                pathsMap[row.toolId].paths.append_range(item->paths());
                                break;
                            case GCode::Outer:
                                pathsMap[row.toolId].paths.append_range(item->offset());
                                break;
                            case GCode::Inner:
                                // if (item->fit(dsbxDepth->value())) {
                                pathsMap[row.toolId].paths.append_range(item->offset());
                                // }
                                break;
                            }
                        }
                        break;
                    case GCType::Pocket:
                        if(App::toolHolder().tool(row.toolId).type() != Tool::Drill) {
                            // if (App::toolHolder().tool(row.toolId).type() != Tool::Drill && item->fit(dsbxDepth->value())) {
                            pathsMap[row.toolId].paths.append_range(item->offset());
                            // }
                        }
                        break;
                    case GCType::Drill:
                        if(App::toolHolder().tool(row.toolId).type() != Tool::Engraver || App::toolHolder().tool(row.toolId).type() != Tool::Laser) {
                            pathsMap[row.toolId].drillPath.emplace_back(item->pos());
                        }
                        break;
                    default:;
                    }
                }
            }
            ++i;
        }

        for(auto& [toolId, val]: pathsMap) {
            if(val.drillPath.size()) {
                GCode::sortByProximity(val.drillPath, App::home().pos());
                // Центры отверстий едут одной полилинией: saveDrill читает их
                // из toolPathss, а не как контур -- полигоном тут ничего не
                // задаётся, у набора точек нет ни площади, ни обхода.
                auto* gcode = new File{
                    GCode::Params{
                                  App::toolHolder().tool(toolId),
                                  dsbxDepth->value(),
                                  Geo::Polylines{std::move(val.drillPath)},
                                  }
                };

                gcode->setRows(rowRefs(val.toolsApertures));
                gcode->setWorckType(static_cast<int>(worckType));
                gcode->setSrcFileId(file ? file->id() : -1);
                gcode->setFileName(App::toolHolder().tool(toolId).nameEnc() + /*type_ +*/ indexes(val.toolsApertures));
                if(file) gcode->setSide(file->side());

                auto it = createdDrillFileIds_.find(toolId);
                if(it != createdDrillFileIds_.end() && App::project().file(it->second))
                    App::project().replaceFile(it->second, gcode);
                else
                    createdDrillFileIds_[toolId] = App::project().addFile(gcode);

                // УП по этим строкам реально получилась -- только теперь снимаем галки.
                for(int row: val.toolsApertures)
                    model->setCreate(row, false);
            }
            if(val.paths.size()) {
                switch(worckType) {
                case GCType::Profile: {
                    pendingRows_ = rowRefs(val.toolsApertures);
                    gcp = {};
                    gcp.setConvent(ui->rbConventional->isChecked());
                    gcp.setSide(side);
                    gcp.tools = {App::toolHolder().tool(toolId)};
                    gcp.params[GCode::Params::Depth] = dsbxDepth->value();
                    gcp.closedCurves = Geo::Polygons{val.paths};
                    setCreator(new Profile::Creator);
                    fileCount = 1;
                    emit createToolpath(&gcp);
                } break;
                case GCType::Pocket: {
                    pendingRows_ = rowRefs(val.toolsApertures);
                    gcp = {};
                    gcp.setConvent(ui->rbConventional->isChecked());
                    gcp.setSide(GCode::Inner);
                    gcp.tools = {App::toolHolder().tool(toolId)};
                    gcp.params[GCode::Params::Depth] = dsbxDepth->value();
                    gcp.closedCurves = Geo::Polygons{val.paths};
                    setCreator(new PocketOffset::Creator);
                    fileCount = 1;
                    emit createToolpath(&gcp);
                } break;
                default: continue;
                }
            }
        }
    }

    QTimer::singleShot(1, Qt::CoarseTimer, [this] { header->onChecked(); });
}

std::vector<RowRef> Form::rowRefs(const std::vector<int>& indexes) const {
    std::vector<RowRef> refs;
    if(!model) return refs;
    const auto& rows = model->data();
    for(int i: indexes)
        if(0 <= i && std::cmp_less(i, rows.size()))
            refs.emplace_back(rows[i].name.join(u'|'), rows[i].toolId, rows[i].useForCalc);
    return refs;
}

void Form::uncheckRows(const std::vector<RowRef>& rows) {
    if(!model) return;
    auto& data = model->data();
    for(const RowRef& ref: rows)
        for(size_t i{}; i < data.size(); ++i)
            if(data[i].name.join(u'|') == ref.name) {
                model->setCreate(static_cast<int>(i), false);
                break;
            }
}

void Form::fileHandler(GCode::File* file_) {
    // Профиль и карман считает чужой Creator, а результат приходит асинхронно:
    // галки снимаем только теперь, когда успех (file_ != nullptr) уже известен.
    if(file_)
        uncheckRows(pendingRows_);

    if(auto* drillFile = dynamic_cast<File*>(file_); drillFile) {
        drillFile->setRows(std::move(pendingRows_));
        drillFile->setWorckType(static_cast<int>(worckType));
        drillFile->setSrcFileId(file ? file->id() : -1);
    }
    pendingRows_.clear();
    GCode::Form::fileHandler(file_);
}

void Form::editFile(GCode::File* file_) {
    auto* drillFile = dynamic_cast<File*>(file_);
    if(!drillFile) return GCode::Form::editFile(file_);

    // Режим -- первым: от него зависит, какие столбцы и кнопки вообще активны.
    switch(static_cast<GCType>(drillFile->worckType())) {
    case GCType::Profile: ui->rb_profile->setChecked(true); break;
    case GCType::Pocket : ui->rb_pocket->setChecked(true); break;
    default             : ui->rb_drilling->setChecked(true); break;
    }
    updateState();

    // Таблица строится по выбранному в cbxFile файлу: без него имена строк не с
    // чем сопоставлять. Смена индекса перестраивает модель синхронно.
    if(drillFile->srcFileId() > -1)
        for(int i{}; i < ui->cbxFile->count(); ++i)
            if(auto* f = ui->cbxFile->itemData(i, Qt::UserRole).value<AbstractFile*>();
                f && f->id() == drillFile->srcFileId()) {
                ui->cbxFile->setCurrentIndex(i);
                break;
            }

    if(model) {
        // Восстанавливаются ТОЛЬКО свои строки: у остальных галка снимается,
        // иначе повторный расчёт захватил бы чужое.
        std::map<QString, const RowRef*> byName;
        for(const RowRef& ref: drillFile->rows()) byName.emplace(ref.name, &ref);

        auto& rows = model->data();
        for(size_t i{}; i < rows.size(); ++i) {
            auto it = byName.find(rows[i].name.join(u'|'));
            const bool mine = it != byName.end();
            if(mine && it->second->toolId > Tool::ID::Null)
                model->setToolId(static_cast<int>(i), it->second->toolId);
            model->setCreate(static_cast<int>(i), mine && it->second->useForCalc);
        }
        header->onChecked();
    }

    GCode::Form::editFile(file_);
}

void Form::updateName() { }

void Form::hideEvent(QHideEvent* event) { // NOTE clean and hide pr gi
    delete ui->toolTable->model();
    model = nullptr;
    event->accept();
}

} // namespace Drilling

#include "moc_drill_form.cpp"

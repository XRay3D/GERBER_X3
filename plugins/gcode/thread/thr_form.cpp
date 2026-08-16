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
#include "thr_form.h"
#include "circle/shape.h"
#include "gi_point.h"
#include "thr_delegate.h"
#include "thr_gi_preview.h"
#include "thr_header.h"
#include "thr_model.h"
#include "tool_database.h"
#include "ui_thrform.h"
#include <boost/graph/bellman_ford_shortest_paths.hpp>

// #include "file.h"
// #include "gi_point.h"
// #include "gi_preview.h"
#include "graphicsview.h"
// #include "project.h"
// #include "settings.h"
// #include "tool_pch.h"

#include <QHeaderView>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QThread>
#include <QTimer>
#include <QToolBar>
#include <QWidget>
#include <set>

namespace Threading {

/////////////////////////////////////////////

Form::Form(GCode::Plugin* plugin)
    : GCode::Form{plugin, nullptr}
    , ui(new Ui::ThreadForm) {
    ui->setupUi(content);

    grid->setRowStretch(2, 1);
    grid->setRowStretch(7, 0);

    initToolTable();

    pbCreate->setEnabled(false);
    ui->rbInside->setChecked(true);

    {
        MySettings settings;
        settings.beginGroup(u"ThreadForm"_s);
        settings.getValue(ui->rbClimb, true);
        settings.getValue(ui->rbConventional);
        settings.getValue(ui->rbInside, true);
        settings.getValue(ui->rbOutside);
        settings.getValue(ui->rbLeft);
        settings.getValue(ui->rbRight, true);
        settings.getValue(ui->chbxZoomToSelected);
        settings.getValue(ui->chbxCircle);
        settings.getValue(ui->chbxChamfer);
        settings.getValue(ui->sbxStarts, 1);
        settings.endGroup();
    }

    //
    connect(ui->pbPickUpTools, &QPushButton::clicked, this, &Form::pickUpTool);
    connect(ui->rbRight, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rbLeft, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rbClimb, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &Form::updateState);
    connect(ui->rbInside, &QRadioButton::clicked, this, &Form::updateState);

    updateButtonIconSize();
    updateState();
    updateFiles();
    setWindowTitle(tr("Thread"));
    // FIXME App::setDrillForm(this);
}

Form::~Form() {
    MySettings settings;
    settings.beginGroup(u"ThreadForm"_s);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOutside);
    settings.setValue(ui->rbLeft);
    settings.setValue(ui->rbRight);
    settings.setValue(ui->chbxZoomToSelected);
    settings.setValue(ui->chbxCircle);
    settings.setValue(ui->chbxChamfer);
    settings.setValue(ui->sbxStarts);
    settings.endGroup();

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
    ui->toolTable->setItemDelegateForColumn(Model::Correction, new Delegate{this});
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
        // auto peview = ui->cbxFile->currentData().value<mvector<const GraphicObject*>>();
        // for (auto* var : peview)
        // qDebug() << var->name << var->pos;

        using Key = QString;
        using Val = std::vector<const GraphicObject*>;
        std::map<Key, Val> map;

        static std::vector<GraphicObject> gos;
        gos = file->getDataForGC(criterias, GCType::Drill);

        for(auto& var: gos) {
            // if(var.raw.has_value())
            // qWarning() << std::any_cast<double>(var.raw);
            map[var.name].emplace_back(&var);
        }

        model = new Model{map.size(), ui->toolTable};
        auto& data = model->data();

        QColor color{App::settings().theme() == DarkTheme ? Qt::white : Qt::black};

        for(int i{}; auto& [key, val]: map) {
            auto& row = data[i++];
            row.icon = drawIcon(val.front()->fill, color);
            row.name = key.split(u'|');
            row.name.back() += u": Ø"_s + QString::number(std::any_cast<double>(val.front()->raw));
            row.diameter = std::any_cast<double>(val.front()->raw);
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
    ui->toolTable->horizontalHeader()->setSectionResizeMode(Model::Correction, QHeaderView::ResizeToContents);
    ui->toolTable->horizontalHeader()->setSectionResizeMode(Model::HoleDiameter, QHeaderView::ResizeToContents);
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
        ToolDatabase tdb{this, std::array{Tool::ThreadMill}};
        if(tdb.exec()) {
            for(QModelIndex current: selectedIndexes())
                model->setToolId(current.row(), tdb.tool().id());
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
        ToolDatabase tdb{this, std::array{Tool::ThreadMill}};
        if(tdb.exec()) {
            for(QModelIndex current: selectedIndexes())
                model->setToolId(current.row(), tdb.tool().id());
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
        std::set set{Tool::ThreadMill};
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
    // worckType = ui->rbInside->isChecked() ? GCType::Drill : (ui->rb_profile->isChecked() ? GCType::Profile : GCType::Pocket);
    // ui->grbxSide->setEnabled(worckType == GCType::Profile);
    // ui->grbxDirection->setEnabled(worckType == GCType::Profile || worckType == GCType::Pocket);
    // side = ui->rb_on->isChecked() ? GCode::On : (ui->rb_out->isChecked() ? GCode::Outer : GCode::Inner);
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

    struct Data {
        Geo::Polylines paths;
        std::vector<int> toolsApertures;
    };

    std::map<Tool::ID, Data> pathsMap;

    // Each thread mark is encoded as a 3-point path: {center, center + (M/2, 0),
    // center + (0, holeD/2)}. The threading.js script derives the hole/rod
    // center, its nominal thread diameter M and its pre-drilled hole diameter
    // from that triple; none of it is otherwise available per-point.
    for(int i{}; auto&& row: *model) {
        if(row.toolId > Tool::ID{} && row.useForCalc) {
            pathsMap[row.toolId].toolsApertures.push_back(i);
            for(auto& item: row.items) {
                if(!item->isUsed()) continue;
                const QPointF center{item->pos()};
                const double radius{item->sourceDiameter() * 0.5 + row.correction};
                const double holeRadius{row.holeDiameter * 0.5};
                pathsMap[row.toolId].paths.push_back(Geo::Polyline{
                    Geo::Vertex{center},
                    Geo::Vertex{QPointF{center.x() + radius, center.y()}},
                    Geo::Vertex{QPointF{center.x(), center.y() + holeRadius}}
                });
            }
            model->setCreate(i++, !pathsMap.contains(row.toolId));
        }
    }

    for(auto [usedToolId, data]: pathsMap) {
        if(data.paths.size()) {
            GCode::Params gcp{App::toolHolder().tool(usedToolId), dsbxDepth->value(), std::move(data.paths)};
            gcp.setSide(ui->rbInside->isChecked() ? GCode::Inner : GCode::Outer);
            gcp.setConvent(!ui->rbClimb->isChecked());
            gcp.setLeftHand(ui->rbLeft->isChecked());
            gcp.setCircle(ui->chbxCircle->isChecked());
            gcp.setChamfer(ui->chbxChamfer->isChecked());
            gcp.setStarts(ui->sbxStarts->value());

            GCode::File* gcode = new File{std::move(gcp)};
            gcode->setFileName(
                App::toolHolder().tool(usedToolId).nameEnc()
                + u"_T"_s + indexes(pathsMap[usedToolId].toolsApertures));
            if(file) gcode->setSide(file->side());

            // Повторный Create для уже пересчитанного в этом показе формы
            // инструмента -- заменить его УП на месте (тот же id/узел дерева),
            // а не плодить дубли; при первом Create или после переоткрытия
            // формы (createdFileIds_ сброшен в showEvent) -- завести новую.
            auto it = createdFileIds_.find(usedToolId);
            if(it != createdFileIds_.end() && App::project().file(it->second))
                App::project().replaceFile(it->second, gcode);
            else
                createdFileIds_[usedToolId] = App::project().addFile(gcode);
        }
    }

    QTimer::singleShot(1, Qt::CoarseTimer, [this] { header->onChecked(); });
}

void Form::updateName() { }

void Form::hideEvent(QHideEvent* event) { // NOTE clean and hide pr gi
    delete ui->toolTable->model();
    model = nullptr;
    event->accept();
}

void Form::editFile(GCode::File* /*file*/) { }

} // namespace Threading

#include "moc_thr_form.cpp"

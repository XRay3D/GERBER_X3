// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thread_form.h"

#include "gc_gi_bridge.h"
// #include "graphicsview.h"
// #include "settings.h"
#include "ui_threadform.h"
#include <QMessageBox>
#include <ranges>

namespace Thread {

static QStringList side{QObject::tr("Null"), QObject::tr("Outer"), QObject::tr("Inner")};

struct ThreadParam {
    int thread;
    int side;
    int toolT;
    int tool;
    double depth;
    double x;
    double y;
};

class Model : public QAbstractTableModel {
    //    Q_OBJECT

    std::vector<ThreadParam> data_;

public:
    Model(QObject* parent = nullptr)
        : QAbstractTableModel{parent} { data_.resize(1); }
    ~Model() override = default;

    // QAbstractItemModel interface
    int rowCount(const QModelIndex& parent) const override { return data_.size(); }
    int columnCount(const QModelIndex& parent) const override { return 7; }

    QVariant data(const QModelIndex& index, int role) const override {
        auto& data = data_.at(index.row());
        if(role == Qt::DisplayRole) {
            switch(index.column()) {
            case 0: return Settings::threads.at(data.thread).toStr();
            case 1: return side[data.side];
            case 2:
                if(App::toolHolder().tools().contains(data.toolT))
                    return App::toolHolder().tool(data.toolT).name();
            case 3:
                if(App::toolHolder().tools().contains(data.tool))
                    return App::toolHolder().tool(data.tool).name();
            case 4: return data.depth;
            case 5: return data.x;
            case 6: return data.y;
            }
        }
        if(role == Qt::EditRole) {
            switch(index.column()) {
            case 0: return data.thread;
            case 1: return data.side;
            case 2: return data.toolT;
            case 3: return data.tool;
            case 4: return data.depth;
            case 5: return data.x;
            case 6: return data.y;
            }
        }

        return {};
    }

    bool setData(const QModelIndex& index, const QVariant& value, int role) override {
        auto& data = data_.at(index.row());
        if(role == Qt::EditRole) {
            switch(index.column()) {
            case 0: data.thread = value.value<decltype(data.thread)>(); return true;
            case 1: data.side = value.value<decltype(data.side)>(); return true;
            case 2: data.toolT = value.value<decltype(data.toolT)>(); return true;
            case 3: data.tool = value.value<decltype(data.tool)>(); return true;
            case 4: data.depth = value.value<decltype(data.depth)>(); return true;
            case 5: data.x = value.value<decltype(data.x)>(); return true;
            case 6: data.y = value.value<decltype(data.y)>(); return true;
            }
        }
        return {};
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
        if(orientation == Qt::Horizontal && role == Qt::DisplayRole) {
            switch(section) {
            case 0: return tr("Thread");
            case 1: return tr("Side");
            case 2: return tr("ToolT");
            case 3: return tr("ToolD");
            case 4: return tr("Depth, mm");
            case 5: return tr("X, mm");
            case 6: return tr("Y, mm");
            }
        }
        return QAbstractTableModel::headerData(section, orientation, role);
    }

    Qt::ItemFlags flags(const QModelIndex& index) const override {
        return QAbstractTableModel::flags(index) | Qt::ItemIsEditable;
    }
};

class Delegate : public QStyledItemDelegate {

public:
    Delegate(QObject* parent)
        : QStyledItemDelegate{parent} { }
    ~Delegate() override = default;

    // QAbstractItemDelegate interface

    QWidget* createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const override {
        switch(index.column()) {
        case 0: {
            auto cbx = new QComboBox{parent};
            for(int i{}; auto&& thread: Settings::threads)
                cbx->addItem(thread.toStr(), i++);
            return cbx;
        }
        case 1: {
            auto cbx = new QComboBox{parent};
            for(int i{}; auto&& side: side)
                cbx->addItem(side, i++);
            return cbx;
        }
        case 2: {
            auto cbx = new QComboBox{parent};
            for(auto&& [id, tool]: App::toolHolder().tools())
                if(tool.type() == Tool::ThreadMill)
                    cbx->addItem(tool.name(), tool.id());
            return cbx;
        }
        case 3: {
            auto cbx = new QComboBox{parent};
            for(auto&& [id, tool]: App::toolHolder().tools())
                if(tool.type() == Tool::EndMill || tool.type() == Tool::Drill)
                    cbx->addItem(tool.name(), tool.id());
            return cbx;
        }
        case 4:
        case 5:
        case 6: return new QDoubleSpinBox{parent};
        }
        return nullptr;
    }
    void setEditorData(QWidget* editor, const QModelIndex& index) const override {
        if(auto* sbx = dynamic_cast<QDoubleSpinBox*>(editor); sbx)
            sbx->setDecimals(3);
        QStyledItemDelegate::setEditorData(editor, index);
    }
    void setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const override {
        if(auto* sbx = dynamic_cast<QDoubleSpinBox*>(editor); sbx)
            model->setData(index, sbx->value());
        else if(auto* cbx = dynamic_cast<QComboBox*>(editor); cbx)
            model->setData(index, cbx->currentData());

        QStyledItemDelegate::setModelData(editor, model, index);
    }
};

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new Ui::ThreadForm) {
    ui->setupUi(content);
    setWindowTitle(tr("Thread Toolpath"));

    ui->tableView->setModel(new Model{ui->tableView});
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->setItemDelegate(new Delegate{ui->tableView});

    //    MySettings settings;
    //    settings.beginGroup("Thread");
    //    settings.getValue(ui->dsbxBridgeLenght, 1.0);
    //    settings.getValue(ui->rbClimb);
    //    settings.getValue(ui->rbConventional);
    //    settings.getValue(ui->rbInside);
    //    settings.getValue(ui->rbOn);
    //    settings.getValue(ui->rbOutside);
    //    settings.getValue(ui->cbxTrimming);
    //    settings.getValue(ui->dsbxBridgeValue, 1.0);
    //    settings.getValue(ui->cbxBridgeAlignType, 1.0);
    //    settings.getValue(varName(trimming_), 0);
    //    settings.endGroup();

    //    rb_clicked();

    //    // clang-format off
    //    connect(App::grViewPtr(), &GraphicsView::mouseMove,      this, &Form::updateBridgePos);
    //    connect(dsbxDepth,              &DepthForm::valueChanged,      this, &Form::updateBridges);
    //    connect(leName,                 &QLineEdit::textChanged,       this, &Form::onNameTextChanged);
    //    connect(ui->dsbxBridgeLenght,   &QDoubleSpinBox::valueChanged, this, &Form::updateBridges);
    //    connect(ui->pbAddBridge,        &QPushButton::clicked,         this, &Form::onAddBridgeClicked);
    //    connect(ui->rbClimb,            &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbConventional,     &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbInside,           &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbOn,               &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->rbOutside,          &QRadioButton::clicked,        this, &Form::rb_clicked);
    //    connect(ui->toolHolder,         &ToolSelectorForm::updateName, this, &Form::updateName);
    //    // clang-format on

    //    connect(ui->cbxTrimming, &QCheckBox::toggled, [this](bool checked) {
    //        if (side == GCode::On)
    //            checked ? trimming_ |= Trimming::Line : trimming_ &= ~Trimming::Line;
    //        else
    //            checked ? trimming_ |= Trimming::Corner : trimming_ &= ~Trimming::Corner;
    //    });
}

Form::~Form() {
    //    MySettings settings;
    //    settings.beginGroup("Thread");
    //    settings.setValue(ui->dsbxBridgeLenght);
    //    settings.setValue(ui->rbClimb);
    //    settings.setValue(ui->rbConventional);
    //    settings.setValue(ui->rbInside);
    //    settings.setValue(ui->rbOn);
    //    settings.setValue(ui->rbOutside);
    //    settings.setValue(ui->cbxTrimming);
    //    settings.setValue(ui->cbxBridgeAlignType);
    //    settings.setValue(ui->dsbxBridgeValue);
    //    settings.setValue(varName(trimming_));
    //    settings.endGroup();

    //    for (QGraphicsItem* giItem : App::grView().items()) {
    //        if (giItem->type() == Gi::Type::Bridge)
    //            delete giItem;
    //    }
    delete ui;
}

void Form::computePaths() {
    //    usedItems_.clear();
    //    const auto tool {ui->toolHolder->tool()};
    //    if (!tool.isValid()) {
    //        tool.errorMessageBox(this);
    //        return;
    //    }

    //    Paths wPaths;
    //    Paths wRawPaths;
    //    AbstractFile const* file = nullptr;
    //    bool skip {true};

    //    for (auto* gi : App::grView().selectedItems<Gi::Item>()) {
    //        switch (gi->type()) {
    //        case Gi::Type::DataSolid:
    //            wPaths.append(gi->paths());
    //            break;
    //        case Gi::Type::DataPath: {
    //            auto paths = gi->paths();
    //            if (paths.front() == paths.back())
    //                wPaths.append(paths);
    //            else
    //                wRawPaths.append(paths);
    //        } break;
    //            //            if (!file) {
    //            //                file = gi->file();
    //            //                boardSide = file->side();
    //            //            } else if (file != gi->file()) {
    //            //                if (skip) {
    //            //                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
    //            //                        return;
    //            //                }
    //            //            }
    //            //            if (gi->type() == Gi::Type::DataSolid)
    //            //                wPaths.append(gi->paths());
    //            //            else
    //            //                wRawPaths.append(gi->paths());
    //            //            break;
    //        case Gi::Type::ShCircle:
    //        case Gi::Type::ShRectangle:
    //        case Gi::Type::ShText:
    //        case Gi::Type::Drill:
    //            wPaths.append(gi->paths());
    //            break;
    //        case Gi::Type::ShPolyLine:
    //        case Gi::Type::ShCirArc:
    //            wRawPaths.append(gi->paths());
    //            break;
    //        default:
    //            break;
    //        }
    //        addUsedGi(gi);
    //    }

    //    if (wRawPaths.empty() && wPaths.empty()) {
    //        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
    //        return;
    //    }

    //    auto gcp = new GCode::Params;
    //    gcp->setConvent(ui->rbConventional->isChecked());
    //    gcp->setSide(side);
    //    gcp->tools.push_back(tool);
    //    gcp->params[GCode::Params::Depth] = dsbxDepth->value();

    //    gcp->params[Creator::BridgeAlignType] = ui->cbxBridgeAlignType->currentIndex();
    //    gcp->params[Creator::BridgeValue] = ui->dsbxBridgeValue->value();
    //    // NOTE reserve   gcp_.params[Creator::BridgeValue2] = ui->dsbxBridgeValue->value();

    //    if (side == GCode::On)
    //        gcp->params[Creator::TrimmingOpenPaths] = ui->cbxTrimming->isChecked();
    //    else
    //        gcp->params[Creator::TrimmingCorners] = ui->cbxTrimming->isChecked();

    //    gcp->params[GCode::Params::GrItems].setValue(usedItems_);

    //    QPolygonF brv;
    //    for (QGraphicsItem* item : App::grView().items()) {
    //        if (item->type() == Gi::Type::Bridge)
    //            brv.push_back(item->pos());
    //    }
    //    if (!brv.isEmpty()) {
    //        // gcp_.params[GCode::Params::Bridges].fromValue(brv);
    //        gcp->params[Creator::BridgeLen] = ui->dsbxBridgeLenght->value();
    //    }

    //    gcp->closedPaths = std::move(wPaths);
    //    gcp->openPaths = std::move(wRawPaths);
    //    fileCount = 1;
    //    emit createToolpath(gcp);
}

void Form::updateName() {
    leName->setText(names[side]);
}

void Form::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void Form::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void Form::updatePixmap() {
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[side + direction * 3]).pixmap(QSize(size, size)));
}

void Form::rb_clicked() {
    //    if (ui->rbOn->isChecked()) {
    //        side = GCode::On;
    //        ui->cbxTrimming->setText(tr("Trimming"));
    //        ui->cbxTrimming->setChecked(trimming_ & Trimming::Line);
    //    } else if (ui->rbOutside->isChecked()) {
    //        side = GCode::Outer;
    //        ui->cbxTrimming->setText(tr("Corner Trimming"));
    //        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    //    } else if (ui->rbInside->isChecked()) {
    //        side = GCode::Inner;
    //        ui->cbxTrimming->setText(tr("Corner Trimming"));
    //        ui->cbxTrimming->setChecked(trimming_ & Trimming::Corner);
    //    }

    //    if (ui->rbClimb->isChecked())
    //        direction = GCode::Climb;
    //    else if (ui->rbConventional->isChecked())
    //        direction = GCode::Conventional;

    //    updateName();
    //    updateButtonIconSize();

    //    updatePixmap();
}

void Form::updateBridgePos(QPointF pos) {
    if(GiBridge::moveBrPtr)
        GiBridge::moveBrPtr->setPos(pos);
}

void Form::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void Form::editFile(GCode::File* file) {

    //    GCode::Params gcp_ {file->gcp()};

    //    fileId = gcp_.fileId;
    //    editMode_ = true;

    //    { // GUI
    //        side = gcp_.side();
    //        direction = static_cast<GCode::Direction>(gcp_.convent());
    //        ui->toolHolder->setTool(gcp_.tools.front());
    //        dsbxDepth->setValue(gcp_.params[GCode::Params::Depth].toDouble());

    //        switch (side) {
    //        case GCode::On:
    //            ui->rbOn->setChecked(true);
    //            break;
    //        case GCode::Outer:
    //            ui->rbOutside->setChecked(true);
    //            break;
    //        case GCode::Inner:
    //            ui->rbInside->setChecked(true);
    //            break;
    //        }

    //        switch (direction) {
    //        case GCode::Climb:
    //            ui->rbClimb->setChecked(true);
    //            break;
    //        case GCode::Conventional:
    //            ui->rbConventional->setChecked(true);
    //            break;
    //        }
    //    }

    //    { // GrItems
    //        usedItems_.clear();
    //        auto items {gcp_.params[GCode::Params::GrItems].value<UsedItems>()};

    //        auto i = items.cbegin();
    //        while (i != items.cend()) {

    //            //            auto [_fileId, _] = i.key();
    //            //            Q_UNUSED(_)
    //            //            App::project().file(_fileId)->itemGroup()->setSelected(i.value());
    //            //            ++i;
    //        }
    //    }

    //    { // Bridges
    //        if (gcp_.params.contains(GCode::Params::Bridges)) {
    //            ui->dsbxBridgeLenght->setValue(gcp_.params[GCode::Params::BridgeLen].toDouble());
    //            //            for (auto& pos : gcp_.params[GCode::Params::Bridges].value<QPolygonF>()) {
    //            //                brItem = new BridgeItem{lenght_, size_, side, brItem};
    //            //                 App::grView().addItem(brItem);
    //            //                brItem->setPos(pos);
    //            //                brItem->lastPos_ = pos;
    //            //            }
    //            updateBridge();
    //            brItem = new GiBridge{lenght_, size_, side, brItem};
    //            //        delete item;
    //        }
    //    }
}

} // namespace Thread

#include "moc_thread_form.cpp"

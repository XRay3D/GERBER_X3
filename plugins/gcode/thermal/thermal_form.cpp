/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thermal_form.h"
#include "ui_thermalform.h"

#include "thermal.h"
#include "thermal_delegate.h"
#include "thermal_model.h"
#include "thermal_node.h"
#include "thermal_previewitem.h"

#include "graphicsview.h"

namespace Thermal {

enum {
    Size = 24
};

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new Ui::ThermalForm) {
    ui->setupUi(content);
    ui->treeView->setIconSize(QSize(Size, Size));

    grid->setRowStretch(2, 1);
    grid->setRowStretch(7, 0);

    MySettings settings;
    settings.beginGroup("Thermal");
    settings.getValue(par.angle, "angle", 0.0);
    settings.getValue(par.count, "count", 4);
    settings.getValue(par.tickness, "tickness", 0.5);
    lastMax = settings.getValue(ui->dsbxAreaMax, 10.0);
    lastMin = settings.getValue(ui->dsbxAreaMin);
    settings.getValue(ui->chbxIgnoreCopper);
    settings.getValue(ui->chbxAperture, true);
    settings.getValue(ui->chbxPath);
    settings.getValue(ui->chbxPour);
    settings.endGroup();
    setWindowTitle(tr("Thermal Insulation Toolpath"));

    connect(leName, &QLineEdit::textChanged, this, &Form::onNameTextChanged);
    connect(ui->chbxAperture, &QCheckBox::toggled, this, &Form::updateThermalGi);
    connect(ui->chbxPath, &QCheckBox::toggled, this, &Form::updateThermalGi);
    connect(ui->chbxPour, &QCheckBox::toggled, this, &Form::updateThermalGi);
    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &Form::updateName);
    connect(ui->treeView, &QTreeView::clicked, ui->treeView, qOverload<const QModelIndex&>(&QTreeView::edit));

    connect(ui->dsbxAreaMin, &QDoubleSpinBox::editingFinished, this, &Form::onDsbxAreaMinEditingFinished);
    connect(ui->dsbxAreaMax, &QDoubleSpinBox::editingFinished, this, &Form::onDsbxAreaMaxEditingFinished);

    updateName();
    updateButtonIconSize();

    if(0) {
        chbx = new QCheckBox{"", ui->treeView};
        chbx->setMinimumHeight(ui->treeView->header()->height() - 4);
        chbx->setEnabled(false);
        auto lay = new QGridLayout{ui->treeView->header()};
        lay->addWidget(chbx, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        lay->setContentsMargins(3, 0, 0, 0);
    }

    updateFiles();

    if(ui->cbxFile->count() < 1)
        return;

    ui->treeView->setUniformRowHeights(true);
    ui->treeView->header()->setMinimumHeight(Size);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setStretchLastSection(false);
    //    ui->treeView->hideColumn(1);
    ui->treeView->setItemDelegate(new Delegate{this});
}

Form::~Form() {
    MySettings settings;
    settings.beginGroup("Thermal");
    if(model && model->data_.size()) {
        settings.setValue(model->thParam().angle, "angle");
        settings.setValue(model->thParam().count, "count");
        settings.setValue(model->thParam().tickness, "tickness");
    }
    settings.setValue(ui->dsbxAreaMax);
    settings.setValue(ui->dsbxAreaMin);
    settings.setValue(ui->chbxIgnoreCopper);
    settings.setValue(ui->chbxAperture);
    settings.setValue(ui->chbxPath);
    settings.setValue(ui->chbxPour);
    settings.endGroup();
    delete ui;
}

void Form::updateFiles() {
    disconnect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &Form::updateThermalGi);
    ui->cbxFile->clear();

    updateCriterias();
    for(auto file: App::project().files()) {
        auto gos = file->getDataForGC(criterias, GCType::Profile, true);
        if(gos.size())
            ui->cbxFile->addItem(file->icon(), file->shortName(), QVariant::fromValue(file));
    }

    if(ui->cbxFile->count())
        updateThermalGi();

    widget()->setEnabled(ui->cbxFile->count());

    connect(ui->cbxFile, &QComboBox::currentIndexChanged, this, &Form::updateThermalGi);
}

void Form::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void Form::computePaths() {
    qDebug(__FUNCTION__);
    if(!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    auto gpc = new GCode::Params;

    for(auto& item: items_) {
        if(item->isValid()) {
            gpc->closedPaths += item->paths();
            if(Paths bridge = item->bridge(); bridge.size())
                gpc->supportPathss.emplace_back(item->bridge());
        }
    }

    gpc->setConvent(true);
    gpc->setSide(GCode::Outer);
    gpc->tools.push_back(tool);
    gpc->params[GCode::Params::Depth] = dsbxDepth->value();
    gpc->params[Creator::FileId] = ui->cbxFile->currentData().value<AbstractFile*>()->id();
    gpc->params[Creator::IgnoreCopper] = ui->chbxIgnoreCopper->isChecked();
    fileCount = 1;
    emit createToolpath(gpc);
}

void Form::updateName() {
    tool = ui->toolHolder->tool();
    leName->setText(tr("Thermal"));
    redraw();
}

void Form::hideEvent(QHideEvent* event) { // NOTE clean and hide pr gi
    delete ui->treeView->model();
    model = nullptr;
    items_.clear();
    event->accept();
}

void Form::updateThermalGi() {
    auto file = ui->cbxFile->currentData(Qt::UserRole).value<AbstractFile*>();
    if(!file)
        return;

    if(model)
        delete ui->treeView->model();

    model = new Model{ui->treeView};
    model->appendRow(QIcon(), tr("All"), par);
    boardSide = file->side();

    updateCriterias();
    thPaths.clear();
    for(auto&& var: file->getDataForGC(criterias, GCType::Profile))
        thPaths[var.name].emplace_back(var.fill, var.pos);

    int count = std::accumulate(thPaths.begin(), thPaths.end(),
        0, [](int i, auto& val) { return i + int(val.second.size()); });

    QProgressDialog pd("create th", "", 0, count, this);
    pd.setCancelButton(nullptr);
    count = 0;
    { // create Preview Items
        QColor color{App::settings().theme() > LightRed ? Qt::white : Qt::black};

        items_.clear();
        for(const auto& [keyId, valVec]: thPaths) {
            if(valVec.empty())
                continue;
            auto node = model->appendRow(drawIcon(valVec.front().first, color), keyId, par);
            for(const auto& [paths, pos]: valVec) {
                auto tprItem = items_.emplace_back(std::make_shared<PreviewItem>(paths, pos, tool));
                tprItem->setVisible(true);
                tprItem->setOpacity(1.0);
                node->append(new Node{drawIcon(paths), "", par, pos, tprItem.get(), model});
            }
            qApp->processEvents();
            pd.setValue(++count);
        }

        for(auto& item: items_) {
            App::grView().addItem(item.get());
            connect(item.get(), &AbstractThermPrGi::selectionChanged, this, &Form::setSelection);
        }
    }

    ui->treeView->setModel(model);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &Form::onSelectionChanged);
    if(0 && App::isDebug())
        ui->treeView->expandAll();
}

void Form::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    for(const auto& index: selected.indexes()) {
        auto* node = static_cast<Node*>(index.internalPointer());
        auto* item = node->item();
        if(item)
            item->setSelected(true);
        else
            for(int i = 0; i < node->childCount(); ++i)
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    for(const auto& index: deselected.indexes()) {
        auto* node = static_cast<Node*>(index.internalPointer());
        auto* item = node->item();
        if(item)
            item->setSelected(false);
        else
            for(int i = 0; i < node->childCount(); ++i)
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
    }
}

void Form::setSelection(const QModelIndex& selected, const QModelIndex& deselected) {
    if(selected.isValid())
        ui->treeView->selectionModel()->select(selected, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    if(deselected.isValid())
        ui->treeView->selectionModel()->select(deselected, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}

void Form::updateCriterias() {
    criterias.clear();
    if(ui->chbxAperture->isChecked())
        criterias.emplace_back(
            std::vector{GraphicObject::FlStamp} //
        );
    if(ui->chbxPath->isChecked())
        criterias.emplace_back(
            std::vector{GraphicObject::Line, GraphicObject::PolyLine},
            Range{ui->dsbxAreaMin->value() * uScale * uScale, ui->dsbxAreaMax->value() * uScale * uScale} //
        );
    if(ui->chbxPour->isChecked())
        criterias.emplace_back(
            std::vector{GraphicObject::Polygon, GraphicObject::Composite},
            Range{ui->dsbxAreaMin->value() * uScale * uScale, ui->dsbxAreaMax->value() * uScale * uScale} //
        );
}

void Form::redraw() {
    for(auto item: items_) item->redraw();
}

void Form::onDsbxDepthValueChanged(double arg1) {
    depth_ = arg1;
    redraw();
}

void Form::editFile(GCode::File* /*file*/) { }

void Form::onDsbxAreaMinEditingFinished() {
    if(lastMin != ui->dsbxAreaMin->value()) // skip if dsbxAreaMin hasn't changed
        lastMin = ui->dsbxAreaMin->value(), updateThermalGi();
}

void Form::onDsbxAreaMaxEditingFinished() {
    if(lastMax != ui->dsbxAreaMax->value()) // skip if dsbAreaMax hasn't changed
        lastMax = ui->dsbxAreaMax->value(), updateThermalGi();
}

} // namespace Thermal

#include "moc_thermal_form.cpp"

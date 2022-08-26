// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thermal_form.h"
#include "thermal.h"
#include "thermal_delegate.h"
#include "thermal_model.h"
#include "thermal_node.h"
#include "thermal_previewitem.h"
#include "ui_thermalform.h"

#include "graphicsview.h"
#include "myclipper.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include "tool_pch.h"

#include <QCheckBox>
#include <QDockWidget>
#include <QFuture>
#include <QItemSelection>
#include <QMessageBox>
#include <QPicture>
#include <QProgressDialog>
#include <QTimer>

enum { Size = 24 };

extern QIcon drawApertureIcon(Gerber::AbstractAperture* aperture);

ThermalForm::ThermalForm(GCodePlugin* plugin, QWidget* parent)
    : FormsUtil(plugin, new GCode::ThermalCreator, parent)
    , ui(new Ui::ThermalForm) {
    ui->setupUi(content);

    MySettings settings;
    settings.beginGroup("ThermalForm");
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
    label->setText(tr("Thermal Insulation Toolpath"));
    /*parent->*/ setWindowTitle(label->text());

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({16, 16});

    connect(pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(pbCreate, &QPushButton::clicked, this, &ThermalForm::createFile);
    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &ThermalForm::updateName);

    ui->treeView->setIconSize(QSize(Size, Size));
    connect(ui->treeView, &QTreeView::clicked, ui->treeView, qOverload<const QModelIndex&>(&QTreeView::edit));

    connect(ui->chbxAperture, &QCheckBox::toggled, [this] { createTPI(nullptr); });
    connect(ui->chbxPath, &QCheckBox::toggled, [this] { createTPI(nullptr); });
    connect(ui->chbxPour, &QCheckBox::toggled, [this] { createTPI(nullptr); });

    updateName();

    if (0) {
        chbx = new QCheckBox("", ui->treeView);
        chbx->setMinimumHeight(ui->treeView->header()->height() - 4);
        chbx->setEnabled(false);
        auto lay = new QGridLayout(ui->treeView->header());
        lay->addWidget(chbx, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
        lay->setContentsMargins(3, 0, 0, 0);
    }

    updateFiles();

    if (ui->cbxFile->count() < 1)
        return;

    ui->treeView->setUniformRowHeights(true);
    ui->treeView->header()->setMinimumHeight(Size);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setStretchLastSection(false);
    ui->treeView->hideColumn(1);
    ui->treeView->setItemDelegate(new ThermalDelegate(this));
    {
        ui->treeView->setIconSize(QSize(24, 24));
        const int w = ui->treeView->indentation();
        const int h = Size;
        QImage i(w, h, QImage::Format_ARGB32);
        QPainter p(&i);
        p.setPen(QColor(128, 128, 128));
        // │
        i.fill(Qt::transparent);
        p.drawLine(w >> 1, /**/ 0, w >> 1, /**/ h);
        i.save("settings/vline.png", "PNG");
        // ├─
        p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
        i.save("settings/branch-more.png", "PNG");
        // └─
        i.fill(Qt::transparent);
        p.drawLine(w >> 1, /**/ 0, w >> 1, h >> 1);
        p.drawLine(w >> 1, h >> 1, /**/ w, h >> 1);
        i.save("settings/branch-end.png", "PNG");
        QFile file(":/qtreeviewstylesheet/QTreeView.qss");
        file.open(QFile::ReadOnly);
        ui->treeView->setStyleSheet(file.readAll());
        ui->treeView->header()->setMinimumHeight(h);
    }
}

ThermalForm::~ThermalForm() {
    MySettings settings;
    settings.beginGroup("ThermalForm");
    if (model && model->data_.size()) {
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

void ThermalForm::updateFiles() {
    disconnect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);

    ui->cbxFile->clear();

    for (auto file : App::project()->files(FileType::Gerber))
        App::filePlugin(int(file->type()))->addToGcForm(file, ui->cbxFile);
    qDebug() << ui->cbxFile->count();
    on_cbxFileCurrentIndexChanged(0);

    connect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
}

bool ThermalForm::canToShow() {
    QComboBox cbx;
    for (auto file : App::project()->files(FileType::Gerber)) {
        App::filePlugin(int(file->type()))->addToGcForm(file, &cbx);
        if (cbx.count())
            return true;
    }

    QMessageBox::information(nullptr, "", tr("No data to process."));
    return false;
}

void ThermalForm::on_leName_textChanged(const QString& arg1) { fileName_ = arg1; }

void ThermalForm::createFile() {
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Pathss wBridgePaths;

    for (auto& item : items_) {
        if (item->isValid()) {
            wPaths.append(item->paths());
            wBridgePaths.push_back(item->bridge());
        }
    }

    GCode::GCodeParams gpc;
    gpc.setConvent(true);
    gpc.setSide(GCode::Outer);
    gpc.tools.push_back(tool);
    gpc.params[GCode::GCodeParams::Depth] = dsbxDepth->value();
    gpc.params[GCode::GCodeParams::FileId] = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>())->id();
    gpc.params[GCode::GCodeParams::IgnoreCopper] = ui->chbxIgnoreCopper->isChecked();
    creator->setGcp(gpc);
    creator->addPaths(wPaths);
    creator->addSupportPaths(wBridgePaths);
    fileCount = 1;
    emit createToolpath();
}

void ThermalForm::updateName() {
    tool = ui->toolHolder->tool();
    leName->setText(tr("Thermal"));
    redraw();
}

void ThermalForm::on_cbxFileCurrentIndexChanged(int /*index*/) {
    FileInterface* file = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>());
    createTPI(file);
}

void ThermalForm::createTPI(FileInterface* file) {
    if (!file)
        file = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>());
    items_.clear();

    if (model)
        delete ui->treeView->model();

    model = new ThermalModel(ui->treeView);
    model->appendRow(QIcon(), tr("All"), par);

    boardSide = file->side();

    ThParam2 tp2 {
        ui->chbxAperture->isChecked(),
        ui->chbxPath->isChecked(),
        ui->chbxPour->isChecked(),
        ui->dsbxAreaMax->value() * uScale * uScale,
        ui->dsbxAreaMin->value() * uScale * uScale};

    // FIXME     auto thPaths = App::filePlugin(int(file->type()))->createThermalPreviewGi(file, tp2);

    int count {};
    int ctr {};
    // FIXME     for (const auto& [key, val] : thPaths)
    //        count += val.size();

    //    QProgressDialog pd("create th", "", 0, count, this);
    //    pd.setCancelButton(nullptr);

    //    for (const auto& [key1, val] : thPaths) {
    //        for (const auto& [key, val] : val) {
    //            if (!val.size())
    //                continue;
    //            auto node = model->appendRow(drawIcon(*val.front().first), key, par);
    //            for (const auto& [paths, pos] : val) {
    //                items_.emplace_back(std::make_shared<ThermalPreviewItem>(paths, pos, tool));
    //                node->append(new ThermalNode(drawIcon(*paths), "", par, pos, items_.back().get(), model));
    //            }
    //            qApp->processEvents();
    //            pd.setValue(++ctr);
    //        }
    //    }

    for (auto& item : items_) {
        App::scene()->addItem(item.get());
        connect(item.get(), &AbstractThermPrGi::selectionChanged, this, &ThermalForm::setSelection);
    }

    ui->treeView->setModel(model);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ThermalForm::onSelectionChanged);
    if (0 && qApp->applicationDirPath().contains("GERBER_X3/bin"))
        ui->treeView->expandAll();
}

void ThermalForm::onSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected) {
    for (const auto& index : selected.indexes()) {
        auto* node = static_cast<ThermalNode*>(index.internalPointer());
        auto* item = node->item();
        if (item)
            item->setSelected(true);
        else {
            for (int i = 0; i < node->childCount(); ++i) {
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }
    for (const auto& index : deselected.indexes()) {
        auto* node = static_cast<ThermalNode*>(index.internalPointer());
        auto* item = node->item();
        if (item)
            item->setSelected(false);
        else {
            for (int i = 0; i < node->childCount(); ++i) {
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
            }
        }
    }
}

void ThermalForm::setSelection(const QModelIndex& selected, const QModelIndex& deselected) {
    if (selected.isValid())
        ui->treeView->selectionModel()->select(selected, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    if (deselected.isValid())
        ui->treeView->selectionModel()->select(deselected, QItemSelectionModel::Deselect | QItemSelectionModel::Rows);
}

void ThermalForm::redraw() {
    for (auto item : items_)
        item->redraw();
}

void ThermalForm::on_dsbxDepth_valueChanged(double arg1) {
    depth_ = arg1;
    redraw();
}

void ThermalForm::editFile(GCode::File* /*file*/) { }

void ThermalForm::on_dsbxAreaMin_editingFinished() {
    if (lastMin != ui->dsbxAreaMin->value()) { // skip if dsbxAreaMin hasn't changed
        lastMin = ui->dsbxAreaMin->value();
        createTPI(nullptr);
    }
}

void ThermalForm::on_dsbxAreaMax_editingFinished() {
    if (lastMax != ui->dsbxAreaMax->value()) { // skip if dsbAreaMax hasn't changed
        lastMax = ui->dsbxAreaMax->value();
        createTPI(nullptr);
    }
}

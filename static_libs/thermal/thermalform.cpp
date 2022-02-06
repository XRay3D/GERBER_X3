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
#include "thermalform.h"
#include "ui_thermalform.h"

#include "graphicsview.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include "thermaldelegate.h"
#include "thermalmodel.h"
#include "thermalnode.h"
#include "thermalpreviewitem.h"
#include "toolpch.h"
#include <myclipper.h>

#include <QCheckBox>
#include <QDockWidget>
#include <QFuture>
#include <QItemSelection>
#include <QMessageBox>
#include <QPicture>
#include <QTimer>

enum { Size = 24 };

extern QIcon drawApertureIcon(Gerber::AbstractAperture* aperture);

ThermalForm::ThermalForm(QWidget* parent)
    : FormsUtil(new GCode::ThermalCreator, parent)
    , ui(new Ui::ThermalForm) {
    ui->setupUi(this);

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

    parent->setWindowTitle(ui->label->text());

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    connect(ui->pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(ui->pbCreate, &QPushButton::clicked, this, &ThermalForm::createFile);
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

#ifdef GERBER
    par = model->rootItem->child(0)->getParam();
#endif
    MySettings settings;
    settings.beginGroup("ThermalForm");
    if (model) {
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
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    disconnect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
#else
    disconnect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
#endif
    ui->cbxFile->clear();

    for (auto file : App::project()->files(FileType::Gerber))
        App::filePlugin(int(file->type()))->addToDrillForm(file, ui->cbxFile);
    qDebug() << ui->cbxFile->count();
    on_cbxFileCurrentIndexChanged(0);
#if QT_VERSION < QT_VERSION_CHECK(5, 15, 0)
    connect(ui->cbxFile, qOverload<int>(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
#else
    connect(ui->cbxFile, qOverload<int /*, const QString&*/>(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
#endif
}

bool ThermalForm::canToShow() {
    QComboBox cbx;
    for (auto file : App::project()->files(FileType::Gerber)) {
        App::filePlugin(int(file->type()))->addToDrillForm(file, &cbx);
        if (cbx.count())
            return true;
    }

    QMessageBox::information(nullptr, "", tr("No data to process."));
    return false;
}

void ThermalForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void ThermalForm::createFile() {
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Pathss wBridgePaths;

    for (auto& item : m_sourcePreview) {
        if (item->isValid()) {
            wPaths.append(item->paths());
            wBridgePaths.push_back(item->bridge());
        }
    }

    GCode::GCodeParams gpc;
    gpc.setConvent(true);
    gpc.setSide(GCode::Outer);
    gpc.tools.push_back(tool);
    gpc.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    gpc.params[GCode::GCodeParams::FileId] = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>())->id();
    gpc.params[GCode::GCodeParams::IgnoreCopper] = ui->chbxIgnoreCopper->isChecked();
    m_tpc->setGcp(gpc);
    m_tpc->addPaths(wPaths);
    m_tpc->addSupportPaths(wBridgePaths);
    fileCount = 1;
    emit createToolpath();
}

void ThermalForm::updateName() {
    tool = ui->toolHolder->tool();
    ui->leName->setText(tr("Thermal"));
    redraw();
}

void ThermalForm::on_cbxFileCurrentIndexChanged(int /*index*/) {
    FileInterface* file = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>());
    createTPI(file);
}

void ThermalForm::createTPI(FileInterface* file) {
    if (!file)
        file = static_cast<FileInterface*>(ui->cbxFile->currentData().value<void*>());
    m_sourcePreview.clear();

    if (model)
        delete ui->treeView->model();

    model = new ThermalModel(ui->treeView);
    boardSide = file->side();

    ThParam2 tp2 {
        par,
        model,
        ui->chbxAperture->isChecked(),
        ui->chbxPath->isChecked(),
        ui->chbxPour->isChecked(),
        ui->dsbxAreaMax->value() * uScale * uScale,
        ui->dsbxAreaMin->value() * uScale * uScale
    };

    m_sourcePreview = App::filePlugin(int(file->type()))->createThermalPreviewGi(file, tp2, tool);

    for (auto& item : m_sourcePreview) {
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
    for (auto item : m_sourcePreview) {
        item->redraw();
    }
}

void ThermalForm::on_dsbxDepth_valueChanged(double arg1) {
    m_depth = arg1;
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

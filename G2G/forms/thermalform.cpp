#include "thermalform.h"
#include "ui_thermalform.h"

#include "gcode/gcfile.h"
#include "gcodepropertiesform.h"
#include "gi/bridgeitem.h"
#include "project.h"
#include "thermalmodel.h"
#include "thermalpreviewitem.h"
#include "tooldatabase/tooldatabase.h"
#include "tooldatabase/tooleditdialog.h"
#include <QCheckBox>
#include <QDockWidget>
#include <QMessageBox>
#include <QPicture>
#include <QTimer>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

enum { Size = 24 };

extern QIcon drawApertureIcon(Gerber::AbstractAperture* aperture);

QIcon drawRegionIcon(const Gerber::GraphicObject& go)
{
    QPainterPath painterPath;

    for (QPolygonF& polygon : toQPolygons(go.paths()))
        painterPath.addPolygon(polygon);

    const QRectF rect = painterPath.boundingRect();

    qreal scale = (double)Size / qMax(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (Size - rect.height() * scale) / 2;
    else
        kx -= (Size - rect.width() * scale) / 2;

    QPixmap pixmap(Size, Size);
    pixmap.fill(Qt::transparent);
    QPainter painter;
    painter.begin(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);
    painter.setPen(Qt::NoPen);
    painter.setBrush(Qt::black);
    //    painter.translate(tr);
    painter.translate(-kx, ky);
    painter.scale(scale, -scale);
    painter.drawPath(painterPath);
    return QIcon(pixmap);
}

ThermalForm::ThermalForm(QWidget* parent)
    : FormsUtil("ThermalForm", parent)
    , ui(new Ui::ThermalForm)
{
    ui->setupUi(this);

    ui->treeView->setIconSize(QSize(Size, Size));

    ui->lblToolName->setText(tool.name());
    ui->dsbxDepth->setValue(GCodePropertiesForm::thickness);

    updateName();

    //    QSettings settings;
    //    settings.beginGroup("ProfileForm");
    //    settings.endGroup();

    lay = new QGridLayout(ui->treeView->header());
    cbx = new QCheckBox("", ui->treeView);
    lay->addWidget(cbx, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    lay->setContentsMargins(3, 0, 0, 0);
    cbx->setMinimumHeight(ui->treeView->header()->height() - 4);
    connect(cbx, &QCheckBox::toggled, [=](bool /*checked*/) {
        //        model->setCreate(checked);
        //        updateCreateButton();
    });

    updateFiles();

    ui->pbEdit->setIcon(Icon(ButtonEditIcon));
    ui->pbSelect->setIcon(Icon(ButtonSelectIcon));
    ui->pbClose->setIcon(Icon(ButtonCloseIcon));
    ui->pbCreate->setIcon(Icon(ButtonCreateIcon));
    parent->setWindowTitle(ui->label->text());

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    QSettings settings;
    settings.beginGroup("ThermalForm");
    ui->dsbxDepth->setValue(settings.value("dsbxDepth").toDouble());
    if (settings.value("rbBoard").toBool())
        ui->dsbxDepth->rbBoard->setChecked(true);
    if (settings.value("rbCopper").toBool())
        ui->dsbxDepth->rbCopper->setChecked(true);
    settings.endGroup();
}

ThermalForm::~ThermalForm()
{
    m_sourcePreview.clear();
    QSettings settings;
    settings.beginGroup("ThermalForm");
    settings.setValue("dsbxDepth", ui->dsbxDepth->value(true));
    settings.setValue("rbBoard", ui->dsbxDepth->rbBoard->isChecked());
    settings.setValue("rbCopper", ui->dsbxDepth->rbCopper->isChecked());
    settings.endGroup();
    delete ui;
}

void ThermalForm::updateFiles()
{
    disconnect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);

    ui->cbxFile->clear();

    for (Gerber::File* file : Project::files<Gerber::File>()) {
        if (file->flashedApertures()) {
            ui->cbxFile->addItem(file->shortName(), QVariant::fromValue(static_cast<void*>(file)));
            QPixmap pixmap(Size, Size);
            QColor color(file->color());
            color.setAlpha(255);
            pixmap.fill(color);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QIcon(pixmap), Qt::DecorationRole);
            ui->cbxFile->setItemData(ui->cbxFile->count() - 1, QSize(0, Size), Qt::SizeHintRole);
        }
    }

    if (!ui->cbxFile->count()) {
        QMessageBox::information(this, "", tr("No data to process."));
        QTimer::singleShot(1, Qt::CoarseTimer, [=] { on_pbClose_clicked(); });
    } else
        on_cbxFileCurrentIndexChanged(0);

    connect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
}

void ThermalForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->lblToolName->setText(tool.name());
        updateName();
        redraw();
    }
}

void ThermalForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.setTool(tool);
    if (d.exec()) {
        tool = d.tool();
        tool.setId(-1);
        ui->lblToolName->setText(tool.name());
        updateName();
        redraw();
    }
}

void ThermalForm::on_pbCreate_clicked() { create(); }

void ThermalForm::on_pbClose_clicked() { static_cast<QWidget*>(parent())->close(); }

void ThermalForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void ThermalForm::create()
{
    //    Scene* scene = Scene::self;

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Pathss wBridgePaths;

    for (QSharedPointer<ThermalPreviewItem> item : m_sourcePreview) {
        if (item->flags() & QGraphicsItem::ItemIsSelectable) {
            wPaths.append(item->paths());
            wBridgePaths.append(item->bridge());
        }
    }

    //    for (QGraphicsItem* item : scene->selectedItems()) {
    //        if (item->type() == GerberItemType) {
    //            GerberItem* gi = static_cast<GerberItem*>(item);
    //            if (!file) {
    //                file = gi->file();
    //                boardSide = gi->file()->side();
    //            }
    //            if (file != gi->file()) {
    //                QMessageBox::warning(this, "", tr("Working items from different files!"));
    //                return;
    //            }
    //        }
    //        if (item->type() == RawItemType) {
    //            RawItem* gi = static_cast<RawItem*>(item);
    //            if (!file) {
    //                file = gi->file();
    //                boardSide = gi->file()->side();
    //            }
    //            if (file != gi->file()) {
    //                QMessageBox::warning(this, "", tr("Working items from different files!"));
    //                return;
    //            }
    //        }
    //        if (item->type() == GerberItemType)
    //            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //        if (item->type() == DrillItemType) {
    //            //            if (static_cast<DrillItem*>(item)->isSlot())
    //            //                wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //            //            else
    //            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //        }
    //        if (item->type() == RawItemType)
    //            wRawPaths.append(static_cast<GraphicsItem*>(item)->paths());
    //    }

    //    if (wPaths.isEmpty() && wRawPaths.isEmpty()) {
    //        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
    //        return;
    //    }

    GCode::Creator* tps = toolPathCreator(wPaths, true, side);
    tps->addSupportPaths(wBridgePaths);
    connect(this, &ThermalForm::createThermal, tps, &GCode::Creator::createThermal);
    emit createThermal(static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>()), tool, ui->dsbxDepth->value());
    showProgress();
    //progress(1, 0);
}

void ThermalForm::updateName()
{
    ui->leName->setText(tr("Thermal"));
}

void ThermalForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    setApertures(static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>())->apertures());
}

void ThermalForm::setApertures(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value)
{
    m_sourcePreview.clear();

    m_apertures = *value;
    model = new ThermalModel(this);
    const Gerber::File* file = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>());
    boardSide = file->side();

    ThermalNode* thermalNode = model->appendRow(QIcon(), "Regions");
    for (const Gerber::GraphicObject& go : *file) {
        if (go.state().type() == Gerber::Region && go.state().imgPolarity() == Gerber::Positive) {
            ThermalPreviewItem* item = new ThermalPreviewItem(go, tool, m_depth);
            item->setToolTip("Region");
            m_sourcePreview.append(QSharedPointer<ThermalPreviewItem>(item));
            Scene::addItem(item);
            thermalNode->append(new ThermalNode(drawRegionIcon(go), "Region", 0.0, 0.5, 4, go.state().curPos(), item));
        }
    }

    thermalNode = model->appendRow(QIcon(), "Lines");
    for (const Gerber::GraphicObject& go : *file) {
        if (go.state().type() == Gerber::Line && go.state().imgPolarity() == Gerber::Positive && go.path().size() == 2 && Length(go.path().first(), go.path().last()) * dScale * 0.3 < m_apertures[go.state().aperture()]->minSize()) {
            ThermalPreviewItem* item = new ThermalPreviewItem(go, tool, m_depth);
            item->setToolTip("Line");
            m_sourcePreview.append(QSharedPointer<ThermalPreviewItem>(item));
            Scene::addItem(item);
            thermalNode->append(new ThermalNode(drawRegionIcon(go), "Line", 0.0, 0.5, 4, go.state().curPos(), item));
        }
    }

    QMap<int, QSharedPointer<Gerber::AbstractAperture>>::const_iterator apertureIt;
    for (apertureIt = m_apertures.begin(); apertureIt != m_apertures.end(); ++apertureIt) {
        if (apertureIt.value()->isFlashed()) {
            QString name(apertureIt.value()->name());
            ThermalNode* thermalNode = model->appendRow(drawApertureIcon(apertureIt.value().data()), name);
            for (const Gerber::GraphicObject& go : *file) {
                qDebug() << go.state().dCode();
                if (go.state().dCode() == Gerber::D03 && go.state().aperture() == apertureIt.key()) {
                    ThermalPreviewItem* item = new ThermalPreviewItem(go, tool, m_depth);
                    item->setToolTip(name);
                    m_sourcePreview.append(QSharedPointer<ThermalPreviewItem>(item));
                    Scene::addItem(item);
                    thermalNode->append(new ThermalNode(drawRegionIcon(go), name, 0.0, 0.5, 4, go.state().curPos(), item));
                }
            }
        }
    }

    //    updateCreateButton();

    delete ui->treeView->model();
    ui->treeView->setModel(model);
    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setStretchLastSection(false);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ThermalForm::on_selectionChanged);
}

void ThermalForm::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    for (QModelIndex index : selected.indexes()) {
        ThermalNode* node = reinterpret_cast<ThermalNode*>(index.internalPointer());
        ThermalPreviewItem* item = node->item();
        if (item)
            item->setSelected(true);
        else {
            for (int i = 0; i < node->childCount(); ++i) {
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Select | QItemSelectionModel::Rows);
            }
        }
    }
    for (QModelIndex index : deselected.indexes()) {
        ThermalNode* node = reinterpret_cast<ThermalNode*>(index.internalPointer());
        ThermalPreviewItem* item = node->item();
        if (item)
            item->setSelected(false);
        else {
            for (int i = 0; i < node->childCount(); ++i) {
                ui->treeView->selectionModel()->select(model->createIndex(i, 0, node->child(i)), QItemSelectionModel::Clear | QItemSelectionModel::Rows);
            }
        }
    }
}

void ThermalForm::redraw()
{
    for (QSharedPointer<ThermalPreviewItem> item : m_sourcePreview) {
        item->redraw();
    }
}

void ThermalForm::on_dsbxDepth_valueChanged(double arg1)
{
    m_depth = arg1;
    redraw();
}

void ThermalForm::on_pbExclude_clicked()
{
    for (const QSharedPointer<ThermalPreviewItem>& item : m_sourcePreview) {
        if (item->node() && !item->isSelected())
            item->node()->disable();
    }
    ui->treeView->update();
}

void ThermalForm::editFile(GCode::File* /*file*/)
{
}

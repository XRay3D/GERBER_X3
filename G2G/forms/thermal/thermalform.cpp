#include "thermalform.h"
#include "ui_thermalform.h"

#include "../gcodepropertiesform.h"
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
#include <gcfile.h>
#include <gcthermal.h>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

enum { Size = 24 };

extern QIcon drawApertureIcon(Gerber::AbstractAperture* aperture);

QIcon drawRegionIcon(const Gerber::GraphicObject& go)
{
    static QMutex m;
    QMutexLocker l(&m);

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
    : FormsUtil("ThermalForm", new GCode::ThermalCreator, parent)
    , ui(new Ui::ThermalForm)
{
    ui->setupUi(this);

    ui->treeView->setIconSize(QSize(Size, Size));

    ui->lblToolName->setText(tool.name());
    ui->dsbxDepth->setValue(GCodePropertiesForm::thickness);

    updateName();

    lay = new QGridLayout(ui->treeView->header());
    cbx = new QCheckBox("", ui->treeView);
    lay->addWidget(cbx, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    lay->setContentsMargins(3, 0, 0, 0);
    cbx->setMinimumHeight(ui->treeView->header()->height() - 4);

    updateFiles();
    if (ui->cbxFile->count() < 1) {
        return;
    }

    ui->pbEdit->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect->setIcon(QIcon::fromTheme("view-form"));
    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon( QIcon::fromTheme("document-export"));
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

    ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeView->header()->setStretchLastSection(false);
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

    on_cbxFileCurrentIndexChanged(0);

    connect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);
}

bool ThermalForm::canToShow()
{
    for (Gerber::File* file : Project::files<Gerber::File>())
        if (file->flashedApertures())
            return true;

    QMessageBox::information(nullptr, "", tr("No data to process."));
    return false;
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

void ThermalForm::on_pbCreate_clicked() { createFile(); }

void ThermalForm::on_pbClose_clicked()
{
    if (parent())
        if (auto* w = dynamic_cast<QWidget*>(parent()); w)
            w->close();
}

void ThermalForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void ThermalForm::createFile()
{

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Pathss wBridgePaths;

    for (QSharedPointer<ThermalPreviewItem> item : m_sourcePreview) {
        if (item->isValid()) {
            wPaths.append(item->paths());
            wBridgePaths.append(item->bridge());
        }
    }

    GCode::GCodeParams gpc;
    gpc.convent = true;
    gpc.side = GCode::Outer;
    gpc.tool.append(tool);
    gpc.dParam[GCode::Depth] = ui->dsbxDepth->value();
    gpc.dParam[GCode::FileId] = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>())->id();
    m_tpc->setGcp(gpc);
    m_tpc->addPaths(wPaths);
    m_tpc->addSupportPaths(wBridgePaths);
    createToolpath(gpc);
}

void ThermalForm::updateName()
{
    ui->leName->setText(tr("Thermal"));
}

void ThermalForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    createTPI(static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>())->apertures());
}

void ThermalForm::createTPI(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value)
{
    m_sourcePreview.clear();
    m_apertures = *value;
    model = new ThermalModel(this);
    const auto* file = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>());
    boardSide = file->side();

    using Worker = std::tuple<const Gerber::GraphicObject*, ThermalNode*, QString>;
    QVector<Worker> map;
    auto creator = [this](Worker w) {
        static QMutex m;
        auto [go, thermalNode, name] = w;
        auto item = new ThermalPreviewItem(*go, tool, m_depth);
        item->setToolTip(name);
        QMutexLocker lock(&m);
        m_sourcePreview.append(QSharedPointer<ThermalPreviewItem>(item));
        thermalNode->append(new ThermalNode(drawRegionIcon(*go), name, 0.0, 0.5, 4, go->state().curPos(), item));
    };

    ThermalNode* thermalNode = nullptr;
    for (const Gerber::GraphicObject& go : *file) {
        if (go.state().type() == Gerber::Region && go.state().imgPolarity() == Gerber::Positive) {
            if (thermalNode == nullptr)
                thermalNode = model->appendRow(QIcon(), "Regions");
            map.append({ &go, thermalNode, "Region" });
        }
    }
    thermalNode = nullptr;
    for (const Gerber::GraphicObject& go : *file) {
        if (go.state().type() == Gerber::Line && go.state().imgPolarity() == Gerber::Positive && go.path().size() == 2 && Length(go.path().first(), go.path().last()) * dScale * 0.3 < m_apertures[go.state().aperture()]->minSize()) {
            if (thermalNode == nullptr)
                thermalNode = model->appendRow(QIcon(), "Lines");
            map.append({ &go, thermalNode, "Line" });
        }
    }
    QMap<int, QSharedPointer<Gerber::AbstractAperture>>::const_iterator apIt;
    for (apIt = m_apertures.cbegin(); apIt != m_apertures.cend(); ++apIt) {
        if (apIt.value()->isFlashed()) {
            QString name(apIt.value()->name());
            ThermalNode* thermalNode = model->appendRow(drawApertureIcon(apIt.value().data()), name);
            for (const Gerber::GraphicObject& go : *file) {
                if (go.state().dCode() == Gerber::D03 && go.state().aperture() == apIt.key())
                    map.append({ &go, thermalNode, name });
            }
        }
    }
    for (int i = 0, c = QThread::idealThreadCount(); i < map.size(); i += c) {
        auto m(map.mid(i, c));
        qDebug() << m.size();
        QFuture<void> future = QtConcurrent::map(m, creator);
        future.waitForFinished();
    }
    qDebug("QFuture");

    for (QSharedPointer<ThermalPreviewItem> item : m_sourcePreview)
        Scene::addItem(item.data());
    qDebug("Scene");
    //    updateCreateButton();
    delete ui->treeView->model();
    ui->treeView->setModel(model);
    connect(ui->treeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &ThermalForm::on_selectionChanged);
}

void ThermalForm::on_selectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    for (auto index : selected.indexes()) {
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
    for (auto index : deselected.indexes()) {
        auto* node = static_cast<ThermalNode*>(index.internalPointer());
        auto* item = node->item();
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
    for (auto item : m_sourcePreview) {
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

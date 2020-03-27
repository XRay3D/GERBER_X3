#include "thermalform.h"
#include "thermaldelegate.h"
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
#include <settings.h>

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

    qreal scale = static_cast<double>(Size) / qMax(rect.width(), rect.height());

    double ky = rect.bottom() * scale;
    double kx = rect.left() * scale;
    if (rect.width() > rect.height())
        ky += (static_cast<double>(Size) - rect.height() * scale) / 2;
    else
        kx -= (static_cast<double>(Size) - rect.width() * scale) / 2;

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
    : FormsUtil(new GCode::ThermalCreator, parent)
    , ui(new Ui::ThermalForm)
{
    ui->setupUi(this);

    parent->setWindowTitle(ui->label->text());

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    MySettings settings;
    settings.beginGroup("ThermalForm");
    settings.endGroup();

    connect(ui->pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(ui->pbCreate, &QPushButton::clicked, this, &ThermalForm::createFile);
    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &ThermalForm::updateName);

    ui->treeView->setIconSize(QSize(Size, Size));

    updateName();

    chbx = new QCheckBox("", ui->treeView);
    chbx->setMinimumHeight(ui->treeView->header()->height() - 4);
    chbx->setEnabled(false);
    auto lay = new QGridLayout(ui->treeView->header());
    lay->addWidget(chbx, 0, 0, 1, 1, Qt::AlignLeft | Qt::AlignTop);
    lay->setContentsMargins(3, 0, 0, 0);

    updateFiles();

    if (ui->cbxFile->count() < 1)
        return;

    {
        int w = ui->treeView->indentation();
        int h = Size; //QFontMetrics(font()).height(); // ui->treeView->rowHeight(m_model->index(0, 0, QModelIndex()));
        QImage i(w, h, QImage::Format_ARGB32);
        i.fill(Qt::transparent);
        for (int y = 0; y < h; ++y)
            i.setPixelColor(w / 2, y, QColor(128, 128, 128));
        i.save("vline.png", "PNG");

        for (int x = w / 2; x < w; ++x)
            i.setPixelColor(x, h / 2, QColor(128, 128, 128));
        i.save("branch-more.png", "PNG");

        i.fill(Qt::transparent);
        for (int y = 0; y < h / 2; ++y)
            i.setPixelColor(w / 2, y, QColor(128, 128, 128));
        for (int x = w / 2; x < w; ++x)
            i.setPixelColor(x, h / 2, QColor(128, 128, 128));
        i.save("branch-end.png", "PNG");

        QFile file(":/qtreeviewstylesheet/QTreeView.qss");
        file.open(QFile::ReadOnly);
        ui->treeView->setUniformRowHeights(true);
        ui->treeView->setStyleSheet(file.readAll());
        ui->treeView->header()->setMinimumHeight(h);
        ui->treeView->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
        ui->treeView->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        ui->treeView->header()->setStretchLastSection(false);
        ui->treeView->hideColumn(1);
        ui->treeView->setItemDelegate(new ThermalDelegate(this));
    }
}

ThermalForm::~ThermalForm()
{
    m_sourcePreview.clear();
    QSettings settings;
    settings.beginGroup("ThermalForm");
    settings.endGroup();
    delete ui;
}

void ThermalForm::updateFiles()
{

    disconnect(ui->cbxFile, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ThermalForm::on_cbxFileCurrentIndexChanged);

    ui->cbxFile->clear();

    for (Gerber::File* file : Project::instance()->files<Gerber::File>()) {
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
    for (Gerber::File* file : Project::instance()->files<Gerber::File>())
        if (file->flashedApertures())
            return true;

    QMessageBox::information(nullptr, "", tr("No data to process."));
    return false;
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
    gpc.setConvent(true);
    gpc.setSide(GCode::Outer);
    gpc.tools.append(tool);
    gpc.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    gpc.params[GCode::GCodeParams::FileId] = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>())->id();
    m_tpc->setGcp(gpc);
    m_tpc->addPaths(wPaths);
    m_tpc->addSupportPaths(wBridgePaths);
    createToolpath();
}

void ThermalForm::updateName()
{
    tool = ui->toolHolder->tool();
    ui->leName->setText(tr("Thermal"));
    redraw();
}

void ThermalForm::on_cbxFileCurrentIndexChanged(int /*index*/)
{
    createTPI(static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>())->apertures());
}

void ThermalForm::createTPI(const QMap<int, QSharedPointer<Gerber::AbstractAperture>>* value)
{
    m_sourcePreview.clear();
    m_apertures = *value;

    if (model)
        delete ui->treeView->model();

    model = new ThermalModel(ui->treeView);
    auto const file = static_cast<Gerber::File*>(ui->cbxFile->currentData().value<void*>());
    boardSide = file->side();
    model->appendRow(QIcon(), tr("All"));

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
                thermalNode = model->appendRow(QIcon(), tr("Regions"));
            map.append({ &go, thermalNode, tr("Region") });
        }
    }

    thermalNode = nullptr;
    for (const Gerber::GraphicObject& go : *file) {
        if (go.state().type() == Gerber::Line
            && go.state().imgPolarity() == Gerber::Positive
            && go.path().size() == 2
            && Length(go.path().first(), go.path().last()) * dScale * 0.3 < m_apertures[go.state().aperture()]->minSize()) {
            if (thermalNode == nullptr)
                thermalNode = model->appendRow(QIcon(), tr("Lines"));
            map.append({ &go, thermalNode, tr("Line") });
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
        QFuture<void> future = QtConcurrent::map(m, creator);
        future.waitForFinished();
    }

    for (QSharedPointer<ThermalPreviewItem> item : m_sourcePreview)
        Scene::addItem(item.data());

    //    updateCreateButton();

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

#include "voronoiform.h"
#include "ui_voronoiform.h"

#include "gcodepropertiesform.h"
#include "gi/bridgeitem.h"
#include "tooldatabase/tooldatabase.h"
#include "tooldatabase/tooleditdialog.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QPicture>
#include <gcvoronoi.h>
#include <graphicsview.h>
#include <myclipper.h>
#include <scene.h>

VoronoiForm::VoronoiForm(QWidget* parent)
    : FormsUtil("VoronoiForm", new GCode::VoronoiCreator, parent)
    , ui(new Ui::VoronoiForm)
{
    ui->setupUi(this);

    ui->toolName->setTool(tool);

    updateName();

    ui->pbEdit->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect->setIcon(QIcon::fromTheme("view-form"));
    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));
    connect(ui->pbCreate, &QPushButton::clicked, this, &VoronoiForm::createFile);

    parent->setWindowTitle(ui->label->text());

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }
    connect(ui->dsbxDepth, &DepthForm::valueChanged, this, &VoronoiForm::setWidth);
    connect(ui->dsbxWidth, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &VoronoiForm::setWidth);

    QSettings settings;
    settings.beginGroup("VoronoiForm");
    ui->dsbxPrecision->setValue(settings.value("dsbxPrecision", 0.1).toDouble());
    ui->dsbxWidth->setValue(settings.value("dsbxWidth").toDouble());
#ifdef _USE_CGAL_
    ui->cbxSolver->setCurrentIndex(settings.value("cbxSolver").toInt());
#else
    ui->cbxSolver->setCurrentIndex(0);
    ui->cbxSolver->setEnabled(false);
#endif
    ui->dsbxOffset->setValue(settings.value("dsbxOffset", 1.0).toDouble());
    settings.endGroup();
}

VoronoiForm::~VoronoiForm()
{
    QSettings settings;
    settings.beginGroup("VoronoiForm");
    settings.setValue("dsbxPrecision", ui->dsbxPrecision->value());
    settings.setValue("dsbxWidth", ui->dsbxWidth->value());
    settings.setValue("cbxSolver", ui->cbxSolver->currentIndex());
    settings.setValue("dsbxOffset", ui->dsbxOffset->value());
    settings.endGroup();
    delete ui;
}

void VoronoiForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving, Tool::Laser });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->toolName->setTool(tool);
        updateName();
    }
}

void VoronoiForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.setTool(tool);
    if (d.exec()) {
        tool = d.tool();
        tool.setId(-1);
        ui->toolName->setTool(tool);
        updateName();
    }
}

void VoronoiForm::on_pbClose_clicked()
{
    if (parent())
        if (auto* w = dynamic_cast<QWidget*>(parent()); w)
            w->close();
}

void VoronoiForm::createFile()
{
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;

    for (auto* item : Scene::selectedItems()) {
        auto* gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GiGerber:
            //GerberItem* gi = static_cast<GerberItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                QMessageBox::warning(this, tr("Warning"), tr("Working items from different files!"));
                return;
            }
            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
            break;
        case GiAperturePath:
            //RawItem* gi = static_cast<RawItem*>(item);
            if (!file) {
                file = gi->file();
                boardSide = gi->file()->side();
            }
            if (file != gi->file()) {
                QMessageBox::warning(this, tr("Warning"), tr("Working items from different files!"));
                return;
            }
            wRawPaths.append(static_cast<GraphicsItem*>(item)->paths());
            break;
            //        case DrillItemType:
            //            //            if (static_cast<DrillItem*>(item)->isSlot())
            //            //                wrPaths.append(static_cast<GraphicsItem*>(item)->paths());
            //            //            else
            //            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
            //            break;
        default:
            break;
        }
    }

    if (wPaths.isEmpty() && wRawPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::GCodeParams gpc;
    gpc.convent = true;
    gpc.side = GCode::Outer;
    gpc.tool.append(tool);
    gpc.dParam[GCode::Depth] = ui->dsbxDepth->value();
    gpc.dParam[GCode::Tolerance] = ui->dsbxPrecision->value();
    gpc.dParam[GCode::Width] = ui->dsbxWidth->value() + 0.001;
    gpc.dParam[GCode::VorT] = ui->cbxSolver->currentIndex();
    gpc.dParam[GCode::FrameOffset] = ui->dsbxOffset->value();

    m_tpc->setGcp(gpc);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    createToolpath();
}

void VoronoiForm::updateName()
{
    ui->leName->setText(tr("Voronoi"));
    setWidth(0.0);
}

void VoronoiForm::on_leName_textChanged(const QString& arg1)
{
    m_fileName = arg1;
}

void VoronoiForm::setWidth(double /*w*/)
{
    const double d = tool.getDiameter(ui->dsbxDepth->value());
    if (ui->dsbxWidth->value() > 0.0 && (qFuzzyCompare(ui->dsbxWidth->value(), d) || ui->dsbxWidth->value() < d)) {
        QMessageBox::warning(this, tr("Warning"), tr("The width must be larger than the tool diameter!"));
        ui->dsbxWidth->setValue(d + 0.05);
    }
}

void VoronoiForm::editFile(GCode::File* /*file*/)
{
}

void VoronoiForm::on_cbxSolver_currentIndexChanged(int index)
{
    ui->dsbxPrecision->setEnabled(!index);
}

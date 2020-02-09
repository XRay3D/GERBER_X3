#include "pocketrasterform.h"
#include "ui_pocketrasterform.h"

#include <tooldatabase/tooldatabase.h>
#include <tooldatabase/tooleditdialog.h>

#include <gcpocketraster.h>

PocketRasterForm::PocketRasterForm(QWidget* parent)
    : FormsUtil("PocketRasterForm", new GCode::RasterCreator, parent)
    , ui(new Ui::PocketRasterForm)
    , names { tr("Raster On"), tr("Raster Outside"), tr("Raster Inside") }
    , pixmaps {
        QStringLiteral(":/toolpath/pock_rast_climb.svg"),
        QStringLiteral(":/toolpath/pock_rast_conv.svg"),
    }
{
    ui->setupUi(this);
    ui->toolName->setTool(tool);

    connect(ui->rbClimb, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &PocketRasterForm::rb_clicked);

    QSettings settings;
    settings.beginGroup("PocketRasterForm");
    ui->cbxPass->setCurrentIndex(settings.value("cbxPass").toInt());
    ui->dsbxAcc->setValue(settings.value("dsbxAcc").toDouble());
    ui->dsbxAngle->setValue(settings.value("dsbxAngle").toDouble());
    ui->rbClimb->setChecked(settings.value("rbClimb").toBool());
    ui->rbConventional->setChecked(settings.value("rbConventional").toBool());
    ui->rbFast->setChecked(settings.value("rbFast").toBool());
    ui->rbInside->setChecked(settings.value("rbInside").toBool());
    ui->rbNormal->setChecked(settings.value("rbNormal").toBool());
    ui->rbOutside->setChecked(settings.value("rbOutside").toBool());
    settings.endGroup();

    ui->pbEdit->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect->setIcon(QIcon::fromTheme("view-form"));
    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    rb_clicked();

    parent->setWindowTitle(ui->label->text());
}

PocketRasterForm::~PocketRasterForm()
{
    QSettings settings;
    settings.beginGroup("PocketRasterForm");
    settings.setValue("cbxPass", ui->cbxPass->currentIndex());
    settings.setValue("dsbxAcc", ui->dsbxAcc->value());
    settings.setValue("dsbxAngle", ui->dsbxAngle->value());
    settings.setValue("rbClimb", ui->rbClimb->isChecked());
    settings.setValue("rbConventional", ui->rbConventional->isChecked());
    settings.setValue("rbFast", ui->rbFast->isChecked());
    settings.setValue("rbInside", ui->rbInside->isChecked());
    settings.setValue("rbNormal", ui->rbNormal->isChecked());
    settings.setValue("rbOutside", ui->rbOutside->isChecked());
    settings.endGroup();
    delete ui;
}

void PocketRasterForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving, Tool::Laser });
    if (tdb.exec()) {
        tool = tdb.tool();
        ui->rbFast->setEnabled(tool.type() == Tool::Laser);
        ui->rbFast->setChecked(tool.type() == Tool::Laser);
        ui->rbNormal->setChecked(tool.type() != Tool::Laser);
        ui->toolName->setTool(tool);
        updateName();
    }
}

void PocketRasterForm::on_pbEdit_clicked()
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

void PocketRasterForm::on_pbCreate_clicked()
{
    createFile();
}

void PocketRasterForm::on_pbClose_clicked()
{
    if (parent())
        if (auto* w = dynamic_cast<QWidget*>(parent()); w)
            w->close();
}

void PocketRasterForm::createFile()
{
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;

    for (auto* item : Scene::selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GiGerber:
        case GiAperturePath:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                QMessageBox::warning(this, tr("Warning"), tr("Working items from different files!"));
                return;
            }
            if (item->type() == GiGerber)
                wPaths.append(gi->paths());
            else
                wRawPaths.append(gi->paths());
            m_usedItems[gi->file()->id()].append(gi->id());
            break;
        case GiShapeC:
            wRawPaths.append(gi->paths());
            //m_used[gi->file()->id()].append(gi->id());
            break;
        case GiDrill:
            wPaths.append(gi->paths());
            m_usedItems[gi->file()->id()].append(gi->id());
            break;
        default:
            break;
        }

        //        if (item->type() == GerberItemType) {
        //            GerberItem* gi = static_cast<GerberItem*>(item);
        //            if (!file)
        //                file = gi->typedFile<Gerber::File>();
        //            if (file != gi->file()) {
        //                QMessageBox::warning(this, tr("Warning"), tr("Working items from different files!"));
        //                return;
        //            }
        //            boardSide = gi->file()->side();
        //        }
        //        if (item->type() == GerberItemType || item->type() == DrillItemType)
        //            wPaths.append(static_cast<GraphicsItem*>(item)->paths());
    }

    if (wRawPaths.isEmpty() && wPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::GCodeParams gcp;
    gcp.setConvent(ui->rbConventional->isChecked());
    gcp.setSide(side);
    gcp.tool.append(tool);
    gcp.tool.append(tool2);

    gcp.dParam[GCode::UseAngle] = ui->dsbxAngle->value();
    gcp.dParam[GCode::Depth] = ui->dsbxDepth->value();
    gcp.dParam[GCode::Pass] = ui->cbxPass->currentIndex();
    if (ui->rbFast->isChecked()) {
        gcp.dParam[GCode::Fast] = true;
        gcp.dParam[GCode::AccDistance] = (tool.feedRateMmS() * tool.feedRateMmS()) / (2 * ui->dsbxAcc->value());
    }

    m_tpc->setGcp(gcp);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    fileCount = 1;
    createToolpath();
}

void PocketRasterForm::updateName()
{
    ui->leName->setText(names[side]);
}

void PocketRasterForm::updatePixmap()
{
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmaps[direction]).pixmap(QSize(size, size)));
}

void PocketRasterForm::rb_clicked()
{

    if (ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if (ui->rbInside->isChecked())
        side = GCode::Inner;

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updatePixmap();
}

void PocketRasterForm::resizeEvent(QResizeEvent* event)
{
    updatePixmap();
    QWidget::resizeEvent(event);
}

void PocketRasterForm::showEvent(QShowEvent* event)
{
    updatePixmap();
    QWidget::showEvent(event);
}

void PocketRasterForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void PocketRasterForm::editFile(GCode::File* /*file*/)
{
}

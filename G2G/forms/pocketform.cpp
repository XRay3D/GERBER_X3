#include "pocketform.h"
#include "filetree/filemodel.h"
#include "gcodepropertiesform.h"
#include "tooldatabase/tooldatabase.h"
#include "ui_pocketform.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <gccreator.h>
#include <gcfile.h>
#include <gcpocket.h>
#include <myclipper.h>
#include <scene.h>
#include <tooldatabase/tooleditdialog.h>

const int QPairID = qRegisterMetaType<QPair<Tool, Tool>>("QPair<Tool,Tool>");

enum {
    Offset,
    Raster,
};

PocketForm::PocketForm(QWidget* parent)
    : FormsUtil("PocketForm", new GCode::PocketCreator, parent)
    , ui(new Ui::PocketForm)
{
    ui->setupUi(this);
    ui->lblToolName->setText(tool.name());
    ui->lblToolName_2->setText(tool2.name());
    updateArea();
    //    ui->dsbxDepth->setValue(GCodePropertiesForm::thickness);

    auto rb_clicked = [this] {
        if (ui->rbOutside->isChecked())
            side = GCode::Outer;
        else if (ui->rbInside->isChecked())
            side = GCode::Inner;

        if (ui->rbOffset->isChecked()) {
            type = Offset;
        } else if (ui->rbRaster->isChecked()) {
            type = Raster;
            ui->chbxUseTwoTools->setChecked(false);
        }
        {
            ui->cbxPass->setVisible(ui->rbRaster->isChecked());
            ui->labelPass->setVisible(ui->rbRaster->isChecked());
            ui->dsbxAngle->setVisible(ui->rbRaster->isChecked());
            ui->labelAngle->setVisible(ui->rbRaster->isChecked());
        }
        {
            ui->chbxUseTwoTools->setVisible(!ui->rbRaster->isChecked());
            ui->sbxSteps->setVisible(!ui->rbRaster->isChecked());
            ui->labelSteps->setVisible(!ui->rbRaster->isChecked());
        }

        if (ui->rbClimb->isChecked())
            direction = GCode::Climb;
        else if (ui->rbConventional->isChecked())
            direction = GCode::Conventional;

        updateName();
        updatePixmap();
    };

    QSettings settings;
    settings.beginGroup("PocketForm");
    if (settings.value("rbClimb").toBool())
        ui->rbClimb->setChecked(true);
    if (settings.value("rbConventional").toBool())
        ui->rbConventional->setChecked(true);
    if (settings.value("rbInside").toBool())
        ui->rbInside->setChecked(true);
    if (settings.value("rbOffset").toBool())
        ui->rbOffset->setChecked(true);
    if (settings.value("rbOutside").toBool())
        ui->rbOutside->setChecked(true);
    if (settings.value("rbRaster").toBool())
        ui->rbRaster->setChecked(true);
    ui->dsbxAngle->setValue(settings.value("dsbxAngle").toDouble());
    ui->cbxPass->setCurrentIndex(settings.value("cbxPass").toInt());
    ui->dsbxDepth->setValue(settings.value("dsbxDepth").toDouble());
    if (settings.value("rbBoard").toBool())
        ui->dsbxDepth->rbBoard->setChecked(true);
    if (settings.value("rbCopper").toBool())
        ui->dsbxDepth->rbCopper->setChecked(true);
    rb_clicked();
    on_chbxUseTwoTools_toggled(settings.value("chbxUseTwoTools").toBool());
    settings.endGroup();

    ui->pbEdit->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect->setIcon(QIcon::fromTheme("view-form"));
    ui->pbEdit_2->setIcon(QIcon::fromTheme("document-edit"));
    ui->pbSelect_2->setIcon(QIcon::fromTheme("view-form"));
    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    ui->sbxSteps->setSuffix(tr(" - Infinity"));

    connect(ui->rbClimb, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOffset, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, rb_clicked);
    connect(ui->rbRaster, &QRadioButton::clicked, rb_clicked);
    parent->setWindowTitle(ui->label->text());
}

PocketForm::~PocketForm()
{
    QSettings settings;
    settings.beginGroup("PocketForm");
    settings.setValue("rbClimb", ui->rbClimb->isChecked());
    settings.setValue("rbConventional", ui->rbConventional->isChecked());
    settings.setValue("rbInside", ui->rbInside->isChecked());
    settings.setValue("rbOffset", ui->rbOffset->isChecked());
    settings.setValue("rbOutside", ui->rbOutside->isChecked());
    settings.setValue("rbRaster", ui->rbRaster->isChecked());
    settings.setValue("chbxUseTwoTools", ui->chbxUseTwoTools->isChecked());

    settings.setValue("dsbxAngle", ui->dsbxAngle->value());
    settings.setValue("cbxPass", ui->cbxPass->currentIndex());
    settings.setValue("dsbxDepth", ui->dsbxDepth->value(true));
    settings.setValue("rbBoard", ui->dsbxDepth->rbBoard->isChecked());
    settings.setValue("rbCopper", ui->dsbxDepth->rbCopper->isChecked());
    settings.endGroup();
    delete ui;
}

void PocketForm::on_pbSelect_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        //        tool = tdb.tool();
        //        ui->lblToolName->setText(tool.name);
        //        updateName();
        Tool mpTool(tdb.tool());
        if (ui->chbxUseTwoTools->isChecked() && tool2.id() > -1 && tool2.diameter() <= mpTool.diameter()) {
            QMessageBox::warning(this, tr("Warning"), tr("The diameter of the second tool must be greater than the first!"));
        } else {
            tool = mpTool;
            ui->lblToolName->setText(tool.name());
            updateArea();
        }
    }
}
void PocketForm::on_pbSelect_2_clicked()
{
    ToolDatabase tdb(this, { Tool::EndMill, Tool::Engraving });
    if (tdb.exec()) {
        Tool mpTool(tdb.tool());
        if (tool.id() > -1 && tool.diameter() >= mpTool.diameter()) {
            QMessageBox::warning(this, tr("Warning"), tr("The diameter of the second tool must be greater than the first!"));
        } else {
            tool2 = mpTool;
            ui->lblToolName_2->setText(tool2.name());
        }
    }
}

void PocketForm::on_pbEdit_clicked()
{
    ToolEditDialog d;
    d.setTool(tool);
    if (d.exec()) {
        if (ui->chbxUseTwoTools->isChecked() && tool2.id() > -1 && tool2.diameter() <= d.tool().diameter()) {
            QMessageBox::warning(this, tr("Warning"), tr("The diameter of the second tool must be greater than the first!"));
            return;
        }
        updateArea();
        tool = d.tool();
        tool.setId(-1);
        ui->lblToolName->setText(tool.name());
        updateName();
    }
}

void PocketForm::on_pbEdit_2_clicked()
{
    ToolEditDialog d;
    d.setTool(tool2);
    if (d.exec()) {
        if (tool.id() > -1 && tool.diameter() >= d.tool().diameter()) {
            QMessageBox::warning(this, tr("Warning"), tr("The diameter of the second tool must be greater than the first!"));
            return;
        }
        tool2 = d.tool();
        tool.setId(-1);
        ui->lblToolName_2->setText(tool2.name());
        updateName();
    }
}

void PocketForm::on_pbCreate_clicked()
{
    createFile();
}

void PocketForm::on_pbClose_clicked()
{
    if (parent())
        if (auto* w = dynamic_cast<QWidget*>(parent()); w)
            w->close();
}

void PocketForm::createFile()
{
    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }
    if (ui->chbxUseTwoTools->isChecked() && !tool2.isValid()) {
        tool2.errorMessageBox(this);
        return;
    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;

    for (auto* item : Scene::selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GerberItemType:
        case RawItemType:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
                return;
            }
            if (item->type() == GerberItemType)
                wPaths.append(gi->paths());
            else
                wRawPaths.append(gi->paths());
            m_usedItems[gi->file()->id()].append(gi->id());
            break;
        case Shape:
            wRawPaths.append(gi->paths());
            //m_used[gi->file()->id()].append(gi->id());
            break;
        case DrillItemType:
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
    gcp.convent = ui->rbConventional->isChecked();
    gcp.side = side;
    gcp.tool.append(tool);
    gcp.tool.append(tool2);

    gcp.dParam[GCode::UseAngle] = ui->dsbxAngle->value();
    gcp.dParam[GCode::Depth] = ui->dsbxDepth->value();

    gcp.dParam[GCode::Pass] = ui->cbxPass->currentIndex();
    gcp.dParam[GCode::UseRaster] = ui->rbRaster->isChecked();
    gcp.dParam[GCode::Steps] = ui->sbxSteps->value();
    gcp.dParam[GCode::TwoTools] = ui->chbxUseTwoTools->isChecked();
    gcp.dParam[GCode::MinArea] = ui->dsbxMinArea->value();
    m_tpc->setGcp(gcp);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    if (ui->chbxUseTwoTools->isChecked())
        fileCount = 2;
    createToolpath(gcp);
}

void PocketForm::on_sbxSteps_valueChanged(int arg1)
{
    ui->sbxSteps->setSuffix(!arg1 ? tr(" - Infinity") : "");
}

void PocketForm::updateName()
{
    static const QStringList name = { tr("Pocket On"), tr("Pocket Outside"), tr("Pocket Inside"), tr("Raster On"), tr("Raster Outside"), tr("Raster Inside") };
    ui->leName->setText(name[side + type * 3]);
}

void PocketForm::updatePixmap()
{
    static const QStringList pixmapList = {
        QStringLiteral(":/toolpath/pock_offs_climb.svg"),
        QStringLiteral(":/toolpath/pock_rast_climb.svg"),
        QStringLiteral(":/toolpath/pock_offs_conv.svg"),
        QStringLiteral(":/toolpath/pock_rast_conv.svg"),
    };
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmapList[type + direction * 2]).pixmap(QSize(size, size)));
}

void PocketForm::updateArea()
{
    //    if (qFuzzyIsNull(ui->dsbxMinArea->value()))
    ui->dsbxMinArea->setValue((tool.getDiameter(ui->dsbxDepth->value() * 0.5)) * (tool.getDiameter(ui->dsbxDepth->value() * 0.5)) * M_PI * 0.5);
}

void PocketForm::resizeEvent(QResizeEvent* event)
{
    updatePixmap();
    QWidget::resizeEvent(event);
}

void PocketForm::showEvent(QShowEvent* event)
{
    updatePixmap();
    QWidget::showEvent(event);
}

void PocketForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void PocketForm::editFile(GCode::File* /*file*/)
{
}

void PocketForm::on_chbxUseTwoTools_toggled(bool checked)
{
    //ui->pbSelect_2->setEnabled(checked);
    //ui->lblToolName_2->setEnabled(checked);
    //ui->pbEdit_2->setEnabled(checked);
    //ui->sbxSteps->setEnabled(!checked);
    ui->chbxUseTwoTools->setChecked(checked);

    ui->labelSteps->setVisible(!checked);
    ui->sbxSteps->setVisible(!checked);

    ui->dsbxMinArea->setVisible(checked);
    ui->labelMinArea->setVisible(checked);

    ui->labelToolName2->setVisible(checked);
    ui->lblToolName_2->setVisible(checked);
    ui->pbEdit_2->setVisible(checked);
    ui->pbSelect_2->setVisible(checked);
}

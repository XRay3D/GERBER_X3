#include "pocketform.h"
#include "gcodepropertiesform.h"
#include "ui_pocketform.h"

#include "filetree/filemodel.h"
#include "gcode/gccreator.h"
#include "gcode/gcfile.h"
#include "tooldatabase/tooldatabase.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <myclipper.h>
#include <scene.h>
#include <tooldatabase/tooleditdialog.h>

const int QPairID = qRegisterMetaType<QPair<Tool, Tool>>("QPair<Tool,Tool>");

enum {
    Offset,
    Raster,
};

PocketForm::PocketForm(QWidget* parent)
    : FormsUtil("PocketForm", parent)
    , ui(new Ui::PocketForm)
{
    ui->setupUi(this);
    ui->lblToolName->setText(tool.name());
    ui->lblToolName_2->setText(tool2.name());

    ui->dsbxDepth->setValue(GCodePropertiesForm::thickness);

    auto rb_clicked = [&] {
        if (ui->rbOutside->isChecked())
            side = Outer;
        else if (ui->rbInside->isChecked())
            side = Inner;

        if (ui->rbOffset->isChecked()) {
            type = Offset;
        } else if (ui->rbRaster->isChecked()) {
            type = Raster;
            ui->chbxUseTwoTools->setChecked(false);
            ui->chbxUseTwoTools->clicked(false);
        }
        ui->chbxUseTwoTools->setEnabled(!ui->rbRaster->isChecked());
        ui->sbxSteps->setEnabled(!ui->rbRaster->isChecked());
        //ui->cbxPass->setEnabled(ui->rbRaster->isChecked());
        ui->dsbxAngle->setEnabled(ui->rbRaster->isChecked());
        ui->cbxPass->setEnabled(ui->rbRaster->isChecked());

        if (ui->rbClimb->isChecked())
            direction = Climb;
        else if (ui->rbConventional->isChecked())
            direction = Conventional;

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

    on_chbxUseTwoTools_clicked(settings.value("chbxUseTwoTools").toBool());
    settings.endGroup();

    ui->pbEdit->setIcon(Icon(ButtonEditIcon));
    ui->pbSelect->setIcon(Icon(ButtonSelectIcon));
    ui->pbEdit_2->setIcon(Icon(ButtonEditIcon));
    ui->pbSelect_2->setIcon(Icon(ButtonSelectIcon));
    ui->pbClose->setIcon(Icon(ButtonCloseIcon));
    ui->pbCreate->setIcon(Icon(ButtonCreateIcon));

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    ui->sbxSteps->setSuffix(tr(" - Infinity"));

    rb_clicked();
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
        tool2 = d.tool();
        tool.setId(-1);
        ui->lblToolName_2->setText(tool2.name());
        updateName();
    }
}

void PocketForm::on_pbCreate_clicked()
{
    create();
}

void PocketForm::on_pbClose_clicked()
{
    if (parent())
        static_cast<QWidget*>(parent())->close();
}

void PocketForm::create()
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
    AbstractFile const* file = nullptr;

    for (QGraphicsItem* item : Scene::selectedItems()) {
        GraphicsItem* gi = static_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GerberItemType:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                QMessageBox::warning(this, "", tr("Working items from different files!"));
                return;
            }
            wPaths.append(gi->paths());
            m_used[gi->file()->id()].append(gi->id());
            break;
        case DrillItemType:
            wPaths.append(gi->paths());
            m_used[gi->file()->id()].append(gi->id());
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

    if (wPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::Creator* tps = toolPathCreator(wPaths, ui->rbConventional->isChecked(), side);
    if (ui->chbxUseTwoTools->isChecked()) {
        fileCount = 2;
        connect(this, &PocketForm::createPocket2, tps, &GCode::Creator::createPocket2);
        emit createPocket2({ tool, tool2 }, ui->dsbxDepth->value());
    } else if (ui->rbRaster->isChecked()) {
        connect(this, &PocketForm::createRaster, tps, &GCode::Creator::createRaster);
        emit createRaster(tool, ui->dsbxDepth->value(), ui->dsbxAngle->value(), ui->cbxPass->currentIndex());
    } else {
        connect(this, &PocketForm::createPocket, tps, &GCode::Creator::createPocket);
        emit createPocket(tool, ui->dsbxDepth->value(), ui->sbxSteps->value());
    }

    showProgress();
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

void PocketForm::on_chbxUseTwoTools_clicked(bool checked)
{
    ui->chbxUseTwoTools->setChecked(checked);
    ui->lblToolName_2->setEnabled(checked);
    ui->pbEdit_2->setEnabled(checked);
    ui->pbSelect_2->setEnabled(checked);
    ui->sbxSteps->setEnabled(!checked);
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

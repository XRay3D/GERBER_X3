#include "pocketoffsetform.h"
#include "filetree/filemodel.h"
#include "gcodepropertiesform.h"
#include "tooldatabase/tooldatabase.h"
#include "ui_pocketoffsetform.h"
#include <QDockWidget>
#include <QMessageBox>
#include <QSettings>
#include <gbrfile.h>
#include <gccreator.h>
#include <gcfile.h>
#include <gcpocketoffset.h>
#include <myclipper.h>
#include <scene.h>
#include <settings.h>
#include <tooldatabase/tooleditdialog.h>

enum {
    Offset,
    Raster,
};

PocketOffsetForm::PocketOffsetForm(QWidget* parent)
    : FormsUtil(new GCode::PocketCreator, parent)
    , ui(new Ui::PocketOffsetForm)
    , names { tr("Pocket On"), tr("Pocket Outside"), tr("Pocket Inside") }
    , pixmaps {
        QStringLiteral(":/toolpath/pock_offs_climb.svg"),
        QStringLiteral(":/toolpath/pock_offs_conv.svg"),
    }
{
    ui->setupUi(this);
    ui->toolHolder->label()->setText("Tool 1:");
    ui->toolHolder2->label()->setText("Tool 2:");
    ui->toolHolder3->label()->setText("Tool 3:");
    ui->toolHolder4->label()->setText("Tool 4:");

    parent->setWindowTitle(ui->label->text());

    ui->pbClose->setIcon(QIcon::fromTheme("window-close"));
    ui->pbCreate->setIcon(QIcon::fromTheme("document-export"));

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    MySettings settings;
    settings.beginGroup("PocketOffsetForm");
    settings.getValue(ui->sbxToolQty);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOutside);
    settings.endGroup();

    updateArea();
    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &PocketOffsetForm::rb_clicked);
    connect(ui->sbxToolQty, qOverload<int>(&QSpinBox::valueChanged), this, &PocketOffsetForm::rb_clicked);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);
    connect(ui->toolHolder2, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);
    connect(ui->toolHolder3, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);
    connect(ui->toolHolder4, &ToolSelectorForm::updateName, this, &PocketOffsetForm::updateName);

    connect(ui->pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(ui->pbCreate, &QPushButton::clicked, this, &PocketOffsetForm::createFile);

    ui->sbxSteps->setSuffix(tr(" - Infinity"));
}

PocketOffsetForm::~PocketOffsetForm()
{
    MySettings settings;
    settings.beginGroup("PocketOffsetForm");
    settings.setValue(ui->sbxToolQty);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOutside);
    settings.endGroup();
    delete ui;
}

void PocketOffsetForm::createFile()
{
    const auto tool { ui->toolHolder->tool() };
    const auto tool2 { ui->toolHolder2->tool() };
    const auto tool3 { ui->toolHolder3->tool() };
    const auto tool4 { ui->toolHolder4->tool() };

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }
    if (!tool2.isValid()) {
        tool2.errorMessageBox(this);
        return;
    }
    if (!tool3.isValid()) {
        tool3.errorMessageBox(this);
        return;
    }
    if (!tool4.isValid()) {
        tool4.errorMessageBox(this);
        return;
    }

    //    if (ui->chbxUseTwoTools->isChecked() && !tool2.isValid()) {
    //        tool2.errorMessageBox(this);
    //        return;
    //    }

    Paths wPaths;
    Paths wRawPaths;
    AbstractFile const* file = nullptr;
    bool skip { true };

    for (auto* item : App::scene()->selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (item->type()) {
        case GiGerber:
        case GiAperturePath:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                if (skip) {
                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                        return;
                }
            }
            if (item->type() == GiGerber)
                wPaths.append(gi->paths());
            else
                wRawPaths.append(gi->paths());
            break;
        case GiShapeC:
            wRawPaths.append(gi->paths());
            break;
        case GiDrill:
            wPaths.append(gi->paths());
            break;
        default:
            break;
        }
        addUsedGi(gi);
    }

    if (wRawPaths.isEmpty() && wPaths.isEmpty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::GCodeParams gcp;
    gcp.setConvent(ui->rbConventional->isChecked());
    gcp.setSide(side);
    gcp.tools.append(tool);
    gcp.tools.append(tool2);

    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    gcp.params[GCode::GCodeParams::Steps] = ui->sbxSteps->value();
    //    gcp.params[GCode::GCodeParams::TwoTools] = ui->chbxUseTwoTools->isChecked();
    gcp.params[GCode::GCodeParams::MinArea] = ui->dsbxMinArea->value();

    m_tpc->setGcp(gcp);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    //    if (ui->chbxUseTwoTools->isChecked())
    //        fileCount = 2;
    createToolpath();
}

void PocketOffsetForm::on_sbxSteps_valueChanged(int arg1)
{
    ui->sbxSteps->setSuffix(!arg1 ? tr(" - Infinity") : "");
}

void PocketOffsetForm::updateName()
{
    ui->leName->setText(names[side]);
}

void PocketOffsetForm::updatePixmap()
{
    int size = qMin(ui->lblPixmap->height(), ui->lblPixmap->width());
    ui->lblPixmap->setPixmap(QIcon(pixmaps[direction]).pixmap(QSize(size, size)));
}

void PocketOffsetForm::updateArea()
{
    const auto tool { ui->toolHolder->tool() };

    if (qFuzzyIsNull(ui->dsbxMinArea->value()))
        ui->dsbxMinArea->setValue((tool.getDiameter(ui->dsbxDepth->value() * 0.5)) * (tool.getDiameter(ui->dsbxDepth->value() * 0.5)) * M_PI * 0.5);
}

void PocketOffsetForm::rb_clicked()
{
    const auto tool { ui->toolHolder->tool() };

    if (ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if (ui->rbInside->isChecked())
        side = GCode::Inner;

    //    if (tool.type() == Tool::Laser)
    //        ui->chbxUseTwoTools->setChecked(false);

    if (ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if (ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    {
        //        const bool checked = ui->chbxUseTwoTools->isChecked();
        //        ui->chbxUseTwoTools->setChecked(checked);
        const bool checked { ui->sbxToolQty->value() > 1 };

        ui->labelSteps->setVisible(!checked);
        ui->sbxSteps->setVisible(!checked);

        ui->dsbxMinArea->setVisible(checked);
        ui->labelMinArea->setVisible(checked);

        switch (ui->sbxToolQty->value()) {
        case 1:
            ui->toolHolder2->setVisible(checked);
            ui->toolHolder3->setVisible(checked);
            ui->toolHolder4->setVisible(checked);
            break;
        case 2:
            ui->toolHolder2->setVisible(checked);
            ui->toolHolder3->setVisible(!checked);
            ui->toolHolder4->setVisible(!checked);
            break;
        case 3:
            ui->toolHolder2->setVisible(checked);
            ui->toolHolder3->setVisible(checked);
            ui->toolHolder4->setVisible(!checked);
            break;
        case 4:
            ui->toolHolder2->setVisible(checked);
            ui->toolHolder3->setVisible(checked);
            ui->toolHolder4->setVisible(checked);
            break;
        }
    }

    updateName();
    updatePixmap();
}

void PocketOffsetForm::resizeEvent(QResizeEvent* event)
{
    updatePixmap();
    QWidget::resizeEvent(event);
}

void PocketOffsetForm::showEvent(QShowEvent* event)
{
    updatePixmap();
    QWidget::showEvent(event);
}

void PocketOffsetForm::on_leName_textChanged(const QString& arg1) { m_fileName = arg1; }

void PocketOffsetForm::editFile(GCode::File* /*file*/)
{
}

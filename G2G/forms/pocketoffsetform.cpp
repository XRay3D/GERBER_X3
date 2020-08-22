// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

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
    settings.getValue(ui->sbxSteps);
    settings.endGroup();

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
    settings.setValue(ui->sbxSteps);
    settings.endGroup();
    delete ui;
}

void PocketOffsetForm::createFile()
{
    const Tool tool[] {
        ui->toolHolder->tool(),
        ui->toolHolder2->tool(),
        ui->toolHolder3->tool(),
        ui->toolHolder4->tool()
    };

    for (const Tool& t : tool) {
        if (!t.isValid()) {
            t.errorMessageBox(this);
            return;
        }
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
        case GiShapeR:
        case GiShapeL:
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
    for (const Tool& t : tool) {
        gcp.tools.append(t);
        if (gcp.tools.size() == ui->sbxToolQty->value())
            break;
    }

    { // sort and unique
        auto cmp = [this](const Tool& t1, const Tool& t2) -> bool {
            return t1.getDiameter(ui->dsbxDepth->value()) > t2.getDiameter(ui->dsbxDepth->value());
        };
        auto cmp2 = [this](const Tool& t1, const Tool& t2) -> bool {
            return qFuzzyCompare(t1.getDiameter(ui->dsbxDepth->value()), t2.getDiameter(ui->dsbxDepth->value()));
        };
        std::sort(gcp.tools.begin(), gcp.tools.end(), cmp);
        auto last = std::unique(gcp.tools.begin(), gcp.tools.end(), cmp2);
        gcp.tools.erase(last, gcp.tools.end());
    }

    gcp.setConvent(ui->rbConventional->isChecked());
    gcp.setSide(side);
    gcp.params[GCode::GCodeParams::Depth] = ui->dsbxDepth->value();
    if (ui->sbxSteps->isVisible())
        gcp.params[GCode::GCodeParams::Steps] = ui->sbxSteps->value();

    m_tpc->setGcp(gcp);
    m_tpc->addPaths(wPaths);
    m_tpc->addRawPaths(wRawPaths);
    fileCount = gcp.tools.size(); //    ui->sbxToolQty->value();
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
        int fl = ui->sbxToolQty->value();

        ui->labelSteps->setVisible(fl < 2);
        ui->sbxSteps->setVisible(fl < 2);

        ui->toolHolder2->setVisible(--fl > 0);
        ui->toolHolder3->setVisible(--fl > 0);
        ui->toolHolder4->setVisible(--fl > 0);
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

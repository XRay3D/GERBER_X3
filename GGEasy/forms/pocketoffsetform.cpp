// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "pocketoffsetform.h"
#include "ui_pocketoffsetform.h"

#include "scene.h"
#include "settings.h"
#include <QMessageBox>

#include "leakdetector.h"

enum {
    Offset,
    Raster,
};

PocketOffsetForm::PocketOffsetForm(QWidget* parent)
    : FormsUtil(new GCode::PocketCreator, parent)
    , ui(new Ui::PocketOffsetForm)
    , names { tr("Pockert On"), tr("Pocket Outside"), tr("Pocket Inside") }
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
    FileInterface const* file = nullptr;
    bool skip { true };

    for (auto* item : App::scene()->selectedItems()) {
        GraphicsItem* gi = dynamic_cast<GraphicsItem*>(item);
        switch (static_cast<GiType>(item->type())) {
        case GiType::DataSolid:
        case GiType::DataPath:
            if (!file) {
                file = gi->file();
                boardSide = file->side();
            } else if (file != gi->file()) {
                if (skip) {
                    if ((skip = (QMessageBox::question(this, tr("Warning"), tr("Work items from different files!\nWould you like to continue?"), QMessageBox::Yes, QMessageBox::No) == QMessageBox::No)))
                        return;
                }
            }
            if (static_cast<GiType>(item->type()) == GiType::DataSolid)
                wPaths.append(gi->paths());
            else
                wRawPaths.append(gi->paths());
            break;
        case GiType::ShCircle:
        case GiType::ShRectangle:
        case GiType::ShPolyLine:
        case GiType::ShCirArc:
        case GiType::ShText:
            wRawPaths.append(gi->paths());
            break;
        case GiType::Drill:
            wPaths.append(gi->paths());
            break;
        default:
            break;
        }
        addUsedGi(gi);
    }

    if (wRawPaths.empty() && wPaths.empty()) {
        QMessageBox::warning(this, tr("Warning"), tr("No selected items for working..."));
        return;
    }

    GCode::GCodeParams gcp;
    for (const Tool& t : tool) {
        gcp.tools.push_back(t);
        if (gcp.tools.size() == static_cast<size_t>(ui->sbxToolQty->value()))
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
    fileCount = static_cast<int>(gcp.tools.size());
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
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[direction]).pixmap(QSize(150, 150)));
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

        //        ui->labelSteps->setVisible(fl < 2);
        //        ui->sbxSteps->setVisible(fl < 2);

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

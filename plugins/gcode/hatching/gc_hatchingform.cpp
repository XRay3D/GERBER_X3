// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/

#include "gc_hatchingform.h"
#include "gc_hatching.h"
#include "ui_hatchingform.h"

#include "scene.h"
#include "settings.h"
#include <QMessageBox>

HatchingForm::HatchingForm(GCodePlugin* plugin, QWidget* parent)
    : FormsUtil(plugin, new GCode::HatchingCreator, parent)
    , ui(new Ui::HatchingForm)
    , names { tr("Raster On"), tr("Hatching Outside"), tr("Hatching Inside") }
    , pixmaps {
        QStringLiteral("pock_rast_climb"),
        QStringLiteral("pock_rast_conv"),
    } {
    ui->setupUi(this);
    label->setText(tr("Crosshatch Toolpath"));
    /*parent->*/ setWindowTitle(label->text());

    for (QPushButton* button : findChildren<QPushButton*>())
        button->setIconSize({ 16, 16 });

    MySettings settings;
    settings.beginGroup("HatchingForm");
    settings.getValue(ui->cbxPass);
    settings.getValue(ui->dsbxAngle);
    settings.getValue(ui->dsbxHathStep);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbOutside);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &HatchingForm::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &HatchingForm::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &HatchingForm::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &HatchingForm::rb_clicked);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &HatchingForm::updateName);

    connect(pbClose, &QPushButton::clicked, dynamic_cast<QWidget*>(parent), &QWidget::close);
    connect(pbCreate, &QPushButton::clicked, this, &HatchingForm::createFile);
}

HatchingForm::~HatchingForm() {

    MySettings settings;
    settings.beginGroup("HatchingForm");
    settings.setValue(ui->cbxPass);
    settings.setValue(ui->dsbxAngle);
    settings.setValue(ui->dsbxHathStep);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbOutside);
    settings.endGroup();
    delete ui;
}

void HatchingForm::createFile() {
    const auto tool { ui->toolHolder->tool() };

    if (!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

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

    GCode::GCodeParams gcp_;
    gcp_.setConvent(ui->rbConventional->isChecked());
    gcp_.setSide(side);
    gcp_.tools.push_back(tool);

    gcp_.params[GCode::GCodeParams::Depth] = dsbxDepth->value();
    gcp_.params[GCode::GCodeParams::HathStep] = ui->dsbxHathStep->value();
    gcp_.params[GCode::GCodeParams::Pass] = ui->cbxPass->currentIndex();
    gcp_.params[GCode::GCodeParams::UseAngle] = ui->dsbxAngle->value();
    //    if (ui->rbFast->isChecked()) {
    //        gcp_.params[GCode::GCodeParams::Fast] = true;
    //        gcp_.params[GCode::GCodeParams::AccDistance] = (tool.feedRateMmS() * tool.feedRateMmS()) / (2 * ui->dsbxAcc->value());
    //    }

    creator->setGcp(gcp_);
    creator->addPaths(wPaths);
    creator->addRawPaths(wRawPaths);
    fileCount = 1;
    createToolpath();
}

void HatchingForm::updateName() {
    //    const auto& tool { ui->toolHolder->tool() };
    //    if (tool.type() != Tool::Laser)
    //        ui->rbNormal->setChecked(true);
    //    ui->rbFast->setEnabled(tool.type() == Tool::Laser);
    ui->dsbxHathStep->setMinimum(ui->toolHolder->tool().diameter());
    leName->setText(names[side]);
}

void HatchingForm::updatePixmap() {
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[direction]).pixmap(QSize(150, 150)));
}

void HatchingForm::rb_clicked() {

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

void HatchingForm::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void HatchingForm::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void HatchingForm::on_leName_textChanged(const QString& arg1) { fileName_ = arg1; }

void HatchingForm::editFile(GCode::File* /*file*/) {
}

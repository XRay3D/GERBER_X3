// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/

#include "hatching_form.h"
#include "graphicsview.h"
#include "settings.h"
#include "ui_hatchingform.h"
#include <QMessageBox>

namespace CrossHatch {

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new Ui::HatchingForm)
    , names{tr("Raster On"), tr("Hatching Outside"), tr("Hatching Inside")}
    , pixmaps{
          u"pock_rast_climb"_qs,
          u"pock_rast_conv"_qs,
      } {
    ui->setupUi(content);

    setWindowTitle(tr("CrossHatch Toolpath"));

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

    connect(ui->rbClimb, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &Form::rb_clicked);

    connect(ui->toolHolder, &ToolSelectorForm::updateName, this, &Form::updateName);

    connect(leName, &QLineEdit::textChanged, this, &Form::onNameTextChanged);

    //
}

Form::~Form() {

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

void Form::computePaths() {
    const auto tool{ui->toolHolder->tool()};

    if(!tool.isValid()) {
        tool.errorMessageBox(this);
        return;
    }

    auto gcp = getNewGcp();
    if(!gcp)
        return;

    gcp->setConvent(ui->rbConventional->isChecked());
    gcp->setSide(side);
    gcp->tools.push_back(tool);

    gcp->params[GCode::Params::Depth] = dsbxDepth->value();
    gcp->params[Creator::HathStep] = ui->dsbxHathStep->value();
    gcp->params[Creator::Pass] = ui->cbxPass->currentIndex();
    gcp->params[Creator::UseAngle] = ui->dsbxAngle->value();
    //    if (ui->rbFast->isChecked()) {
    //        gcp_->params[GCode::Params::Fast] = true;
    //        gcp_->params[GCode::Params::AccDistance] = (tool.feedRateMmS() * tool.feedRateMmS()) / (2 * ui->dsbxAcc->value());
    //    }

    fileCount = 1;
    createToolpath(gcp);
}

void Form::updateName() {
    //    const auto& tool { ui->toolHolder->tool() };
    //    if (tool.type() != Tool::Laser)
    //        ui->rbNormal->setChecked(true);
    //    ui->rbFast->setEnabled(tool.type() == Tool::Laser);
    ui->dsbxHathStep->setMinimum(ui->toolHolder->tool().diameter());
    leName->setText(names[side]);
}

void Form::updatePixmap() {
    ui->lblPixmap->setPixmap(QIcon::fromTheme(pixmaps[direction]).pixmap(QSize(150, 150)));
}

void Form::rb_clicked() {

    if(ui->rbOutside->isChecked())
        side = GCode::Outer;
    else if(ui->rbInside->isChecked())
        side = GCode::Inner;

    if(ui->rbClimb->isChecked())
        direction = GCode::Climb;
    else if(ui->rbConventional->isChecked())
        direction = GCode::Conventional;

    updateName();
    updatePixmap();
    updateButtonIconSize();
}

void Form::resizeEvent(QResizeEvent* event) {
    updatePixmap();
    QWidget::resizeEvent(event);
}

void Form::showEvent(QShowEvent* event) {
    updatePixmap();
    QWidget::showEvent(event);
}

void Form::onNameTextChanged(const QString& arg1) { fileName_ = arg1; }

void Form::editFile(GCode::File* /*file*/) { }

} // namespace CrossHatch

#include "moc_hatching_form.cpp"

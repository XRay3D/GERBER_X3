/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/

#include "pocketraster_form.h"
#include "ui_pocketrasterform.h"

#include "graphicsview.h"
#include "settings.h"
#include <QMessageBox>

namespace PocketRaster {

Form::Form(GCode::Plugin* plugin, QWidget* parent)
    : GCode::BaseForm(plugin, new Creator, parent)
    , ui(new Ui::PocketRasterForm)
    , names{tr("Raster On"), tr("Raster Outside"), tr("Raster Inside")} {
    ui->setupUi(content);

    setWindowTitle(tr("Pocket Raster Toolpath"));

    MySettings settings;
    settings.beginGroup("PocketRasterForm");
    settings.getValue(ui->cbxPass);
    settings.getValue(ui->dsbxAcc);
    settings.getValue(ui->dsbxAngle);
    settings.getValue(ui->rbClimb);
    settings.getValue(ui->rbConventional);
    settings.getValue(ui->rbFast);
    settings.getValue(ui->rbInside);
    settings.getValue(ui->rbNormal);
    settings.getValue(ui->rbOutside);
    settings.endGroup();

    rb_clicked();

    connect(ui->rbClimb, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbConventional, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbInside, &QRadioButton::clicked, this, &Form::rb_clicked);
    connect(ui->rbOutside, &QRadioButton::clicked, this, &Form::rb_clicked);

    connect(ui->toolHolder1, &ToolSelectorForm::updateName, this, &Form::updateName);

    connect(leName, &QLineEdit::textChanged, this, &Form::onNameTextChanged);

    //
}

Form::~Form() {

    MySettings settings;
    settings.beginGroup("PocketRasterForm");
    settings.setValue(ui->cbxPass);
    settings.setValue(ui->dsbxAcc);
    settings.setValue(ui->dsbxAngle);
    settings.setValue(ui->rbClimb);
    settings.setValue(ui->rbConventional);
    settings.setValue(ui->rbFast);
    settings.setValue(ui->rbInside);
    settings.setValue(ui->rbNormal);
    settings.setValue(ui->rbOutside);
    settings.endGroup();
    delete ui;
}

void Form::computePaths() {
    const auto tool{ui->toolHolder1->tool()};

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

    gcp->params[Creator::UseAngle] = ui->dsbxAngle->value();
    gcp->params[GCode::Params::Depth] = dsbxDepth->value();
    gcp->params[Creator::Pass] = ui->cbxPass->currentIndex();
    if(ui->rbFast->isChecked()) {
        gcp->params[Creator::Fast] = true;
        gcp->params[Creator::AccDistance] = (tool.feedRate_mmPerSec() * tool.feedRate_mmPerSec()) / (2 * ui->dsbxAcc->value());
    }

    fileCount = 1;
    createToolpath(gcp);
}

void Form::updateName() {
    const auto& tool{ui->toolHolder1->tool()};
    if(tool.type() != Tool::Laser)
        ui->rbNormal->setChecked(true);
    ui->rbFast->setEnabled(tool.type() == Tool::Laser);

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
    updateButtonIconSize();

    updatePixmap();
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

} // namespace PocketRaster

#include "moc_pocketraster_form.cpp"

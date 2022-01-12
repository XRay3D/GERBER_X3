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
#include "gcodepropertiesform.h"
#include "ui_gcodepropertiesform.h"

#include "point.h"
#include "project.h"
#include "settings.h"
#include <QMessageBox>

GCodePropertiesForm::GCodePropertiesForm(QWidget* parent)
    : QWidget(parent)
    , ui(new Ui::GCodePropertiesForm)
{
    ui->setupUi(this);

    connect(ui->dsbxClearence, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    connect(ui->dsbxPlunge, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value > ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
    });

    connect(ui->dsbxGlue, qOverload<double>(&QDoubleSpinBox::valueChanged), App::project(), &Project::setGlue);

    connect(ui->dsbxHomeX, qOverload<double>(&QDoubleSpinBox::valueChanged), Marker::get(Marker::Home), &Marker::setPosX);
    connect(ui->dsbxHomeY, qOverload<double>(&QDoubleSpinBox::valueChanged), Marker::get(Marker::Home), &Marker::setPosY);
    connect(ui->dsbxZeroX, qOverload<double>(&QDoubleSpinBox::valueChanged), Marker::get(Marker::Zero), &Marker::setPosX);
    connect(ui->dsbxZeroY, qOverload<double>(&QDoubleSpinBox::valueChanged), Marker::get(Marker::Zero), &Marker::setPosY);

    if (App::project()) {
        connect(ui->dsbxSpaceX, qOverload<double>(&QDoubleSpinBox::valueChanged), App::project(), &Project::setSpaceX);
        connect(ui->dsbxSpaceY, qOverload<double>(&QDoubleSpinBox::valueChanged), App::project(), &Project::setSpaceY);
        connect(ui->sbxStepsX, qOverload<int>(&QSpinBox::valueChanged), App::project(), &Project::setStepsX);
        connect(ui->sbxStepsY, qOverload<int>(&QSpinBox::valueChanged), App::project(), &Project::setStepsY);
        ui->dsbxSpaceX->setValue(App::project()->spaceX());
        ui->dsbxSpaceY->setValue(App::project()->spaceY());
        ui->sbxStepsX->setValue(App::project()->stepsX());
        ui->sbxStepsY->setValue(App::project()->stepsY());
    }

    connect(ui->dsbxSafeZ, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double value) {
        ui->dsbxSafeZ->setValue(value);
        ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    MySettings settings;
    settings.beginGroup("GCodePropertiesForm");
    settings.getValue(ui->dsbxSafeZ, 20);
    settings.getValue(ui->dsbxClearence, 10);
    settings.getValue(ui->dsbxPlunge, 2);
    settings.getValue(ui->dsbxThickness, 1);
    settings.getValue(ui->dsbxCopperThickness, 0.03);
    settings.getValue(ui->dsbxGlue, 0.05);
    settings.endGroup();

    ui->dsbxHomeX->setValue(Marker::get(Marker::Home)->pos().x());
    ui->dsbxHomeY->setValue(Marker::get(Marker::Home)->pos().y());

    ui->dsbxZeroX->setValue(Marker::get(Marker::Zero)->pos().x());
    ui->dsbxZeroY->setValue(Marker::get(Marker::Zero)->pos().y());

    App::project()->setSafeZ(ui->dsbxSafeZ->value());
    App::project()->setBoardThickness(ui->dsbxThickness->value());
    App::project()->setCopperThickness(ui->dsbxCopperThickness->value());
    App::project()->setClearence(ui->dsbxClearence->value());
    App::project()->setPlunge(ui->dsbxPlunge->value());

    connect(ui->pbOk, &QPushButton::clicked, [this, parent] {
        if (parent
            && ui->dsbxThickness->value() > 0.0
            && ui->dsbxCopperThickness->value() > 0.0
            && ui->dsbxClearence->value() > 0.0
            && ui->dsbxSafeZ->value() > 0.0) {

            parent->close();
            return;
        }
        if (ui->dsbxCopperThickness->value() == 0.0)
            ui->dsbxCopperThickness->flicker();
        if (ui->dsbxThickness->value() == 0.0)
            ui->dsbxThickness->flicker();
        if (ui->dsbxClearence->value() == 0.0)
            ui->dsbxClearence->flicker();
    });

    ui->pbOk->setIcon(QIcon::fromTheme("dialog-ok-apply"));

    if (parent != nullptr)
        parent->setWindowTitle(ui->label->text());

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }

    App::setGCodePropertiesForm(this);
}

GCodePropertiesForm::~GCodePropertiesForm()
{
    App::setGCodePropertiesForm(nullptr);

    if (Marker::get(Marker::Home))
        Marker::get(Marker::Home)->setPos(QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value()));
    if (Marker::get(Marker::Zero))
        Marker::get(Marker::Zero)->setPos(QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value()));

    MySettings settings;
    settings.beginGroup("GCodePropertiesForm");
    settings.setValue(ui->dsbxSafeZ);
    settings.setValue(ui->dsbxClearence);
    settings.setValue(ui->dsbxPlunge);
    settings.setValue(ui->dsbxThickness);
    settings.setValue(ui->dsbxCopperThickness);
    settings.setValue(ui->dsbxGlue);
    settings.endGroup();

    App::project()->setSafeZ(ui->dsbxSafeZ->value());
    App::project()->setBoardThickness(ui->dsbxThickness->value());
    App::project()->setCopperThickness(ui->dsbxCopperThickness->value());
    App::project()->setClearence(ui->dsbxClearence->value());
    App::project()->setPlunge(ui->dsbxPlunge->value());

    delete ui;
}

void GCodePropertiesForm::updatePosDsbxs()
{
    ui->dsbxHomeX->setValue(Marker::get(Marker::Home)->pos().x());
    ui->dsbxHomeY->setValue(Marker::get(Marker::Home)->pos().y());
    ui->dsbxZeroX->setValue(Marker::get(Marker::Zero)->pos().x());
    ui->dsbxZeroY->setValue(Marker::get(Marker::Zero)->pos().y());
}

void GCodePropertiesForm::updateAll()
{
    //ui->dsbxSpaceX;
    //ui->dsbxSpaceY;
    //ui->sbxStepsX;
    //ui->sbxStepsY;
}

void GCodePropertiesForm::on_pbResetHome_clicked()
{
    ui->dsbxHomeX->setValue(0);
    ui->dsbxHomeY->setValue(0);
}

void GCodePropertiesForm::on_pbResetZero_clicked()
{
    ui->dsbxZeroX->setValue(0);
    ui->dsbxZeroY->setValue(0);
}

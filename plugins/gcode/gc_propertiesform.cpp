// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_propertiesform.h"
#include "ui_gcodepropertiesform.h"

#include "gi_point.h"
#include "project.h"
#include "settings.h"
#include <QMessageBox>

namespace GCode {

PropertiesForm::PropertiesForm(QWidget* parent)
    : QWidget{parent}
    , ui(new Ui::GCodePropertiesForm) {
    ui->setupUi(this);

    connect(ui->dsbxClearence, &QDoubleSpinBox::valueChanged, [this](double value) {
        if(value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if(value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    connect(ui->dsbxPlunge, &QDoubleSpinBox::valueChanged, [this](double value) {
        if(value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if(value > ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
    });

    connect(ui->dsbxHomeX, &QDoubleSpinBox::valueChanged, App::homePtr(), &Gi::Marker::setPosX);
    connect(ui->dsbxHomeY, &QDoubleSpinBox::valueChanged, App::homePtr(), &Gi::Marker::setPosY);
    connect(ui->dsbxZeroX, &QDoubleSpinBox::valueChanged, App::zeroPtr(), &Gi::Marker::setPosX);
    connect(ui->dsbxZeroY, &QDoubleSpinBox::valueChanged, App::zeroPtr(), &Gi::Marker::setPosY);

    connect(ui->dsbxGlue, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setGlue);
    connect(ui->dsbxSpaceX, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setSpaceX);
    connect(ui->dsbxSpaceY, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setSpaceY);
    connect(ui->sbxStepsX, qOverload<int>(&QSpinBox::valueChanged), App::projectPtr(), &Project::setStepsX);
    connect(ui->sbxStepsY, qOverload<int>(&QSpinBox::valueChanged), App::projectPtr(), &Project::setStepsY);
    ui->dsbxSpaceX->setValue(App::project().spaceX());
    ui->dsbxSpaceY->setValue(App::project().spaceY());
    ui->sbxStepsX->setValue(App::project().stepsX());
    ui->sbxStepsY->setValue(App::project().stepsY());

    connect(ui->dsbxSafeZ, &QDoubleSpinBox::valueChanged, [this](double value) {
        ui->dsbxSafeZ->setValue(value);
        ui->dsbxSafeZ->setValue(value);
        if(value < ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
        if(value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    MySettings settings;
    settings.beginGroup("PropertiesForm");
    settings.getValue(ui->dsbxSafeZ, 20);
    settings.getValue(ui->dsbxClearence, 10);
    settings.getValue(ui->dsbxPlunge, 2);
    settings.getValue(ui->dsbxThickness, 1);
    settings.getValue(ui->dsbxCopperThickness, 0.03);
    settings.getValue(ui->dsbxGlue, 0.05);
    settings.endGroup();

    ui->dsbxHomeX->setValue(App::home().pos().x());
    ui->dsbxHomeY->setValue(App::home().pos().y());

    ui->dsbxZeroX->setValue(App::zero().pos().x());
    ui->dsbxZeroY->setValue(App::zero().pos().y());

    App::project().setSafeZ(ui->dsbxSafeZ->value());
    App::project().setBoardThickness(ui->dsbxThickness->value());
    App::project().setCopperThickness(ui->dsbxCopperThickness->value());
    App::project().setClearence(ui->dsbxClearence->value());
    App::project().setPlunge(ui->dsbxPlunge->value());

    connect(ui->pbOk, &QPushButton::clicked, [this, parent] {
        if(parent
            && ui->dsbxThickness->value() > 0.0
            && ui->dsbxCopperThickness->value() > 0.0
            && ui->dsbxClearence->value() > 0.0
            && ui->dsbxSafeZ->value() > 0.0) {

            parent->close();
            return;
        }
        if(ui->dsbxCopperThickness->value() == 0.0)
            ui->dsbxCopperThickness->flicker();
        if(ui->dsbxThickness->value() == 0.0)
            ui->dsbxThickness->flicker();
        if(ui->dsbxClearence->value() == 0.0)
            ui->dsbxClearence->flicker();
    });

    ui->pbOk->setIcon(QIcon::fromTheme("dialog-ok-apply"));

    if(parent != nullptr)
        setWindowTitle(ui->label->text());

    for(auto* button: findChildren<QPushButton*>())
        button->setIconSize({16, 16});

    App::setGCodePropertiesForm(this);
}

PropertiesForm::~PropertiesForm() {
    App::setGCodePropertiesForm(nullptr);

    if(App::homePtr())
        App::home().setPos(QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value()));
    if(App::zeroPtr())
        App::zero().setPos(QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value()));

    MySettings settings;
    settings.beginGroup("PropertiesForm");
    settings.setValue(ui->dsbxSafeZ);
    settings.setValue(ui->dsbxClearence);
    settings.setValue(ui->dsbxPlunge);
    settings.setValue(ui->dsbxThickness);
    settings.setValue(ui->dsbxCopperThickness);
    settings.setValue(ui->dsbxGlue);
    settings.endGroup();

    App::project().setSafeZ(ui->dsbxSafeZ->value());
    App::project().setBoardThickness(ui->dsbxThickness->value());
    App::project().setCopperThickness(ui->dsbxCopperThickness->value());
    App::project().setClearence(ui->dsbxClearence->value());
    App::project().setPlunge(ui->dsbxPlunge->value());

    delete ui;
}

void PropertiesForm::updatePosDsbxs() {
    ui->dsbxHomeX->setValue(App::home().pos().x());
    ui->dsbxHomeY->setValue(App::home().pos().y());
    ui->dsbxZeroX->setValue(App::zero().pos().x());
    ui->dsbxZeroY->setValue(App::zero().pos().y());
}

void PropertiesForm::updateAll() {
    // ui->dsbxSpaceX;
    // ui->dsbxSpaceY;
    // ui->sbxStepsX;
    // ui->sbxStepsY;
}

void PropertiesForm::on_pbResetHome_clicked() {
    ui->dsbxHomeX->setValue(0);
    ui->dsbxHomeY->setValue(0);
}

void PropertiesForm::on_pbResetZero_clicked() {
    ui->dsbxZeroX->setValue(0);
    ui->dsbxZeroY->setValue(0);
}

} // namespace GCode

#include "moc_gc_propertiesform.cpp"

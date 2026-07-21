/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gc_propertiesform.h"
#include "ui_gcodepropertiesform.h"

#include "gc_file.h"
#include "gi_point.h"
#include "project.h"
#include "settings.h"
#include <QMessageBox>

namespace GCode {

// Безопасная высота/зазор/глубина подвода влияют на уже сгенерированный текст
// G-кода существующих файлов (File::lines_) — пересобираем его сразу же, а не
// ждём следующего явного "Save Toolpath", иначе экспортированный/показанный
// текст останется устаревшим.

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
    connect(ui->sbxStepsX, &QSpinBox::valueChanged, App::projectPtr(), &Project::setStepsX);
    connect(ui->sbxStepsY, &QSpinBox::valueChanged, App::projectPtr(), &Project::setStepsY);

    connect(ui->dsbxClearence, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setClearence);
    connect(ui->dsbxPlunge, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setPlunge);
    connect(ui->dsbxSafeZ, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setSafeZ);
    connect(ui->dsbxThickness, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setBoardThickness);

    // connect(ui->dsbxClearence, &QDoubleSpinBox::valueChanged, this, regenerateGCodeFiles);
    // connect(ui->dsbxPlunge, &QDoubleSpinBox::valueChanged, this, regenerateGCodeFiles);
    // connect(ui->dsbxSafeZ, &QDoubleSpinBox::valueChanged, this, regenerateGCodeFiles);

    connect(ui->dsbxCopperThickness, &QDoubleSpinBox::valueChanged, App::projectPtr(), &Project::setCopperThickness);

    connect(ui->dsbxSafeZ, &QDoubleSpinBox::valueChanged, [this](double value) {
        ui->dsbxSafeZ->setValue(value);
        ui->dsbxSafeZ->setValue(value);
        if(value < ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
        if(value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    load();

    connect(ui->pbOk, &QPushButton::clicked, [this, parent] {
        regenerateGCodeFiles();
        if(ui->dsbxThickness->value() > 0.0
            && ui->dsbxCopperThickness->value() > 0.0
            && ui->dsbxClearence->value() > 0.0
            && ui->dsbxSafeZ->value() > 0.0) {

            committed_ = true; // подтверждено — hideEvent не должен откатывать изменения
            if(parent) parent->close();
            else close(); // без родительского диалога (докнутая панель) закрываемся сами
            return;
        }
        if(ui->dsbxCopperThickness->value() == 0.0) ui->dsbxCopperThickness->flicker();
        if(ui->dsbxThickness->value() == 0.0) ui->dsbxThickness->flicker();
        if(ui->dsbxClearence->value() == 0.0) ui->dsbxClearence->flicker();
    });

    ui->pbOk->setIcon(QIcon::fromTheme(u"dialog-ok-apply"_s));

    for(auto* button: findChildren<QPushButton*>())
        button->setIconSize({16, 16});

    App::setGCodePropertiesForm(this);
}

PropertiesForm::~PropertiesForm() {
    App::setGCodePropertiesForm(nullptr);
    save();
    delete ui;
}

void PropertiesForm::showEvent(QShowEvent* event) {
    QWidget::showEvent(event);
    committed_ = false;
    snapshot_  = {
         .safeZ           = ui->dsbxSafeZ->value(),
         .clearence       = ui->dsbxClearence->value(),
         .plunge          = ui->dsbxPlunge->value(),
         .thickness       = ui->dsbxThickness->value(),
         .copperThickness = ui->dsbxCopperThickness->value(),
         .glue            = ui->dsbxGlue->value(),
         .spaceX          = ui->dsbxSpaceX->value(),
         .spaceY          = ui->dsbxSpaceY->value(),
         .stepsX          = ui->sbxStepsX->value(),
         .stepsY          = ui->sbxStepsY->value(),
         .homeX           = ui->dsbxHomeX->value(),
         .homeY           = ui->dsbxHomeY->value(),
         .zeroX           = ui->dsbxZeroX->value(),
         .zeroY           = ui->dsbxZeroY->value(),
    };
}

void PropertiesForm::hideEvent(QHideEvent* event) {
    QWidget::hideEvent(event);
    if(!committed_) applySnapshot(snapshot_);
}

void PropertiesForm::applySnapshot(const Snapshot& s) {
    // Порядок важен: safeZ/clearence/plunge подтягивают друг друга через свои
    // valueChanged-обработчики, поэтому восстанавливаем их последними в этой тройке
    // друг за другом, чтобы финальным значением осталось именно значение снимка.
    ui->dsbxThickness->setValue(s.thickness);
    ui->dsbxCopperThickness->setValue(s.copperThickness);
    ui->dsbxGlue->setValue(s.glue);
    ui->dsbxSpaceX->setValue(s.spaceX);
    ui->dsbxSpaceY->setValue(s.spaceY);
    ui->sbxStepsX->setValue(s.stepsX);
    ui->sbxStepsY->setValue(s.stepsY);
    ui->dsbxHomeX->setValue(s.homeX);
    ui->dsbxHomeY->setValue(s.homeY);
    ui->dsbxZeroX->setValue(s.zeroX);
    ui->dsbxZeroY->setValue(s.zeroY);
    ui->dsbxSafeZ->setValue(s.safeZ);
    ui->dsbxClearence->setValue(s.clearence);
    ui->dsbxPlunge->setValue(s.plunge);
}

void PropertiesForm::updatePosDsbxs() {
    regenerateGCodeFiles();
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

void PropertiesForm::load() {
    ui->dsbxSpaceX->setValue(App::project().spaceX());
    ui->dsbxSpaceY->setValue(App::project().spaceY());
    ui->sbxStepsX->setValue(App::project().stepsX());
    ui->sbxStepsY->setValue(App::project().stepsY());

    MySettings settings;
    settings.beginGroup(u"PropertiesForm"_s);
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
}

void PropertiesForm::save() {
    if(App::homePtr()) App::home().setPos(QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value()));
    if(App::zeroPtr()) App::zero().setPos(QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value()));

    MySettings settings;
    settings.beginGroup(u"PropertiesForm"_s);
    settings.setValue(ui->dsbxSafeZ);
    settings.setValue(ui->dsbxClearence);
    settings.setValue(ui->dsbxPlunge);
    settings.setValue(ui->dsbxThickness);
    settings.setValue(ui->dsbxCopperThickness);
    settings.setValue(ui->dsbxGlue);
    settings.endGroup();
    if(!App::projectPtr()) return;
    App::project().setSafeZ(ui->dsbxSafeZ->value());
    App::project().setBoardThickness(ui->dsbxThickness->value());
    App::project().setCopperThickness(ui->dsbxCopperThickness->value());
    App::project().setClearence(ui->dsbxClearence->value());
    App::project().setPlunge(ui->dsbxPlunge->value());
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

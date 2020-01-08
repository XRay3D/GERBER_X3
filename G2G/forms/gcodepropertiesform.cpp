#include "gcodepropertiesform.h"
#include "ui_gcodepropertiesform.h"

#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <mainwindow.h>
#include <project.h>
#include <scene.h>

GCodePropertiesForm* GCodePropertiesForm::self = nullptr;

double GCodePropertiesForm::safeZ;
double GCodePropertiesForm::boardThickness;
double GCodePropertiesForm::copperThickness;
double GCodePropertiesForm::clearence;
double GCodePropertiesForm::plunge;
double GCodePropertiesForm::glue;

GCodePropertiesForm::GCodePropertiesForm(QWidget* prnt)
    : QWidget(prnt)
    , ui(new Ui::GCodePropertiesForm)
{
    ui->setupUi(this);

    connect(ui->dsbxClearence, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    connect(ui->dsbxPlunge, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        if (value > ui->dsbxSafeZ->value())
            ui->dsbxSafeZ->setValue(value);
        if (value > ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
    });

    connect(ui->dsbxGlue, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [](double val) { glue = val; });
    connect(ui->dsbxHomeX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [](double val) { Marker::get(Marker::Home)->setPosX(val); });
    connect(ui->dsbxHomeY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [](double val) { Marker::get(Marker::Home)->setPosY(val); });
    connect(ui->dsbxZeroX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [](double val) { Marker::get(Marker::Zero)->setPosX(val); });
    connect(ui->dsbxZeroY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [](double val) { Marker::get(Marker::Zero)->setPosY(val); });

    if (Project::instance()) {
        connect(ui->dsbxSpasingX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), Project::instance(), &Project::setSpasingX);
        connect(ui->dsbxSpasingY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), Project::instance(), &Project::setSpasingY);
        connect(ui->sbxStepsX, QOverload<int>::of(&QSpinBox::valueChanged), Project::instance(), &Project::setStepsX);
        connect(ui->sbxStepsY, QOverload<int>::of(&QSpinBox::valueChanged), Project::instance(), &Project::setStepsY);
        ui->dsbxSpasingX->setValue(Project::instance()->spasingX());
        ui->dsbxSpasingY->setValue(Project::instance()->spasingY());
        ui->sbxStepsX->setValue(Project::instance()->stepsX());
        ui->sbxStepsY->setValue(Project::instance()->stepsY());
    }

    connect(ui->dsbxSafeZ, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
        ui->dsbxSafeZ->setValue(value);
        ui->dsbxSafeZ->setValue(value);
        if (value < ui->dsbxClearence->value())
            ui->dsbxClearence->setValue(value);
        if (value < ui->dsbxPlunge->value())
            ui->dsbxPlunge->setValue(value);
    });

    QSettings settings;
    settings.beginGroup("GCodePropertiesForm");
    ui->dsbxSafeZ->setValue(settings.value("dsbxSafeZ", 20).toDouble());
    ui->dsbxClearence->setValue(settings.value("dsbxClearence", 10).toDouble());
    ui->dsbxPlunge->setValue(settings.value("dsbxPlunge", 2).toDouble());
    ui->dsbxThickness->setValue(settings.value("dsbxThickness", 1).toDouble());
    ui->dsbxCopperThickness->setValue(settings.value("dsbxCopperThickness", 0.03).toDouble());
    ui->dsbxGlue->setValue(settings.value("dsbxGlue", 0.05).toDouble());
    settings.endGroup();

    ui->dsbxHomeX->setValue(Marker::get(Marker::Home)->pos().x());
    ui->dsbxHomeY->setValue(Marker::get(Marker::Home)->pos().y());

    ui->dsbxZeroX->setValue(Marker::get(Marker::Zero)->pos().x());
    ui->dsbxZeroY->setValue(Marker::get(Marker::Zero)->pos().y());

    safeZ = ui->dsbxSafeZ->value();
    boardThickness = ui->dsbxThickness->value();
    copperThickness = ui->dsbxCopperThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

    connect(ui->pbOk, &QPushButton::clicked, [this] {
        if (this->parent()
            && ui->dsbxThickness->value() > 0.0
            && ui->dsbxCopperThickness->value() > 0.0
            && ui->dsbxClearence->value() > 0.0
            && ui->dsbxSafeZ->value() > 0.0) {
            if (parent())
                if (auto* w = dynamic_cast<QWidget*>(parent()); w)
                    w->close();
            return;
        }
        if (ui->dsbxCopperThickness->value() == 0.0)
            ui->dsbxCopperThickness->flicker();
        if (ui->dsbxThickness->value() == 0.0)
            ui->dsbxThickness->flicker();
        if (ui->dsbxClearence->value() == 0.0)
            ui->dsbxClearence->flicker();
    });
    if (prnt != nullptr)
        prnt->setWindowTitle(ui->label->text());
    self = this;

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }
}

GCodePropertiesForm::~GCodePropertiesForm()
{
    self = nullptr;
    if (Marker::get(Marker::Home))
        Marker::get(Marker::Home)->setPos(QPointF(ui->dsbxHomeX->value(), ui->dsbxHomeY->value()));
    if (Marker::get(Marker::Zero))
        Marker::get(Marker::Zero)->setPos(QPointF(ui->dsbxZeroX->value(), ui->dsbxZeroY->value()));

    QSettings settings;
    settings.beginGroup("GCodePropertiesForm");
    settings.setValue("dsbxSafeZ", ui->dsbxSafeZ->value());
    settings.setValue("dsbxClearence", ui->dsbxClearence->value());
    settings.setValue("dsbxPlunge", ui->dsbxPlunge->value());
    settings.setValue("dsbxThickness", ui->dsbxThickness->value());
    settings.setValue("dsbxCopperThickness", ui->dsbxCopperThickness->value());
    settings.setValue("dsbxGlue", ui->dsbxGlue->value());
    settings.endGroup();

    safeZ = ui->dsbxSafeZ->value();
    boardThickness = ui->dsbxThickness->value();
    copperThickness = ui->dsbxCopperThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

    delete ui;
}

void GCodePropertiesForm::updatePosDsbxs()
{
    if (!self)
        return;
    self->ui->dsbxHomeX->setValue(Marker::get(Marker::Home)->pos().x());
    self->ui->dsbxHomeY->setValue(Marker::get(Marker::Home)->pos().y());
    self->ui->dsbxZeroX->setValue(Marker::get(Marker::Zero)->pos().x());
    self->ui->dsbxZeroY->setValue(Marker::get(Marker::Zero)->pos().y());
}

void GCodePropertiesForm::updateAll()
{
    if (!self)
        return;
    self->ui->dsbxSpasingX;
    self->ui->dsbxSpasingY;
    self->ui->sbxStepsX;
    self->ui->sbxStepsY;
}

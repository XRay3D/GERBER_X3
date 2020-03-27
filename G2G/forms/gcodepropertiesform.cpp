#include "gcodepropertiesform.h"
#include "ui_gcodepropertiesform.h"

#include <QDockWidget>
#include <QSettings>
#include <QTimer>
#include <mainwindow.h>
#include <settings.h>
#include <project.h>
#include <scene.h>

GCodePropertiesForm* GCodePropertiesForm::m_instance = nullptr;

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
    if (m_instance) {
        QMessageBox::critical(nullptr, "Err", "You cannot create class GCodePropertiesForm more than 2 times!!!");
        exit(1);
    }
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
        connect(ui->dsbxSpaceX, QOverload<double>::of(&QDoubleSpinBox::valueChanged), Project::instance(), &Project::setSpaceX);
        connect(ui->dsbxSpaceY, QOverload<double>::of(&QDoubleSpinBox::valueChanged), Project::instance(), &Project::setSpaceY);
        connect(ui->sbxStepsX, QOverload<int>::of(&QSpinBox::valueChanged), Project::instance(), &Project::setStepsX);
        connect(ui->sbxStepsY, QOverload<int>::of(&QSpinBox::valueChanged), Project::instance(), &Project::setStepsY);
        ui->dsbxSpaceX->setValue(Project::instance()->spaceX());
        ui->dsbxSpaceY->setValue(Project::instance()->spaceY());
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

    for (QPushButton* button : findChildren<QPushButton*>()) {
        button->setIconSize({ 16, 16 });
    }
    m_instance = this;
}

GCodePropertiesForm::~GCodePropertiesForm()
{
    m_instance = nullptr;
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

    safeZ = ui->dsbxSafeZ->value();
    boardThickness = ui->dsbxThickness->value();
    copperThickness = ui->dsbxCopperThickness->value();
    clearence = ui->dsbxClearence->value();
    plunge = ui->dsbxPlunge->value();

    delete ui;
}

void GCodePropertiesForm::updatePosDsbxs()
{
    if (!m_instance)
        return;
    m_instance->ui->dsbxHomeX->setValue(Marker::get(Marker::Home)->pos().x());
    m_instance->ui->dsbxHomeY->setValue(Marker::get(Marker::Home)->pos().y());
    m_instance->ui->dsbxZeroX->setValue(Marker::get(Marker::Zero)->pos().x());
    m_instance->ui->dsbxZeroY->setValue(Marker::get(Marker::Zero)->pos().y());
}

void GCodePropertiesForm::updateAll()
{
    if (!m_instance)
        return;
    m_instance->ui->dsbxSpaceX;
    m_instance->ui->dsbxSpaceY;
    m_instance->ui->sbxStepsX;
    m_instance->ui->sbxStepsY;
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

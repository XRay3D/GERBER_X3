// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
#include "excellondialog.h"
#include "ui_excellondialog.h"

#include "exfile.h"
#include "graphicsview.h"
#include "scene.h"
#include <cmath>

#include "exsettingstab.h"


using namespace Excellon;

bool ExcellonDialog::showed() { return m_showed; }

ExcellonDialog::ExcellonDialog(Excellon::File* file)
    : ui(new Ui::ExcellonDialog)
    , m_file(file)
    , m_format(file->format())
    , m_tmpFormat(file->format())
{
    m_showed = true;

    ui->setupUi(this);
    setObjectName("ExcellonDialog");

    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowTitle(file->shortName());

    ui->sbxInteger->setValue(m_format.integer);
    ui->sbxDecimal->setValue(m_format.decimal);

    ui->dsbxX->setValue(m_format.offsetPos.x());
    ui->dsbxY->setValue(m_format.offsetPos.y());

    ui->rbInches->setChecked(!m_format.unitMode);
    ui->rbMillimeters->setChecked(m_format.unitMode);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &ExcellonDialog::rejectFormat);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &ExcellonDialog::acceptFormat);

    connect(ui->dsbxX, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double val) { m_tmpFormat.offsetPos.setX(val); m_file->setFormat(m_tmpFormat); });
    connect(ui->dsbxY, qOverload<double>(&QDoubleSpinBox::valueChanged), [&](double val) { m_tmpFormat.offsetPos.setY(val); m_file->setFormat(m_tmpFormat); });

    connect(ui->sbxInteger, qOverload<int>(&QSpinBox::valueChanged), [&] { updateFormat(); });
    connect(ui->sbxDecimal, qOverload<int>(&QSpinBox::valueChanged), [&] { updateFormat(); });

    connect(ui->rbInches, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbLeading, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbMillimeters, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbTrailing, &QRadioButton::toggled, [&] { updateFormat(); });

    ui->rbLeading->setChecked(!m_format.zeroMode);
    ui->rbTrailing->setChecked(m_format.zeroMode);

    on_pbStep_clicked();
}

ExcellonDialog::~ExcellonDialog()
{
    m_showed = false;
    resetFormat();
    delete ui;
}

void ExcellonDialog::on_pbStep_clicked()
{
    if (++m_step == 4)
        m_step = -1;
    const double singleStep = pow(0.1, m_step);
    ui->pbStep->setText("x" + QString::number(singleStep));
    ui->dsbxX->setSingleStep(singleStep);
    ui->dsbxY->setSingleStep(singleStep);
}

void ExcellonDialog::on_pushButton_clicked()
{
    QPair<QPointF, QPointF> pair;
    int c = 0;
    for (QGraphicsItem* item : App::scene()->selectedItems()) {
        if (static_cast<GiType>(item->type()) == GiType::Drill) {
            pair.first = item->boundingRect().center();
            ++c;
        }
        if (static_cast<GiType>(item->type()) != GiType::Drill) {
            pair.second = item->boundingRect().center();
            ++c;
        }
        if (c == 2) {
            QPointF p(pair.second - (pair.first - m_tmpFormat.offsetPos));
            if (QLineF(pair.first, pair.second).length() < 0.001) // 1 uMetr
                return;
            ui->dsbxX->setValue(p.x());
            ui->dsbxY->setValue(p.y());
            App::graphicsView()->zoomFit();
            return;
        }
    }
}

void ExcellonDialog::updateFormat()
{

    m_tmpFormat.offsetPos.rx() = ui->dsbxX->value();
    m_tmpFormat.offsetPos.ry() = ui->dsbxY->value();

    m_tmpFormat.integer = ui->sbxInteger->value();
    m_tmpFormat.decimal = ui->sbxDecimal->value();

    m_tmpFormat.unitMode = static_cast<UnitMode>(ui->rbMillimeters->isChecked());
    m_tmpFormat.zeroMode = static_cast<ZeroMode>(ui->rbTrailing->isChecked());

    m_file->setFormat(m_tmpFormat);
    App::graphicsView()->zoomFit();
}

void ExcellonDialog::acceptFormat()
{
    accepted = true;
    App::graphicsView()->zoomFit();
}

void ExcellonDialog::rejectFormat() { deleteLater(); }

void ExcellonDialog::resetFormat()
{
    if (accepted)
        return;
    m_file->setFormat(m_format);
    App::graphicsView()->zoomFit();
}

void ExcellonDialog::closeEvent(QCloseEvent* event) { deleteLater(); }

void ExcellonDialog::hideEvent(QHideEvent* event) { deleteLater(); }

void ExcellonDialog::on_pbSetAsDefault_clicked()
{
    QSettings settings;
    settings.beginGroup("Excellon");

    settings.setValue("dsbxX", m_tmpFormat.offsetPos.x());
    settings.setValue("dsbxY", m_tmpFormat.offsetPos.y());

    settings.setValue("rbInches", m_tmpFormat.unitMode == Inches);
    settings.setValue("rbMillimeters", m_tmpFormat.unitMode == Millimeters);

    settings.setValue("rbLeading", m_tmpFormat.zeroMode == LeadingZeros);
    settings.setValue("rbTrailing", m_tmpFormat.zeroMode == TrailingZeros);

    settings.setValue("sbxDecimal", m_tmpFormat.decimal);
    settings.setValue("sbxInteger", m_tmpFormat.integer);
    settings.endGroup();

    Settings::setformat(m_tmpFormat);
}

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "excellondialog.h"
#include "ui_excellondialog.h"

#include "exfile.h"
#include "graphicsview.h"
#include "scene.h"
#include <cmath>

#include "leakdetector.h"

using namespace Excellon;

ExcellonDialog::ExcellonDialog(Excellon::File* file)
    : ui(new Ui::ExcellonDialog)
    , m_file(file)
    , m_format(file->format())
    , m_tmpFormat(file->format())
{
    ui->setupUi(this);
    setObjectName("ExcellonDialog");

    setWindowFlag(Qt::WindowStaysOnTopHint);

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

    //    connect(ui->rbLeading, &QRadioButton::toggled, [&](bool checked) { ui->sbxInteger->setEnabled(checked); });
    //    connect(ui->rbTrailing, &QRadioButton::toggled, [&](bool checked) { ui->sbxDecimal->setEnabled(checked); });

    //    connect(ui->sbxInteger, qOverload<int>(&QSpinBox::valueChanged), [&](int val) { ui->sbxDecimal->setValue(8 - val); });
    //    connect(ui->sbxDecimal, qOverload<int>(&QSpinBox::valueChanged), [&](int val) { ui->sbxInteger->setValue(8 - val); });

    ui->rbLeading->setChecked(!m_format.zeroMode);
    ui->rbTrailing->setChecked(m_format.zeroMode);

    on_pbStep_clicked();
}

ExcellonDialog::~ExcellonDialog()
{
    qDebug("~ExcellonDialog()");
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

void ExcellonDialog::updateFormat()
{
    qDebug("updateFormat");
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
    App::graphicsView()->zoomFit();
    deleteLater();
}

void ExcellonDialog::rejectFormat()
{
    App::graphicsView()->zoomFit();
    //if (Project::contains(m_file))
    m_file->setFormat(m_format);
    deleteLater();
}

void ExcellonDialog::closeEvent(QCloseEvent* event)
{
    //if (Project::contains(m_file))
    m_file->setFormat(m_format);
    deleteLater();
    QDialog::closeEvent(event);
}

void ExcellonDialog::on_pushButton_clicked()
{
    QPair<QPointF, QPointF> pair;
    int c = 0;
    for (QGraphicsItem* item : App::scene()->selectedItems()) {
        if (item->type() == static_cast<int>(GiType::Drill)) {
            pair.first = item->boundingRect().center();
            ++c;
        }
        if (item->type() != static_cast<int>(GiType::Drill)) {
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

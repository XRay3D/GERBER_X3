// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "ex_formatdialog.h"
#include "ui_excellondialog.h"

#include "ex_file.h"
#include "graphicsview.h"
#include <cmath>

#include "ex_settingstab.h"

namespace Excellon {

FormatDialog::FormatDialog(Excellon::File* file)
    : ui(new Ui::ExcellonDialog)
    , file_(file)
    , format_(file->format())
    , tmpFormat_(file->format()) {
    showed_ = true;

    ui->setupUi(this);
    setObjectName("ExcellonDialog");

    setWindowFlag(Qt::WindowStaysOnTopHint);
    setWindowTitle(file->shortName());

    ui->sbxInteger->setValue(format_.integer);
    ui->sbxDecimal->setValue(format_.decimal);

    ui->rbInches->setChecked(!format_.unitMode);
    ui->rbMillimeters->setChecked(format_.unitMode);

    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &FormatDialog::rejectFormat);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &FormatDialog::acceptFormat);

    connect(ui->sbxInteger, qOverload<int>(&QSpinBox::valueChanged), [&] { updateFormat(); });
    connect(ui->sbxDecimal, qOverload<int>(&QSpinBox::valueChanged), [&] { updateFormat(); });

    connect(ui->rbInches, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbLeading, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbMillimeters, &QRadioButton::toggled, [&] { updateFormat(); });
    connect(ui->rbTrailing, &QRadioButton::toggled, [&] { updateFormat(); });

    ui->rbLeading->setChecked(!format_.zeroMode);
    ui->rbTrailing->setChecked(format_.zeroMode);
}

FormatDialog::~FormatDialog() {
    showed_ = false;
    resetFormat();
    delete ui;
}

void FormatDialog::on_pushButton_clicked() {
    QPair<QPointF, QPointF> pair;
    int c = 0;
    for (QGraphicsItem* item : App::graphicsView()->scene()->selectedItems()) {
        if (item->type() == GiType::Drill) {
            pair.first = item->boundingRect().center();
            ++c;
        }
        if (item->type() != GiType::Drill) {
            pair.second = item->boundingRect().center();
            ++c;
        }
        if (c == 2) {
            QPointF p(pair.second - pair.first);
            if (QLineF(pair.first, pair.second).length() < 0.001) // 1 uMetr
                return;

            App::graphicsView()->zoomFit();
            return;
        }
    }
}

void FormatDialog::updateFormat() {
    tmpFormat_.integer = ui->sbxInteger->value();
    tmpFormat_.decimal = ui->sbxDecimal->value();

    tmpFormat_.unitMode = static_cast<UnitMode>(ui->rbMillimeters->isChecked());
    tmpFormat_.zeroMode = static_cast<ZeroMode>(ui->rbTrailing->isChecked());

    file_->setFormat(tmpFormat_);
    App::graphicsView()->zoomFit();
}

void FormatDialog::acceptFormat() {
    accepted = true;
    App::graphicsView()->zoomFit();
}

void FormatDialog::rejectFormat() { deleteLater(); }

void FormatDialog::resetFormat() {
    if (accepted)
        return;
    file_->setFormat(format_);
    App::graphicsView()->zoomFit();
}

void FormatDialog::closeEvent(QCloseEvent* event) { deleteLater(); }

void FormatDialog::hideEvent(QHideEvent* event) { deleteLater(); }

void FormatDialog::on_pbSetAsDefault_clicked() {
    QSettings settings;
    settings.beginGroup("Excellon");

    settings.setValue("rbInches", tmpFormat_.unitMode == Inches);
    settings.setValue("rbMillimeters", tmpFormat_.unitMode == Millimeters);

    settings.setValue("rbLeading", tmpFormat_.zeroMode == LeadingZeros);
    settings.setValue("rbTrailing", tmpFormat_.zeroMode == TrailingZeros);

    settings.setValue("sbxDecimal", tmpFormat_.decimal);
    settings.setValue("sbxInteger", tmpFormat_.integer);
    settings.endGroup();

    Settings::setformat(tmpFormat_);
}

} // namespace Excellon

#include "moc_ex_formatdialog.cpp"

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
 *******************************************************************************/
#include "topor_settingstab.h"
#include "settings.h"
#include <QtWidgets>
#include <doublespinbox.h>

namespace TopoR {
ExSettingsTab::ExSettingsTab(QWidget* parent)
    : SettingsTabInterface(parent) {
    setObjectName(QString::fromUtf8("tabExcellon"));

    auto vlayTab = new QVBoxLayout(this);
    vlayTab->setContentsMargins(6, 6, 6, 6);

    {
        auto gboxDefaulValues = new QGroupBox(this);

        gboxDefaulValues->setTitle(QApplication::translate("ExcellonDialog", "Default values", nullptr));
        auto grbxUnits = new QGroupBox(gboxDefaulValues);
        {
            grbxUnits->setTitle(QApplication::translate("ExcellonDialog", "Units", nullptr));
            rbInches = new QRadioButton(grbxUnits);
            rbInches->setObjectName(QString::fromUtf8("rbInches"));

            rbMillimeters = new QRadioButton(grbxUnits);
            rbMillimeters->setObjectName(QString::fromUtf8("rbMillimeters"));

            auto vlay = new QVBoxLayout(grbxUnits);
            vlay->setContentsMargins(6, 6, 6, 6);
            vlay->addWidget(rbInches);
            vlay->addWidget(rbMillimeters);
        }
        auto grbxZeroes = new QGroupBox(gboxDefaulValues);
        {
            grbxZeroes->setTitle(QApplication::translate("ExcellonDialog", "Zeroes", nullptr));

            rbLeading = new QRadioButton(grbxZeroes);
            rbLeading->setObjectName(QString::fromUtf8("rbLeading"));

            rbTrailing = new QRadioButton(grbxZeroes);
            rbTrailing->setObjectName(QString::fromUtf8("rbTrailing"));

            auto vlay = new QVBoxLayout(grbxZeroes);
            vlay->setContentsMargins(6, 6, 6, 6);
            vlay->addWidget(rbLeading);
            vlay->addWidget(rbTrailing);
        }
        auto grbxFormat = new QGroupBox(gboxDefaulValues);
        {
            grbxFormat->setTitle(QApplication::translate("ExcellonDialog", "Format", nullptr));

            sbxInteger = new QSpinBox(grbxFormat);
            sbxInteger->setObjectName(QString::fromUtf8("sbxInteger"));
            sbxInteger->setWrapping(false);
            sbxInteger->setAlignment(Qt::AlignCenter);
            sbxInteger->setProperty("showGroupSeparator", QVariant(false));
            sbxInteger->setMinimum(1);
            sbxInteger->setMaximum(8);

            sbxDecimal = new QSpinBox(grbxFormat);
            sbxDecimal->setObjectName(QString::fromUtf8("sbxDecimal"));
            sbxDecimal->setAlignment(Qt::AlignCenter);
            sbxDecimal->setMinimum(1);
            sbxDecimal->setMaximum(8);

            auto hlay = new QHBoxLayout(grbxFormat);
            hlay->setContentsMargins(6, 6, 6, 6);
            hlay->addWidget(sbxInteger);
            hlay->addWidget(new QLabel(QApplication::translate("ExcellonDialog", ":", nullptr), grbxFormat));
            hlay->addWidget(sbxDecimal);

            hlay->setStretch(0, 1);
            hlay->setStretch(2, 1);
        }

        auto grbxOffset = new QGroupBox(gboxDefaulValues);
        {
            grbxOffset->setTitle(QApplication::translate("ExcellonDialog", "Offset", nullptr));

            dsbxX = new DoubleSpinBox(grbxOffset);
            dsbxX->setObjectName(QString::fromUtf8("dsbxX"));
            dsbxX->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
            dsbxX->setDecimals(4);
            dsbxX->setMinimum(-1000.0);
            dsbxX->setMaximum(1000.0);

            dsbxY = new DoubleSpinBox(grbxOffset);
            dsbxY->setObjectName(QString::fromUtf8("dsbxY"));
            dsbxY->setAlignment(Qt::AlignLeading | Qt::AlignLeft | Qt::AlignVCenter);
            dsbxY->setDecimals(4);
            dsbxY->setMinimum(-1000.0);
            dsbxY->setMaximum(1000.0);

            auto formLay = new QFormLayout(grbxOffset);
            formLay->setContentsMargins(6, 6, 6, 6);
            formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "X:", nullptr), grbxOffset), dsbxX);
            formLay->addRow(new QLabel(QApplication::translate("ExcellonDialog", "Y:", nullptr), grbxOffset), dsbxY);
        }

        auto vlay = new QVBoxLayout(gboxDefaulValues);
        vlay->setContentsMargins(6, 6, 6, 6);
        vlay->addWidget(grbxUnits);
        vlay->addWidget(grbxZeroes);
        vlay->addWidget(grbxFormat);
        vlay->addWidget(grbxOffset);
        vlayTab->addWidget(gboxDefaulValues);
    }
    vlayTab->addItem(new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding));

    rbInches->setText(QApplication::translate("ExcellonDialog", "Inches", nullptr));
    rbLeading->setText(QApplication::translate("ExcellonDialog", "Leading", nullptr));
    rbMillimeters->setText(QApplication::translate("ExcellonDialog", "Millimeters", nullptr));
    rbTrailing->setText(QApplication::translate("ExcellonDialog", "Trailing", nullptr));
}

ExSettingsTab::~ExSettingsTab() { }

void ExSettingsTab::readSettings(MySettings& settings) {
    settings.beginGroup("Excellon");

    settings.endGroup();
}

void ExSettingsTab::writeSettings(MySettings& settings) {
    settings.beginGroup("Excellon");

    settings.setValue(rbInches);

    settings.setValue(rbLeading);

    settings.endGroup();
}

} // namespace TopoR

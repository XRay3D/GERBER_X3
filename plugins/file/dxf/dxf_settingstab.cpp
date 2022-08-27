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

#include "dxf_settingstab.h"

#include <QtWidgets>

namespace Dxf {

SettingsTab::SettingsTab(QWidget* parent)
    : SettingsTabInterface(parent) {
    setObjectName(QString::fromUtf8("tabDxf"));
    auto verticalLayout = new QVBoxLayout(this);
    verticalLayout->setObjectName(QString::fromUtf8("verticalLayout_9"));
    verticalLayout->setContentsMargins(6, 6, 6, 6);

    auto groupBox = new QGroupBox(this);
    groupBox->setObjectName(QString::fromUtf8("groupBox_3"));

    auto formLayout = new QFormLayout(groupBox);
    formLayout->setObjectName(QString::fromUtf8("formLayout_4"));
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    formLayout->setContentsMargins(6, 6, 6, 6);
    // DefaultFont
    auto labelDefaultFont = new QLabel(groupBox);
    labelDefaultFont->setObjectName(QString::fromUtf8("labelDefaultFont"));
    formLayout->setWidget(0, QFormLayout::LabelRole, labelDefaultFont);

    fcbxDxfDefaultFont = new QFontComboBox(groupBox);
    fcbxDxfDefaultFont->setObjectName(QString::fromUtf8("fcbxDxfDefaultFont"));
    formLayout->setWidget(0, QFormLayout::FieldRole, fcbxDxfDefaultFont);
    // Bold Font
    auto labelBoldFont = new QLabel(groupBox);
    labelBoldFont->setObjectName(QString::fromUtf8("labelBoldFont"));
    formLayout->setWidget(1, QFormLayout::LabelRole, labelBoldFont);

    chbxBoldFont = new QCheckBox(" ", groupBox);
    chbxBoldFont->setObjectName(QString::fromUtf8("chbxDxfBoldFont"));
    formLayout->setWidget(1, QFormLayout::FieldRole, chbxBoldFont);
    // Italic Font
    auto labelItalicFont = new QLabel(groupBox);
    labelItalicFont->setObjectName(QString::fromUtf8("labelItalicFont"));
    formLayout->setWidget(2, QFormLayout::LabelRole, labelItalicFont);

    chbxItalicFont = new QCheckBox(" ", groupBox);
    chbxItalicFont->setObjectName(QString::fromUtf8("chbxDxfItalicFont"));
    formLayout->setWidget(2, QFormLayout::FieldRole, chbxItalicFont);
    // Override Fonts
    auto labelOverrideFonts = new QLabel(groupBox);
    labelOverrideFonts->setObjectName(QString::fromUtf8("labelOverrideFonts"));
    formLayout->setWidget(3, QFormLayout::LabelRole, labelOverrideFonts);

    chbxOverrideFonts = new QCheckBox(" ", groupBox);
    chbxOverrideFonts->setObjectName(QString::fromUtf8("chbxDxfOverrideFonts"));
    formLayout->setWidget(3, QFormLayout::FieldRole, chbxOverrideFonts);

    verticalLayout->addWidget(groupBox);
    auto verticalSpacer = new QSpacerItem(20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding);
    verticalLayout->addItem(verticalSpacer);

    chbxBoldFont->setText(QString());
    chbxItalicFont->setText(QString());
    chbxOverrideFonts->setText(QString());

    groupBox->setTitle(QApplication::translate("SettingsDialog", "Font", nullptr));

    labelBoldFont->setText(QApplication::translate("SettingsDialog", "Bold:", nullptr));
    labelDefaultFont->setText(QApplication::translate("SettingsDialog", "Default Font:", nullptr));
    labelItalicFont->setText(QApplication::translate("SettingsDialog", "Italic:", nullptr));
    labelOverrideFonts->setText(QApplication::translate("SettingsDialog", "Override declared fonts in DXF:", nullptr));
}

SettingsTab::~SettingsTab() { }

void SettingsTab::readSettings(MySettings& settings) {
    settings.beginGroup("Dxf");
    defaultFont_ = settings.getValue(fcbxDxfDefaultFont, "Arial");
    boldFont_ = settings.getValue(chbxBoldFont, false);
    italicFont_ = settings.getValue(chbxItalicFont, false);
    overrideFonts_ = settings.getValue(chbxOverrideFonts, false);
    settings.endGroup();
}

void SettingsTab::writeSettings(MySettings& settings) {
    settings.beginGroup("Dxf");
    defaultFont_ = settings.setValue(fcbxDxfDefaultFont);
    boldFont_ = settings.setValue(chbxBoldFont);
    italicFont_ = settings.setValue(chbxItalicFont);
    overrideFonts_ = settings.setValue(chbxOverrideFonts);
    settings.endGroup();
}

} // namespace Dxf

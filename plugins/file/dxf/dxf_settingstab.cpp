/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/

#include "dxf_settingstab.h"

#include <QtWidgets>

namespace Dxf {

SettingsTab::SettingsTab(QWidget* parent)
    : AbstractFileSettings{parent} {
    setObjectName(u"tabDxf"_s);
    auto verticalLayout = new QVBoxLayout{this};
    verticalLayout->setObjectName(u"verticalLayout"_s);
    verticalLayout->setContentsMargins(6, 6, 6, 6);

    auto groupBox = new QGroupBox{this};
    groupBox->setObjectName(u"groupBox"_s);

    auto formLayout = new QFormLayout{groupBox};
    formLayout->setObjectName(u"formLayout"_s);
    formLayout->setLabelAlignment(Qt::AlignRight | Qt::AlignTrailing | Qt::AlignVCenter);
    formLayout->setContentsMargins(6, 6, 6, 6);
    // DefaultFont
    auto labelDefaultFont = new QLabel{groupBox};
    labelDefaultFont->setObjectName(u"labelDefaultFont"_s);
    formLayout->setWidget(0, QFormLayout::LabelRole, labelDefaultFont);

    fcbxDxfDefaultFont = new QFontComboBox{groupBox};
    fcbxDxfDefaultFont->setObjectName(u"fcbxDxfDefaultFont"_s);
    formLayout->setWidget(0, QFormLayout::FieldRole, fcbxDxfDefaultFont);
    // Bold Font
    auto labelBoldFont = new QLabel{groupBox};
    labelBoldFont->setObjectName(u"labelBoldFont"_s);
    formLayout->setWidget(1, QFormLayout::LabelRole, labelBoldFont);

    chbxBoldFont = new QCheckBox{" ", groupBox};
    chbxBoldFont->setObjectName(u"chbxBoldFont"_s);
    formLayout->setWidget(1, QFormLayout::FieldRole, chbxBoldFont);
    // Italic Font
    auto labelItalicFont = new QLabel{groupBox};
    labelItalicFont->setObjectName(u"labelItalicFont"_s);
    formLayout->setWidget(2, QFormLayout::LabelRole, labelItalicFont);

    chbxItalicFont = new QCheckBox{" ", groupBox};
    chbxItalicFont->setObjectName(u"chbxItalicFont"_s);
    formLayout->setWidget(2, QFormLayout::FieldRole, chbxItalicFont);
    // Override Fonts
    auto labelOverrideFonts = new QLabel{groupBox};
    labelOverrideFonts->setObjectName(u"labelOverrideFonts"_s);
    formLayout->setWidget(3, QFormLayout::LabelRole, labelOverrideFonts);

    chbxOverrideFonts = new QCheckBox{" ", groupBox};
    chbxOverrideFonts->setObjectName(u"chbxOverrideFonts"_s);
    formLayout->setWidget(3, QFormLayout::FieldRole, chbxOverrideFonts);

    verticalLayout->addWidget(groupBox);
    auto verticalSpacer = new QSpacerItem{20, 40, QSizePolicy::Minimum, QSizePolicy::Expanding};
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

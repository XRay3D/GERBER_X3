#pragma once

#include "abstract_fileplugin.h"
#include "ex_types.h"
#include <QWidget>

namespace Excellon {

class ExSettingsTab : public AbstractFileSettings, Settings {
    friend class ExcellonDialog;

    DoubleSpinBox* dsbxX;
    DoubleSpinBox* dsbxY;
    QRadioButton* rbInches;
    QRadioButton* rbLeading;
    QRadioButton* rbMillimeters;
    QRadioButton* rbTrailing;
    QSpinBox* sbxDecimal;
    QSpinBox* sbxInteger;

    QLineEdit* leParseDecimalAndInteger;
    QLineEdit* leParseUnit;
    QLineEdit* leParseZero;

public:
    ExSettingsTab(QWidget* parent = nullptr);

    virtual ~ExSettingsTab() override;

    virtual void readSettings(MySettings& settings) override;

    virtual void writeSettings(MySettings& settings) override;
};

} // namespace Excellon

#pragma once

#include "dxf_types.h"

#include "abstract_fileplugin.h"
#include <QWidget>

class QCheckBox;
class QFontComboBox;

namespace Dxf {

class SettingsTab : public AbstractFileSettings, Settings {
    QCheckBox* chbxBoldFont;
    QCheckBox* chbxItalicFont;
    QCheckBox* chbxOverrideFonts;
    QFontComboBox* fcbxDxfDefaultFont;

public:
    SettingsTab(QWidget* parent = nullptr);
    virtual ~SettingsTab() override;
    virtual void readSettings(MySettings& settings) override;
    virtual void writeSettings(MySettings& settings) override;
};

} // namespace Dxf

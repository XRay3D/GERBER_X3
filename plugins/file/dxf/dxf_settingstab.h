#pragma once

#include "dxf_types.h"

#include "file_plugin.h"
#include <QWidget>

class QCheckBox;
class QFontComboBox;

namespace Dxf {

class SettingsTab : public SettingsTabInterface, Settings {
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

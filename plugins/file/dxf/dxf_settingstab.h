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
    // По галочке на каждый Dxf::View: какие проекции строить для 3D-моделей.
    QCheckBox* chbxView[int(View::Count)];

public:
    SettingsTab(QWidget* parent = nullptr);
    virtual ~SettingsTab() override;
    virtual void readSettings(MySettings& settings) override;
    virtual void writeSettings(MySettings& settings) override;
};

} // namespace Dxf

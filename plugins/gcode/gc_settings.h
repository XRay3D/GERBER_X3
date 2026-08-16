/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "abstract_fileplugin.h"

#include <QMap>
#include <vector>

class QComboBox;
class QLabel;

namespace GCode {

class Plugin;

// Вкладка одного gcode-плагина: его скрипт генерации (по умолчанию
// scripts/<gcname>.js рядом с приложением) и, ниже, собственные настройки
// плагина, если он их даёт (Plugin::createSettingsTab).
class PluginTab final : public AbstractFileSettings {
    QString gcName_;
    QLineEdit* leScript_;
    AbstractFileSettings* inner_{};

public:
    PluginTab(Plugin* plugin, QWidget* parent);
    static QString defaultScriptPath(const QString& gcName);
    void readSettings(MySettings& settings) override;
    void writeSettings(MySettings& settings) override;
};

class Tab : public AbstractFileSettings {
    QCheckBox* chbxInfo;
    QCheckBox* chbxSameGFolder;
    QComboBox* cbxPost;
    QLabel* lblPostDescription;
    QTabWidget* tabWidget;
    std::vector<PluginTab*> pluginTabs_;

    void fillPosts();
    void showPostDescription();

public:
    Tab(QWidget* parent);
    virtual ~Tab() override;
    virtual void readSettings(MySettings& settings) override;
    virtual void writeSettings(MySettings& settings) override;
};

} // namespace GCode

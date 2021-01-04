/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "settings.h"
#include "ui_settingsdialog.h"

class SettingsDialog : public QDialog, private Ui::SettingsDialog, private AppSettings {
    Q_OBJECT

    int langIndex;

public:
    explicit SettingsDialog(QWidget* parent = nullptr, int tab = -1);
    ~SettingsDialog() override = default;
    void readSettings();
    void writeSettings();
    /////////////////////
    static void translator(QApplication* app, const QString& path);
    enum {
        Ui,
        Gerber,
        GCode,
        Utils
    };

public slots:
    void reject() override;
    void accept() override;
};

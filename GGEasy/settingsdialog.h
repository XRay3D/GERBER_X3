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
#include "mvector.h"
#include "settings.h"
#include "ui_settingsdialog.h"

class SettingsTabInterface;

class SettingsDialog : public QDialog, private Ui::SettingsDialog {
    Q_OBJECT

    int langIndex;
    MySettings settings;
    mvector<SettingsTabInterface*> tabs;

public:
    explicit SettingsDialog(QWidget* parent = nullptr, int tab = -1);
    ~SettingsDialog() override;
    void readSettings();
    void writeSettings();
    void readSettingsDialog();
    void writeSettingsDialog();
    /////////////////////
    static void translator(QApplication* app, const QString& path);
    enum {
        Ui,
        Utils,
        GCode,
        Gerber,
    };

public slots:
    void reject() override;
    void accept() override;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;
};

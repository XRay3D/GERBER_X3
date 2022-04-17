/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
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
    QPushButton* button;

public:
    explicit SettingsDialog(QWidget* parent = nullptr, int tab = -1);
    ~SettingsDialog() override;
    void readSettings();
    void saveSettings();
    void readSettingsDialog();
    void saveSettingsDialog();
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

    // QObject interface
    bool eventFilter(QObject* watched, QEvent* event) override;

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;
};

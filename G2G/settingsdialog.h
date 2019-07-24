#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "settings.h"
#include "ui_settingsdialog.h"

class SettingsDialog : public QDialog, private Ui::SettingsDialog, private Settings {
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget* parent = nullptr);
    ~SettingsDialog() override;
    void readSettings();
    void writeSettings();
    /////////////////////

public slots:
    void reject() override;
    void accept() override;
};

#endif // SETTINGSDIALOG_H

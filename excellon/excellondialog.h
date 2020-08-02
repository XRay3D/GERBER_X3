#pragma once
//#ifndef EXCELLONDIALOG_H
//#define EXCELLONDIALOG_H

#include "extypes.h"
#include <QDialog>

namespace Ui {
class ExcellonDialog;
}

namespace Excellon {
class File;
}

class ExcellonDialog : public QDialog {
    Q_OBJECT

public:
    explicit ExcellonDialog(Excellon::File* file);
    ~ExcellonDialog() override;

private slots:
    void on_pbStep_clicked();
    void on_pushButton_clicked();

private:
    Ui::ExcellonDialog* ui;
    Excellon::File* m_file;
    const Excellon::Format m_format;
    Excellon::Format m_tmpFormat;
    int m_step = 3;
    void updateFormat();
    void acceptFormat();
    void rejectFormat();

protected:
    void closeEvent(QCloseEvent* event) override;
};

//#endif // EXCELLONDIALOG_H

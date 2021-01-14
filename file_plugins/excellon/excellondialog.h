/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
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

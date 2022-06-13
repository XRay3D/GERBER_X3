/********************************************************************************
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
#include "ex_types.h"
#include <QDialog>

namespace Ui {
class ExcellonDialog;
}

namespace Excellon {
class File;

class FormatDialog : public QDialog {
    Q_OBJECT
    static inline bool m_showed;
    bool accepted;

public:
    explicit FormatDialog(Excellon::File* file);
    ~FormatDialog() override;

    static bool showed() { return m_showed; }

private slots:
    void on_pushButton_clicked();
    void on_pbSetAsDefault_clicked();

private:
    Ui::ExcellonDialog* ui;
    Excellon::File* m_file;
    const Excellon::Format m_format;
    Excellon::Format m_tmpFormat;

    void updateFormat();

    void acceptFormat();
    void rejectFormat();

    void resetFormat();

    // QWidget interface
protected:
    void closeEvent(QCloseEvent* event) override;
    void hideEvent(QHideEvent* event) override;
};

} // namespace Excellon

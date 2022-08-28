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
#include <QWidget>

namespace Ui {
class GCodePropertiesForm;
}

class GCodePropertiesForm : public QWidget {
    Q_OBJECT

public:
    explicit GCodePropertiesForm(QWidget* parent = nullptr);
    ~GCodePropertiesForm() override;

    void updatePosDsbxs();
    void updateAll();

private slots:
    void on_pbResetHome_clicked();
    void on_pbResetZero_clicked();

private:
    Ui::GCodePropertiesForm* ui;
};

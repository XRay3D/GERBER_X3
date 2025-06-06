/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
// #include "a_pch.h"
#include <QDialog>

namespace Ui {
class AboutForm;
}

class AboutForm : public QDialog {
    Q_OBJECT

public:
    explicit AboutForm(QWidget* parent = nullptr);
    ~AboutForm() override;

private:
    Ui::AboutForm* ui;
};

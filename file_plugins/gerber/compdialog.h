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
#include <QDialog>

namespace Ui {
class ComponentsDialog;
}

namespace Gerber {

class ComponentsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ComponentsDialog(QWidget* parent = nullptr);
    ~ComponentsDialog();
    void setFile(int fileId);

private:
    Ui::ComponentsDialog* ui;
};

}

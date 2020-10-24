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
#include <QDialog>

class ErrorItem;
class TableView;

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog {
    Q_OBJECT

public:
    explicit ErrorDialog(const QVector<ErrorItem*>& items, QWidget* parent = nullptr);
    ~ErrorDialog();

private:
    QWidget* lastWidget = nullptr;
    Ui::ErrorDialog* ui;
    TableView* table;
};

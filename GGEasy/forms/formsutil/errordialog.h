/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "mvector.h"

#include <QDialog>

class ErrorItem;
class TableView;

class QVBoxLayout;
class QDialogButtonBox;

class ErrorDialog : public QDialog {
    //    Q_OBJECT
    QDialogButtonBox* buttonBox;
    QVBoxLayout* verticalLayout;
    QWidget* lastWidget = nullptr;
    TableView* table;

    void setupUi(QDialog* ErrorDialog); // setupUi
    void retranslateUi(QDialog* ErrorDialog); // retranslateUi

public:
    explicit ErrorDialog(const mvector<ErrorItem*>& items, QWidget* parent = nullptr);
    ~ErrorDialog();
};

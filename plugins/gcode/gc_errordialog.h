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

#include "mvector.h"

#include <QDialog>

class GiError;
class TableView;

class QVBoxLayout;
class QDialogButtonBox;

class ErrorDialog : public QDialog {
    //    Q_OBJECT
    QDialogButtonBox* buttonBox;
    QVBoxLayout* verticalLayout;
    QWidget* lastWidget = nullptr;
    TableView* table;

    void setupUi(QDialog* ErrorDialog);       // setupUi
    void retranslateUi(QDialog* ErrorDialog); // retranslateUi
    int flikerTimerId = 0;

public:
    explicit ErrorDialog(mvector<GiError*>&& items, QWidget* parent = nullptr);
    ~ErrorDialog();

    // QObject interface
protected:
    void timerEvent(QTimerEvent* event) override;

    // QWidget interface
protected:
    void showEvent(QShowEvent *event) override;
};

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
//#include "a_pch.h"

#include <QDialog>

class QDialogButtonBox;
class QPushButton;
class QTreeWidget;
class QVBoxLayout;

class DialogAboutPlugins : public QDialog {
    Q_OBJECT

    QDialogButtonBox* buttonBox;
    QTreeWidget* treeWidget;
    QVBoxLayout* verticalLayout;
    QPushButton* pbClose;
    QPushButton* pbInfo;

    void setupUi(QDialog* Dialog); // setupUi

    void retranslateUi(QDialog* Dialog); // retranslateUi

public:
    DialogAboutPlugins(QWidget* parent = nullptr);
    virtual ~DialogAboutPlugins();
};

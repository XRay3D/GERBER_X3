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
#include "tool.h"
#include <QWidget>

class QGridLayout;
class QHBoxLayout;
class QLabel;
class QPushButton;

class ToolSelectorForm : public QWidget {
    Q_OBJECT

public:
    explicit ToolSelectorForm(QWidget* parent = nullptr);
    ~ToolSelectorForm();

    void setTool(const Tool& tool);
    const Tool& tool() const;
    QLabel* label() const;

signals:
    void updateName();

private slots:
    void on_pbSelect_clicked();
    void on_pbEdit_clicked();

private:
    Tool tool_;
    const int counter;
    const QString toolFileName_;

    void readTool();
    void updateForm();
    void writeTool() const;

    ///////////////////////////
    QLabel* label_;
    QLabel* lblPixmap;
    QLabel* lblName;
    QPushButton* pbSelect;
    QPushButton* pbEdit;

    void setupUi(QWidget* ToolSelectorForm); // setupUi
    void retranslateUi(QWidget* ToolSelectorForm); // retranslateUi
};

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
    Tool m_tool;
    const int counter;
    const QString m_toolFileName;

    void readTool();
    void updateForm();
    void writeTool() const;

    ///////////////////////////
    QLabel* m_label;
    QLabel* lblPixmap;
    QLabel* lblName;
    QPushButton* pbSelect;
    QPushButton* pbEdit;

    void setupUi(QWidget* ToolSelectorForm); // setupUi
    void retranslateUi(QWidget* ToolSelectorForm); // retranslateUi
};

/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "ui_tooleditdialog.h"
#include <QDialog>

class ToolEditDialog : public QDialog, private Ui::ToolEditDialog {
    Q_OBJECT

public:
    explicit ToolEditDialog(QWidget* parent = nullptr);
    ~ToolEditDialog() override = default;

    Tool tool() const;
    void setTool(const Tool& tool);
};

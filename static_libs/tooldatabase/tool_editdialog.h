/*******************************************************************************
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

#include "tool.h"

#include <QDialog>

namespace Ui {
class ToolEditDialog;
}

class ToolEditDialog : public QDialog {
    Q_OBJECT
    Ui::ToolEditDialog* ui;

public:
    explicit ToolEditDialog(QWidget* parent = nullptr);
    ~ToolEditDialog() override = default;

    Tool tool() const;
    void setTool(const Tool& tool);
};

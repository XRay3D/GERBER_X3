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

#include <QDialog>

namespace Ui {
class ToolEditDialog;
}

class ToolEditDialog : public QDialog {
    Q_OBJECT
    Ui::ToolEditDialog* ui;

public:
    explicit ToolEditDialog(QWidget* parent = nullptr);
    ~ToolEditDialog() override;

    Tool tool() const;
    void setTool(const Tool& tool);

    // QWidget interface
protected:
    void showEvent(QShowEvent* event) override;
};

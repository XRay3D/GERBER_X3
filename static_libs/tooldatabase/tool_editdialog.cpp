// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "tool_editdialog.h"

#include "ui_tooleditdialog.h"

ToolEditDialog::ToolEditDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ToolEditDialog) {
    ui->setupUi(this);
    ui->toolEdit->setDialog();
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this] {
        if (ui->toolEdit->tool_.isValid()) {
            ui->toolEdit->tool_.setId(-1);
            accept();
        } else {
            ui->toolEdit->on_pbApply_clicked();
            // toolEdit->tool_.errorMessageBox(this);
        }
    });
}

Tool ToolEditDialog::tool() const { return ui->toolEdit->tool_; }

void ToolEditDialog::setTool(const Tool& tool) { ui->toolEdit->setTool(tool); }

#include "moc_tool_editdialog.cpp"

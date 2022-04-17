// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "tool_editdialog.h"

#include "ui_tooleditdialog.h"

ToolEditDialog::ToolEditDialog(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::ToolEditDialog) {
    ui->setupUi(this);
    ui->toolEdit->setDialog();
    connect(ui->buttonBox, &QDialogButtonBox::accepted, [this] {
        if (ui->toolEdit->m_tool.isValid()) {
            ui->toolEdit->m_tool.setId(-1);
            accept();
        } else {
            ui->toolEdit->on_pbApply_clicked();
            // toolEdit->m_tool.errorMessageBox(this);
        }
    });
}

Tool ToolEditDialog::tool() const { return ui->toolEdit->m_tool; }

void ToolEditDialog::setTool(const Tool& tool) { ui->toolEdit->setTool(tool); }

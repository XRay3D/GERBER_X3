// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "tooleditdialog.h"

ToolEditDialog::ToolEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    toolEdit->setDialog();
    connect(buttonBox, &QDialogButtonBox::accepted, [this] {
        if (toolEdit->m_tool.isValid()) {
            toolEdit->m_tool.setId(-1);
            accept();
        } else {
            toolEdit->on_pbApply_clicked();
            //toolEdit->m_tool.errorMessageBox(this);
        }
    });
}

Tool ToolEditDialog::tool() const { return toolEdit->m_tool; }

void ToolEditDialog::setTool(const Tool& tool) { toolEdit->setTool(tool); }

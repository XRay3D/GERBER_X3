// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "tooleditdialog.h"

#include <QMessageBox>

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

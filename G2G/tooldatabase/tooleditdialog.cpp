#include "tooleditdialog.h"

#include <QMessageBox>

ToolEditDialog::ToolEditDialog(QWidget* parent)
    : QDialog(parent)
{
    setupUi(this);
    toolEdit->setDialog();
    connect(buttonBox, &QDialogButtonBox::accepted, [=] {
        if (toolEdit->m_tool.isValid())
            accept();
        else
            toolEdit->on_pbApply_clicked(); //            toolEdit->m_tool.errorMessageBox(this);
    });
}

ToolEditDialog::~ToolEditDialog() {}

Tool ToolEditDialog::tool() const { return toolEdit->m_tool; }

void ToolEditDialog::setTool(const Tool& tool) { toolEdit->setTool(tool); }

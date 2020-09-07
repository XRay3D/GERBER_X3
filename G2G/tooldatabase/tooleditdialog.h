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

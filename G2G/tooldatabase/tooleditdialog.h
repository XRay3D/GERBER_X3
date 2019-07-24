#ifndef TOOLEDITDIALOG_H
#define TOOLEDITDIALOG_H

#include "ui_tooleditdialog.h"
#include <QDialog>

class ToolEditDialog : public QDialog, private Ui::ToolEditDialog {
    Q_OBJECT

public:
    explicit ToolEditDialog(QWidget* parent = nullptr);
    ~ToolEditDialog();

    Tool tool() const;
    void setTool(const Tool& tool);
};

#endif // TOOLEDITDIALOG_H

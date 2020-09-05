#pragma once

#include <QDialog>

class ErrorItem;
class TableView;

namespace Ui {
class ErrorDialog;
}

class ErrorDialog : public QDialog {
    Q_OBJECT

public:
    explicit ErrorDialog(const QVector<ErrorItem*>& items, QWidget* parent = nullptr);
    ~ErrorDialog();

private:
    Ui::ErrorDialog* ui;
    TableView* table;
};

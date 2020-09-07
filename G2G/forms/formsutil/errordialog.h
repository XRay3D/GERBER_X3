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
    QWidget* lastWidget = nullptr;
    Ui::ErrorDialog* ui;
    TableView* table;
};

#pragma once

#include <QDialog>

namespace Ui {
class ComponentsDialog;
}

namespace Gerber {

class ComponentsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ComponentsDialog(QWidget* parent = nullptr);
    ~ComponentsDialog();
    void setFile(int fileId);

private:
    Ui::ComponentsDialog* ui;
};

}

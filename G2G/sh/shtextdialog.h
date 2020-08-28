#ifndef SHTEXTDIALOG_H
#define SHTEXTDIALOG_H

#include <QDialog>

namespace Ui {
class ShTextDialog;
}
namespace Shapes {
class Text;
}
class ShTextDialog : public QDialog {
    Q_OBJECT
    friend Shapes::Text;

public:
    explicit ShTextDialog(Shapes::Text* text, QWidget* parent = nullptr);
    ~ShTextDialog();

private:
    Ui::ShTextDialog* ui;
    Shapes::Text* pText;

    void updateText();

    QString text_;
    QString font_;
    double angle_ = 0;
    double height_ = 10;
    int centerAlign_ = 0;

};

#endif // SHTEXTDIALOG_H

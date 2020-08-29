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
    explicit ShTextDialog(QVector<Shapes::Text*> text, QWidget* parent = nullptr);
    ~ShTextDialog();

private:
    Ui::ShTextDialog* ui;

    QVector<Shapes::Text*> shapeText;

    void updateText();
    void updateFont();
    void updateAngle();
    void updateHeight();
    void updateCenterAlign();
    void updateSide();

    // QDialog interface
public slots:
    void accept() override;
    void reject() override;
};

#endif // SHTEXTDIALOG_H

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include <QDialog>

namespace Ui {
class ShTextDialog;
}

namespace ShTxt {

class Shape;

class ShTextDialog : public QDialog {
    Q_OBJECT
    friend Shape;

public:
    explicit ShTextDialog(QVector<Shape*> text, QWidget* parent = nullptr);
    ~ShTextDialog();

private:
    Ui::ShTextDialog* ui;

    QVector<Shape*> shapeText;

    void updateText();
    void updateFont();
    void updateAngle();
    void updateHeight();
    void updateXY();
    void updateCenterAlign();
    void updateSide();

    // QDialog interface
public slots:
    void accept() override;
    void reject() override;
};

} // namespace ShTxt

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
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


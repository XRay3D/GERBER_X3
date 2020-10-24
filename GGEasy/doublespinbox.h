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
#include <QDoubleSpinBox>

class DoubleSpinBox : public QDoubleSpinBox {
    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget* parent = nullptr);
    void setRange(double min, double max);
    void setMaximum(double max);
    void setMinimum(double min);
    void flicker();

    // QObject interface
    bool eventFilter(QObject* watched, QEvent* event) override;

private:
    void red();
    void normal();

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent* event) override;
};

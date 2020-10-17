/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QWidget>

namespace Ui {
class ColorSelector;
}

class ColorSelector : public QWidget {
    Q_OBJECT

public:
    explicit ColorSelector(QColor& color, const QColor& defaultColor, QWidget* parent = nullptr);
    ~ColorSelector() override;

private slots:
    void on_pbResetColor_clicked();

private:
    Ui::ColorSelector* ui;
    QColor& m_color;
    const QColor m_defaultColor;

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;
    void setColor(QColor* color);
};

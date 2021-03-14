/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include <QWidget>

class QPushButton;
class PushButton;
class QLineEdit;

class ColorSelector : public QWidget {
    Q_OBJECT

    QPushButton* pbResetColor;
    PushButton* pbSelectColor;
    QLineEdit* lineEdit;

public:
    explicit ColorSelector(QColor& color, const QColor& defaultColor, QWidget* parent = nullptr);
    ~ColorSelector() override;

private:
    void resetColor();
    void updateName();

    QColor& m_color;
    const QColor m_defaultColor;
};

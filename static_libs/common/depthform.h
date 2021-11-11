/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
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

class DoubleSpinBox;
class QLabel;
class QRadioButton;

class DepthForm : public QWidget {
    Q_OBJECT
public:
    explicit DepthForm(QWidget* parent = nullptr);
    ~DepthForm() override;
    double value() const;
    void setValue(double value);

signals:
    void valueChanged(double);

private:
    double m_value = 0.0;

    DoubleSpinBox* dsbx;
    QLabel* label;
    QRadioButton* rbBoard;
    QRadioButton* rbCopper;
    QRadioButton* rbCustom;

    const QString m_parentName;

    void setupUi(QWidget* Form);
    void retranslateUi(QWidget* Form);
};

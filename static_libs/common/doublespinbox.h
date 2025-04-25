/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once
#include <QDoubleSpinBox>
#include <optional>

class QJSEngine;

class DoubleSpinBox : public QDoubleSpinBox {
    //    Q_OBJECT
    void red();
    void normal();
    // mutable QString str;
    mutable std::optional<double> value_;
    friend struct Validator;
    QJSEngine* const jsEngine;
    void updateToolTip();

public:
    explicit DoubleSpinBox(QWidget* parent = nullptr);
    void setRange(double min, double max);
    void setMaximum(double max);
    void setMinimum(double min);
    void flicker();

    // QObject interface
    bool eventFilter(QObject* watched, QEvent* event) override;

public:
    // QAbstractSpinBox interface
    double valueFromText(const QString& text) const override;
    // QString textFromValue(double value) const override;
    QValidator::State validate(QString& input, int& pos) const override;

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent* event) override;
    void keyReleaseEvent(QKeyEvent* event) override;
};

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

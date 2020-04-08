#pragma once
#ifndef DOUBLESPINBOX_H
#define DOUBLESPINBOX_H

#include <QDoubleSpinBox>
#include <QStack>

class DoubleSpinBox : public QDoubleSpinBox {
    //    Q_OBJECT
public:
    explicit DoubleSpinBox(QWidget* parent = nullptr);
    void setRange(double min, double max);
    void setMaximum(double max);
    void setMinimum(double min);
    void flicker();

private:
    void red();
    void normal();

    // QWidget interface
protected:
    void keyPressEvent(QKeyEvent* event) override;

    // QObject interface
public:
    bool eventFilter(QObject* watched, QEvent* event) override;
};

#endif // DOUBLESPINBOX_H

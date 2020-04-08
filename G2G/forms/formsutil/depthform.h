#pragma once
#ifndef DEPTHFORM_H
#define DEPTHFORM_H

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

#endif // DEPTHFORM_H

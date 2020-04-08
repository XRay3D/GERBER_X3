#pragma once
#ifndef DEPTHFORM_H
#define DEPTHFORM_H

#include <QWidget>
#include <doublespinbox.h>

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDoubleSpinBox>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QRadioButton>
#include <QtWidgets/QWidget>

#include "gcodepropertiesform.h"

class DepthForm : public QWidget {
    Q_OBJECT
public:
    explicit DepthForm(QWidget* parent = nullptr);
    ~DepthForm() override;
    double value() const;
    QRadioButton* rbBoard;
    QRadioButton* rbCopper;
    QRadioButton* rbCustom;

signals:
    void valueChanged(double);

private:
    double m_value = 0.0;
    DoubleSpinBox* dsbx;
    const QString m_parent;
    void setupUi(QWidget* Form);
    void retranslateUi(QWidget* Form);
};

#endif // DEPTHFORM_H

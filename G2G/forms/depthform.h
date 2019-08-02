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
    ~DepthForm() override = default;

    double value(bool fl = false) const;
    void setValue(double value);
    QRadioButton* rbCopper;
    QRadioButton* rbBoard;
    QRadioButton* rbCustom;
signals:
    void valueChanged(double);

private:
    double m_value = 0.0;
    bool m_fl = false;
    QHBoxLayout* horizontalLayout;
    DoubleSpinBox* dsbx;
    void setupUi(QWidget* Form); // setupUi
    void retranslateUi(QWidget* Form); // retranslateUi
};

//class DepthForm : public QWidget
//{
//    Q_OBJECT
//public:
//    explicit DepthForm(QWidget *parent = nullptr);

//signals:

//public slots:
//};

#endif // DEPTHFORM_H

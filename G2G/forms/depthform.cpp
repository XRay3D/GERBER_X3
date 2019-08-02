#include "depthform.h"

//DepthForm::DepthForm(QWidget *parent) : QWidget(parent)
//{

//}

DepthForm::DepthForm(QWidget* parent)
    : QWidget(parent)
{
    setupUi(this);
    retranslateUi(this);
    connect(dsbx, QOverload<double>::of(&QDoubleSpinBox::valueChanged), this, &DepthForm::valueChanged);
    //    connect(dsbx, QOverload<double>::of(&QDoubleSpinBox::valueChanged), [this](double value) {
    //        if (rbCopper->isChecked())
    //            GCodePropertiesForm::copperThickness = value;
    //        if (rbBoard->isChecked())
    //            GCodePropertiesForm::thickness = value;
    //    });

    connect(rbCustom, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            dsbx->setEnabled(true);
            dsbx->setValue(m_value);
            m_fl = true;
        }
    });
    connect(rbCopper, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            if (m_fl)
                m_value = dsbx->value();
            m_fl = false;
            dsbx->setValue(GCodePropertiesForm::copperThickness);
            dsbx->setEnabled(false);
        }
    });
    connect(rbBoard, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            if (m_fl)
                m_value = dsbx->value();
            m_fl = false;
            dsbx->setValue(GCodePropertiesForm::thickness);
            dsbx->setEnabled(false);
        }
    });
    rbCustom->setChecked(true);
}

double DepthForm::value(bool fl) const { return fl ? m_value : dsbx->value(); }

void DepthForm::setValue(double value) { dsbx->setValue(value); }

void DepthForm::setupUi(QWidget* Form)
{
    if (Form->objectName().isEmpty())
        Form->setObjectName(QString::fromUtf8("DepthForm"));

    QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    sizePolicy.setHorizontalStretch(0);
    sizePolicy.setVerticalStretch(0);

    horizontalLayout = new QHBoxLayout(Form);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setSpacing(2);

    rbCopper = new QRadioButton(Form);
    rbCopper->setObjectName(QString::fromUtf8("rbCopper"));
    sizePolicy.setHeightForWidth(rbCopper->sizePolicy().hasHeightForWidth());
    rbCopper->setSizePolicy(sizePolicy);
    horizontalLayout->addWidget(rbCopper);

    rbBoard = new QRadioButton(Form);
    rbBoard->setObjectName(QString::fromUtf8("rbBoard"));
    sizePolicy.setHeightForWidth(rbBoard->sizePolicy().hasHeightForWidth());
    rbBoard->setSizePolicy(sizePolicy);
    horizontalLayout->addWidget(rbBoard);

    rbCustom = new QRadioButton(Form);
    rbCustom->setObjectName(QString::fromUtf8("rbCustom"));
    sizePolicy.setHeightForWidth(rbCustom->sizePolicy().hasHeightForWidth());
    rbCustom->setSizePolicy(sizePolicy);
    horizontalLayout->addWidget(rbCustom);

    dsbx = new DoubleSpinBox(Form);
    dsbx->setObjectName(QString::fromUtf8("dsbx"));
    dsbx->setDecimals(3);
    dsbx->setMaximum(10.0);
    dsbx->setSingleStep(0.005);
    dsbx->setMinimumWidth(QFontMetrics(font()).boundingRect("10.000 mmm").width());

    horizontalLayout->addWidget(dsbx);
    retranslateUi(Form);
    QMetaObject::connectSlotsByName(Form);
}

void DepthForm::retranslateUi(QWidget* Form)
{
    Form->setWindowTitle(QApplication::translate("DepthForm", "Form", nullptr));
    rbCopper->setText(QApplication::translate("DepthForm", "C", nullptr));
    rbCopper->setToolTip(QApplication::translate("DepthForm", "Copper", nullptr));
    rbBoard->setText(QApplication::translate("DepthForm", "B", nullptr));
    rbBoard->setToolTip(QApplication::translate("DepthForm", "Board", nullptr));
    rbCustom->setText(QApplication::translate("DepthForm", "U", nullptr));
    rbCustom->setToolTip(QApplication::translate("DepthForm", "User", nullptr));
    dsbx->setSuffix(QApplication::translate("DepthForm", " mm", nullptr));
    dsbx->setToolTip(QApplication::translate("DepthForm", "Cutting depth", nullptr));
}

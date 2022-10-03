// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "depthform.h"
#include "app.h"
#include "doublespinbox.h"
#include "project.h"
#include "settings.h"
#include <QtWidgets>

DepthForm::DepthForm(QWidget* parent)
    : QWidget(parent)
    , parentName_(parent->objectName()) {
    setupUi(this);
    retranslateUi(this);

    connect(dsbx, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &DepthForm::valueChanged);
    connect(dsbx, qOverload<double>(&QDoubleSpinBox::valueChanged), [this](double value) {
        if (dsbx->isEnabled())
            value_ = value;
    });
    connect(rbCustom, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            dsbx->setEnabled(true);
            dsbx->setValue(value_);
        }
    });
    connect(rbCopper, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            dsbx->setEnabled(false);
            dsbx->setValue(App::project()->copperThickness());
        }
    });
    connect(rbBoard, &QRadioButton::toggled, [this](bool checked) {
        if (checked) {
            dsbx->setEnabled(false);
            dsbx->setValue(App::project()->boardThickness());
        }
    });

    MySettings settings;
    settings.beginGroup(parentName_);
    settings.getValue("dsbxDepth", value_);
    settings.getValue(rbBoard);
    settings.getValue(rbCopper);
    settings.getValue(rbCustom, true);
    settings.endGroup();
}

DepthForm::~DepthForm() {
    MySettings settings;
    settings.beginGroup(parentName_);
    settings.setValue("dsbxDepth", value_);
    settings.setValue(rbBoard);
    settings.setValue(rbCopper);
    settings.setValue(rbCustom);
    settings.endGroup();
}

double DepthForm::value() const { return dsbx->value(); }

void DepthForm::setValue(double value) {
    rbCustom->setChecked(true);
    dsbx->setValue(value);
}

void DepthForm::setupUi(QWidget* Form) {
    if (Form->objectName().isEmpty())
        Form->setObjectName(QString::fromUtf8("DepthForm"));

    QHBoxLayout* horizontalLayout = new QHBoxLayout(Form);
    horizontalLayout->setObjectName(QString::fromUtf8("horizontalLayout"));
    horizontalLayout->setContentsMargins(0, 0, 0, 0);
    horizontalLayout->setSpacing(2);

    {
        label = new QLabel(Form);
        label->setObjectName(QString::fromUtf8("label"));

        QFontMetrics fm(font());
        label->setMinimumWidth(std::max(
            fm.horizontalAdvance(QCoreApplication::translate("ToolSelectorForm", "Tool:", nullptr)),
            fm.horizontalAdvance(QCoreApplication::translate("DepthForm", "Depth:", nullptr))));
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        horizontalLayout->addWidget(label);
    }

    horizontalLayout->addSpacing(6);

    {
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);

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
    }

    {
        dsbx = new DoubleSpinBox(Form);
        dsbx->setObjectName(QString::fromUtf8("dsbx"));
        dsbx->setDecimals(3);
        dsbx->setMaximum(100.0);
        dsbx->setSingleStep(0.005);
        dsbx->setMinimumWidth(QFontMetrics(font()).boundingRect("10.000 mmm").width());
        horizontalLayout->addWidget(dsbx);
        horizontalLayout->setStretchFactor(dsbx, 1);
    }

    retranslateUi(Form);
    QMetaObject::connectSlotsByName(Form);
}

void DepthForm::retranslateUi(QWidget* Form) {
    Form->setWindowTitle(QApplication::translate("DepthForm", "Form", nullptr));
    Form->setToolTip(QApplication::translate("DepthForm", "Cutting depth", nullptr));

    dsbx->setSuffix(QApplication::translate("DepthForm", " mm", nullptr));
    dsbx->setToolTip(QApplication::translate("DepthForm", "Cutting depth", nullptr));

    label->setText(QCoreApplication::translate("DepthForm", "Depth:", nullptr));

    rbBoard->setText(QApplication::translate("DepthForm", "B", nullptr));
    rbBoard->setToolTip(QApplication::translate("DepthForm", "Board", nullptr));

    rbCopper->setText(QApplication::translate("DepthForm", "C", nullptr));
    rbCopper->setToolTip(QApplication::translate("DepthForm", "Copper", nullptr));

    rbCustom->setText(QApplication::translate("DepthForm", "U", nullptr));
    rbCustom->setToolTip(QApplication::translate("DepthForm", "User", nullptr));
}

#include "moc_depthform.cpp"

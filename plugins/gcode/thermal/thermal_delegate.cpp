// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thermal_delegate.h"
#include "doublespinbox.h"
#include "thermal_model.h"

#include <QSpinBox>

namespace Thermal {

Delegate::Delegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    switch (index.column()) {
    case Model::Name:
    case Model::Position:
        break;
    case Model::GapAngle: {
        auto* dsbx = new DoubleSpinBox(parent);
        dsbx->setRange(0, 360);
        dsbx->setSingleStep(15);
        dsbx->setDecimals(2);
        dsbx->setAlignment(Qt::AlignCenter);
        connect(dsbx, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Delegate::emitCommitData);
        return dsbx;
    }
    case Model::apThickness: {
        auto* dsbx = new DoubleSpinBox(parent);
        dsbx->setRange(0, 10);
        dsbx->setSingleStep(0.05);
        dsbx->setDecimals(2);
        dsbx->setAlignment(Qt::AlignCenter);
        connect(dsbx, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &Delegate::emitCommitData);
        return dsbx;
    }
    case Model::GapCount: {
        auto* sbx = new QSpinBox(parent);
        sbx->setRange(0, 16);
        sbx->setSingleStep(1);
        sbx->setAlignment(Qt::AlignCenter);
        connect(sbx, qOverload<int>(&QSpinBox::valueChanged), this, &Delegate::emitCommitData);
        return sbx;
    }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    switch (index.column()) {
    case Model::Name:
    case Model::Position:
        return;
    case Model::GapAngle:
    case Model::apThickness: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        dsbx->setValue(index.data(Qt::EditRole).toDouble());
        return;
    }
    case Model::GapCount: {
        auto* sbx = qobject_cast<QSpinBox*>(editor);
        if (!sbx)
            return;
        sbx->setValue(index.data(Qt::EditRole).toInt());
        return;
    }
    }
}

void Delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    switch (index.column()) {
    case Model::Name:
    case Model::Position:
        return;
    case Model::GapAngle:
    case Model::apThickness: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        model->setData(index, dsbx->value());
        return;
    }
    case Model::GapCount: {
        auto* sbx = qobject_cast<QSpinBox*>(editor);
        if (!sbx)
            return;
        model->setData(index, sbx->value());
        return;
    }
    }
}

void Delegate::emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

} // namespace Thermal

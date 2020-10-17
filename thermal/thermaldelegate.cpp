// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "thermaldelegate.h"
#include "doublespinbox.h"
#include "thermalmodel.h"
#include <QSpinBox>

#include "leakdetector.h"

ThermalDelegate::ThermalDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* ThermalDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    switch (index.column()) {
    case ThermalModel::Name:
    case ThermalModel::Position:
        break;
    case ThermalModel::GapAngle: {
        auto* dsbx = new DoubleSpinBox(parent);
        dsbx->setRange(0, 360);
        dsbx->setSingleStep(15);
        dsbx->setDecimals(2);
        dsbx->setAlignment(Qt::AlignCenter);
        connect(dsbx, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ThermalDelegate::emitCommitData);
        return dsbx;
    }
    case ThermalModel::apThickness: {
        auto* dsbx = new DoubleSpinBox(parent);
        dsbx->setRange(0, 10);
        dsbx->setSingleStep(0.05);
        dsbx->setDecimals(2);
        dsbx->setAlignment(Qt::AlignCenter);
        connect(dsbx, qOverload<double>(&QDoubleSpinBox::valueChanged), this, &ThermalDelegate::emitCommitData);
        return dsbx;
    }
    case ThermalModel::GapCount: {
        auto* sbx = new QSpinBox(parent);
        sbx->setRange(0, 16);
        sbx->setSingleStep(1);
        sbx->setAlignment(Qt::AlignCenter);
        connect(sbx, qOverload<int>(&QSpinBox::valueChanged), this, &ThermalDelegate::emitCommitData);
        return sbx;
    }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ThermalDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    switch (index.column()) {
    case ThermalModel::Name:
    case ThermalModel::Position:
        return;
    case ThermalModel::GapAngle:
    case ThermalModel::apThickness: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        dsbx->setValue(index.data(Qt::EditRole).toDouble());
        return;
    }
    case ThermalModel::GapCount: {
        auto* sbx = qobject_cast<QSpinBox*>(editor);
        if (!sbx)
            return;
        sbx->setValue(index.data(Qt::EditRole).toInt());
        return;
    }
    }
}

void ThermalDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    switch (index.column()) {
    case ThermalModel::Name:
    case ThermalModel::Position:
        return;
    case ThermalModel::GapAngle:
    case ThermalModel::apThickness: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        model->setData(index, dsbx->value());
        return;
    }
    case ThermalModel::GapCount: {
        auto* sbx = qobject_cast<QSpinBox*>(editor);
        if (!sbx)
            return;
        model->setData(index, sbx->value());
        return;
    }
    }
}

void ThermalDelegate::emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

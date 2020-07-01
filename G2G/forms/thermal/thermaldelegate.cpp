// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "thermaldelegate.h"
#include "thermalmodel.h"

#include <QDoubleSpinBox>
#include <QSpinBox>

ThermalDelegate::ThermalDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* ThermalDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    switch (index.column()) {
    case ThermalModel::ThName:
    case ThermalModel::ThPos:
        break;
    case ThermalModel::ThGapAngle: {
        auto* dsbx = new QDoubleSpinBox(parent);
        dsbx->setRange(0, 360);
        dsbx->setSingleStep(15);
        dsbx->setDecimals(2);
        return dsbx;
    }
    case ThermalModel::ThGapThickness: {
        auto* dsbx = new QDoubleSpinBox(parent);
        dsbx->setRange(0, 10);
        dsbx->setSingleStep(0.05);
        dsbx->setDecimals(2);
        return dsbx;
    }
    case ThermalModel::ThGapCount: {
        auto* sbx = new QSpinBox(parent);
        sbx->setRange(0, 16);
        sbx->setSingleStep(1);
        return sbx;
    }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void ThermalDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    switch (index.column()) {
    case ThermalModel::ThName:
    case ThermalModel::ThPos:
        break;
    case ThermalModel::ThGapAngle: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        dsbx->setValue(index.data(Qt::EditRole).toDouble());
        return;
    }
    case ThermalModel::ThGapThickness: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        dsbx->setValue(index.data(Qt::EditRole).toDouble());

        return;
    }
    case ThermalModel::ThGapCount: {
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
    case ThermalModel::ThName:
    case ThermalModel::ThPos:
        break;
    case ThermalModel::ThGapAngle: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        model->setData(index, dsbx->value());
        return;
    }
    case ThermalModel::ThGapThickness: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if (!dsbx)
            return;
        model->setData(index, dsbx->value());
        return;
    }
    case ThermalModel::ThGapCount: {
        auto* sbx = qobject_cast<QSpinBox*>(editor);
        if (!sbx)
            return;
        model->setData(index, sbx->value());
        return;
    }
    }
}

void ThermalDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}

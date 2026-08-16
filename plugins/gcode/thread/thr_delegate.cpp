/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  ХХ ХХХ 2026                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "thr_delegate.h"
#include "doublespinbox.h"
#include "thr_model.h"

namespace Threading {

Delegate::Delegate(QObject* parent)
    : QStyledItemDelegate{parent} {
}

QWidget* Delegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const {
    switch(index.column()) {
    case Model::Correction: {
        auto* dsbx = new DoubleSpinBox{parent};
        dsbx->setRange(-1.0, 1.0);
        dsbx->setSingleStep(0.01);
        dsbx->setDecimals(3);
        dsbx->setSuffix(u" mm"_s);
        dsbx->setAlignment(Qt::AlignCenter);
        connect(dsbx, &QDoubleSpinBox::valueChanged, this, &Delegate::emitCommitData);
        return dsbx;
    }
    case Model::HoleDiameter: {
        auto* dsbx = new DoubleSpinBox{parent};
        dsbx->setRange(0.0, 100.0);
        dsbx->setSingleStep(0.01);
        dsbx->setDecimals(3);
        dsbx->setSuffix(u" mm"_s);
        dsbx->setAlignment(Qt::AlignCenter);
        connect(dsbx, &QDoubleSpinBox::valueChanged, this, &Delegate::emitCommitData);
        return dsbx;
    }
    }
    return QStyledItemDelegate::createEditor(parent, option, index);
}

void Delegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    switch(index.column()) {
    case Model::Correction:
    case Model::HoleDiameter: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if(!dsbx)
            return;
        dsbx->setValue(index.data(Qt::EditRole).toDouble());
        return;
    }
    }
}

void Delegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    switch(index.column()) {
    case Model::Correction:
    case Model::HoleDiameter: {
        auto* dsbx = qobject_cast<QDoubleSpinBox*>(editor);
        if(!dsbx)
            return;
        model->setData(index, dsbx->value());
        return;
    }
    }
}

void Delegate::emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

} // namespace Threading

#include "moc_thr_delegate.cpp"

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "ft_sidedelegate.h"

#include <QComboBox>

namespace FileTree {

SideDelegate::SideDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

QWidget* SideDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const {
    auto* comboBox = new QComboBox(parent);
    comboBox->addItems(tr("Top|Bottom").split('|'));
    comboBox->setItemData(0, comboBox->size(), Qt::SizeHintRole);
    comboBox->setItemData(1, comboBox->size(), Qt::SizeHintRole);
    connect(comboBox, qOverload<int>(&QComboBox::activated), this, &SideDelegate::emitCommitData);
    return comboBox;
}

void SideDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
    comboBox->showPopup();
}

void SideDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox)
        return;
    model->setData(index, bool(comboBox->currentIndex()));
}

void SideDelegate::emitCommitData() {
    emit commitData(qobject_cast<QWidget*>(sender()));
}

} // namespace FileTree

#include "moc_ft_sidedelegate.cpp"

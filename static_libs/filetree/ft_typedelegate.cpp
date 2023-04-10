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
#include "ft_typedelegate.h"

#include "abstract_file.h"
#include "project.h"

#include <QComboBox>

namespace FileTree {

TypeDelegate::TypeDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

QWidget* TypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const {
    auto comboBox = new QComboBox(parent);
    connect(comboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &TypeDelegate::emitCommitData);
    return comboBox;
}

void TypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto comboBox = qobject_cast<QComboBox*>(editor);
    comboBox->clear();
    int ctr = 0;
    for (auto& [id, name, toolTip] : App::project().file(index.data(Qt::UserRole).toInt())->displayedTypes()) {
        if (id < 0)
            continue;
        comboBox->addItem(name, id);
        comboBox->setItemData(ctr, comboBox->size(), Qt::SizeHintRole);
        comboBox->setItemData(ctr, name + '\n' + toolTip, Qt::ToolTipRole);
        ++ctr;
    }
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
    comboBox->showPopup();
}

void TypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto comboBox = qobject_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentData());
}

void TypeDelegate::emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

} // namespace FileTree

#include "moc_ft_typedelegate.cpp"

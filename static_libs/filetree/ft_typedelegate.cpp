// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "ft_typedelegate.h"

#include "interfaces/file.h"
#include "project.h"

#include <QComboBox>

#include "leakdetector.h"

namespace FileTree {

TypeDelegate::TypeDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* TypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    qDebug() << __FUNCTION__;
    auto comboBox = new QComboBox(parent);
    connect(comboBox, qOverload<int>(&QComboBox::activated), this, &TypeDelegate::emitCommitData);
    return comboBox;
}

void TypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    qDebug() << __FUNCTION__ << App::project()->file(index.data(Qt::UserRole).toInt())->name();
    auto comboBox = qobject_cast<QComboBox*>(editor);
    comboBox->clear();
    int ctr = 0;
    for (auto& [id, name, toolTip] : App::project()->file(index.data(Qt::UserRole).toInt())->displayedTypes()) {
        if (id < 0)
            continue;
        comboBox->addItem(name, id);
        comboBox->setItemData(ctr, comboBox->size(), Qt::SizeHintRole);
        comboBox->setItemData(ctr, name + '\n' + toolTip, Qt::ToolTipRole);
        qDebug() << __FUNCTION__ << ctr << name << id;
        ++ctr;
    }
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
    comboBox->showPopup();
}

void TypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto comboBox = qobject_cast<QComboBox*>(editor);
    model->setData(index, comboBox->currentData());
}

void TypeDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}

}

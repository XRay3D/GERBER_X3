// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "layerdelegate.h"
#include "gbrnode.h"

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QRadioButton>


////////////////////////////////////////////////////////////
/// \brief TextDelegate::TextDelegate
/// \param parent
///
TextDelegate::TextDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* TextDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* le = new QLineEdit(parent);
    //    le->setText(index.data(Qt::EditRole).toString());
    connect(le, &QLineEdit::textChanged, this, &TextDelegate::emitCommitData);
    return le;
}

void TextDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* le = qobject_cast<QLineEdit*>(editor);
    if (!le)
        return;
    le->setText(index.data(Qt::EditRole).toString());
}

void TextDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* le = qobject_cast<QLineEdit*>(editor);
    if (!le)
        return;
    model->setData(index, le->text());
}

void TextDelegate::emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

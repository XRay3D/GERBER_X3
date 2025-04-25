/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "ft_textdelegate.h"

#include <QLineEdit>

namespace FileTree {

TextDelegate::TextDelegate(QObject* parent)
    : QStyledItemDelegate{parent} {
}

QWidget* TextDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& /*index*/) const {
    auto* le = new QLineEdit{parent};
    rect_ = option.rect;
    connect(le, &QLineEdit::textChanged, this, &TextDelegate::emitCommitData);
    return le;
}

void TextDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* le = qobject_cast<QLineEdit*>(editor);
    le->setGeometry(rect_);
    le->setText(index.data(Qt::EditRole).toString());
}

void TextDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto* le = qobject_cast<QLineEdit*>(editor);
    model->setData(index, le->text());
}

void TextDelegate::emitCommitData() {
    emit commitData(qobject_cast<QWidget*>(sender()));
}

} // namespace FileTree

#include "moc_ft_textdelegate.cpp"

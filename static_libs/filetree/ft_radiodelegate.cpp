// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#ifdef GBR_
#include "radiodelegate.h"
#include "gbrnode.h"
#include <QApplication>
#include <QRadioButton>

#include "leakdetector.h"

////////////////////////////////////////////////////////////
/// \brief RadioDelegate::RadioDelegate
/// \param parent
///
RadioDelegate::RadioDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* RadioDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    auto* radioButton = new QRadioButton(parent);
    //    radioButton->addItems({ tr("Top"), tr("Bottom") });
    //        if (index.column() == 1)
    //            comboBox->addItems(IconPreviewArea::iconModeNames());
    //        else if (index.column() == 2)
    //            comboBox->addItems(IconPreviewArea::iconStateNames());
    //connect(comboBox, qOverload<int>(&QComboBox::activated), this, &LayerDelegate::emitCommitData);
    connect(radioButton, &QRadioButton::clicked, this, &RadioDelegate::commitAndCloseEditor);
    return radioButton;
}

void RadioDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    auto* radioButton = qobject_cast<QRadioButton*>(editor);
    if (!radioButton)
        return;
    //        int pos = comboBox->findText(index.model()->data(index).toString(),
    //            Qt::MatchExactly);
    radioButton->setChecked(index.model()->data(index).toInt());
}

void RadioDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    auto* radioButton = qobject_cast<QRadioButton*>(editor);
    if (!radioButton)
        return;
    model->setData(index, bool(radioButton->isChecked()));
}

void RadioDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    if (dynamic_cast<Gerber::Node*>(reinterpret_cast<FileTree::Node*>(index.internalPointer()))) {

        //        StarRating starRating = qvariant_cast<StarRating>(index.data());

        //        if (option.state & QStyle::State_Selected)
        //            painter->fillRect(option.rect, option.palette.highlight());

        //        starRating.paint(painter, option.rect, option.palette, StarRating::ReadOnly);
        //QStyleOptionButton progressBarOption;
        //progressBarOption.rect = option.rect;
        QApplication::style()->drawControl(QStyle::CE_RadioButton, &option, painter);
        //QRadioButton rb;
        //option2.init(&rb);
    }
}

QSize RadioDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    return { 24, 24 };
    //    if (index.data().canConvert<StarRating>()) {
    //        StarRating starRating = qvariant_cast<StarRating>(index.data());
    //        return starRating.sizeHint();
    //    } else {
    //    return QStyledItemDelegate::sizeHint(option, index);
    //    }
}

void RadioDelegate::commitAndCloseEditor()
{
    // QRadioButton* editor = qobject_cast<QRadioButton*>(sender());
    // emit commitData(editor);
    // emit closeEditor(editor);
}
#endif

#include "layerdelegate.h"
#include "gerbernode.h"

#include <QApplication>
#include <QComboBox>
#include <QRadioButton>

LayerDelegate::LayerDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

LayerDelegate::~LayerDelegate() {}

QWidget* LayerDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    QComboBox* comboBox = new QComboBox(parent);
    comboBox->addItems({ tr("Top"), tr("Bottom") });
    comboBox->setCurrentIndex(index.model()->data(index).toInt());
    //        if (index.column() == 1)
    //            comboBox->addItems(IconPreviewArea::iconModeNames());
    //        else if (index.column() == 2)
    //            comboBox->addItems(IconPreviewArea::iconStateNames());
    //connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, &LayerDelegate::emitCommitData);
    return comboBox;
}

void LayerDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
    comboBox->showPopup();
}

void LayerDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    model->setData(index, bool(comboBox->currentIndex()));
}

void LayerDelegate::emitCommitData()
{
    emit commitData(qobject_cast<QWidget*>(sender()));
}

RadioDelegate::RadioDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

RadioDelegate::~RadioDelegate() {}

QWidget* RadioDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const
{
    QRadioButton* radioButton = new QRadioButton(parent);
    //    radioButton->addItems({ tr("Top"), tr("Bottom") });
    //        if (index.column() == 1)
    //            comboBox->addItems(IconPreviewArea::iconModeNames());
    //        else if (index.column() == 2)
    //            comboBox->addItems(IconPreviewArea::iconStateNames());
    //connect(comboBox, QOverload<int>::of(&QComboBox::activated), this, &LayerDelegate::emitCommitData);
    connect(radioButton, &QRadioButton::clicked, this, &RadioDelegate::commitAndCloseEditor);
    return radioButton;
}

void RadioDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QRadioButton* radioButton = qobject_cast<QRadioButton*>(editor);
    if (!radioButton)
        return;
    //        int pos = comboBox->findText(index.model()->data(index).toString(),
    //            Qt::MatchExactly);
    radioButton->setChecked(index.model()->data(index).toInt());
}

void RadioDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QRadioButton* radioButton = qobject_cast<QRadioButton*>(editor);
    if (!radioButton)
        return;
    model->setData(index, bool(radioButton->isChecked()));
}

void RadioDelegate::paint(QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QStyledItemDelegate::paint(painter, option, index);
    if (dynamic_cast<GerberNode*>(reinterpret_cast<AbstractNode*>(index.internalPointer()))) {

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

QSize RadioDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    return { 24, 24 };
    //    if (index.data().canConvert<StarRating>()) {
    //        StarRating starRating = qvariant_cast<StarRating>(index.data());
    //        return starRating.sizeHint();
    //    } else {
    return QStyledItemDelegate::sizeHint(option, index);
    //    }
}

void RadioDelegate::commitAndCloseEditor()
{
    // QRadioButton* editor = qobject_cast<QRadioButton*>(sender());
    // emit commitData(editor);
    // emit closeEditor(editor);
}

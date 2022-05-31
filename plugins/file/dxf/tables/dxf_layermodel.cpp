// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ***********************************************************8********************/
#include "dxf_layermodel.h"
#include "dxf_file.h"
#include "dxf_layer.h"
#include "ft_node.h"
#include "itemgroup.h"

#include <QComboBox>
#include <QGraphicsScene>
#include <QPainter>

namespace Dxf {

QStringList keys(const Layers& layers) {
    QStringList sl;
    sl.reserve(static_cast<int>(layers.size()));
    for (auto& [key, _] : layers)
        sl.append(key);
    return sl;
}

LayerModel::LayerModel(Layers layers, QObject* parent)
    : QAbstractTableModel(parent)
    , layers(layers)
    , names(keys(layers)) {
}

int LayerModel::rowCount(const QModelIndex& /*parent*/) const { return names.size(); }

int LayerModel::columnCount(const QModelIndex& /*parent*/) const { return ColumnCount; }

QVariant LayerModel::data(const QModelIndex& index, int role) const {
    switch (index.column()) {
    case Visible:
        switch (role) {
        case Qt::DisplayRole:
            return {};
        case Qt::CheckStateRole:
            if (layers.at(names[index.row()])->itemGroup())
                return (layers.at(names[index.row()])->itemGroup()->isVisible()) ? Qt::Checked : Qt::Unchecked;
            return {};
        case Qt::EditRole:
            if (layers.at(names[index.row()])->itemGroup())
                return static_cast<int>(layers.at(names[index.row()])->itemGroup()->isVisible());
            return {};
        case Qt::DecorationRole:
            return decoration(layers.at(names[index.row()])->color());
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        }
        return {};
    case EntityCount:
        switch (role) {
        case Qt::DisplayRole:
            if (layers.at(names[index.row()])->itemGroup())
                return static_cast<int>(layers.at(names[index.row()])->itemGroup()->size());
            return DxfObj::tr("Empty layer");
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        }
        return {};
    case Type:
        switch (role) {
        case Qt::DisplayRole:
            if (layers.at(names[index.row()])->itemGroup()) {
                static const QString ar[] { DxfObj::tr("Solid"), DxfObj::tr("Paths") };
                return ar[static_cast<int>(layers.at(names[index.row()])->itemsType())];
            }
            return DxfObj::tr("Empty layer");
        case Qt::EditRole:
            return static_cast<int>(layers.at(names[index.row()])->itemsType());
        case Qt::TextAlignmentRole:
            return Qt::AlignCenter;
        }
        return {};
    default:
        return {};
    }
}

bool LayerModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    switch (index.column()) {
    case Visible:
        switch (role) {
        case Qt::CheckStateRole:
            layers[names[index.row()]]->itemGroup()->setVisible(value.toInt());
            layers[names[index.row()]]->file()->m_visible = value.value<Qt::CheckState>() == Qt::Checked;
            return true;
        case Qt::DecorationRole:
            layers[names[index.row()]]->setColor(value.value<QColor>());
            if (layers.at(names[index.row()])->itemGroup())
                for (auto gi : *layers[names[index.row()]]->itemGroup())
                    gi->changeColor();
            return true;
        default:
            return false;
        }
    case EntityCount:
    case Type:
        switch (role) {
        case Qt::EditRole:
            layers.at(names[index.row()])->setItemsType(static_cast<ItemsType>(value.toInt()));
            return true;
        default:
            return {};
        }

    default:
        return false;
    }
    return false;
}

QVariant LayerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal) {
            switch (section) {
            case Visible:
                return DxfObj::tr("Visible\n& color");
            case EntityCount:
                return DxfObj::tr("Entity\ncount");
            case Type:
                return DxfObj::tr("Visible\ntype");
            }
        } else {
            return names[section];
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags LayerModel::flags(const QModelIndex& index) const {
    auto flags = Qt::ItemIsEnabled;
    switch (index.column()) {
    case Visible:
        return flags | Qt::ItemIsUserCheckable;
    case EntityCount:
        return flags;
    case Type:
        return flags | ((layers.at(names[index.row()])->itemGroup()) ? Qt::ItemIsEditable : Qt::NoItemFlags);
    default:
        return flags;
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
/// \brief ItemsTypeDelegate::ItemsTypeDelegate
/// \param parent
///
ItemsTypeDelegate::ItemsTypeDelegate(QObject* parent)
    : QStyledItemDelegate(parent) {
}

QWidget* ItemsTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const {
    auto* comboBox = new QComboBox(parent);
    comboBox->addItems({ DxfObj::tr("Solid"), DxfObj::tr("Paths") });
    comboBox->setItemData(0, comboBox->size(), Qt::SizeHintRole);
    comboBox->setItemData(1, comboBox->size(), Qt::SizeHintRole);
    connect(comboBox, qOverload<int>(&QComboBox::activated), this, &ItemsTypeDelegate::emitCommitData);
    return comboBox;
}

void ItemsTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
    comboBox->showPopup();
}

void ItemsTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto* comboBox = qobject_cast<QComboBox*>(editor);
    if (!comboBox)
        return;
    model->setData(index, bool(comboBox->currentIndex()));
}

void ItemsTypeDelegate::emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

} // namespace Dxf

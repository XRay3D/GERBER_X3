/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "topor_layermodel.h"
#include "gi_group.h"
#include "topor_file.h"

#include <QComboBox>

namespace TopoR {

LayerModel::LayerModel(LayerList layers, QObject* parent)
    : QAbstractTableModel{parent}
    , layers_(std::move(layers)) {
}

int LayerModel::rowCount(const QModelIndex& /*parent*/) const { return static_cast<int>(layers_.size()); }

int LayerModel::columnCount(const QModelIndex& /*parent*/) const { return ColumnCount; }

QVariant LayerModel::data(const QModelIndex& index, int role) const {
    Layer* layer = layers_[index.row()];
    switch(index.column()) {
    case Visible:
        switch(role) {
        case Qt::DisplayRole: return {};
        case Qt::CheckStateRole:
            if(layer->itemGroup()) return layer->itemGroup()->isVisible() ? Qt::Checked : Qt::Unchecked;
            return {};
        case Qt::EditRole:
            if(layer->itemGroup()) return static_cast<int>(layer->itemGroup()->isVisible());
            return {};
        case Qt::DecorationRole   : return decoration(layer->color());
        case Qt::TextAlignmentRole: return Qt::AlignCenter;
        }
        return {};
    case EntityCount:
        switch(role) {
        case Qt::DisplayRole:
            if(layer->itemGroup()) return static_cast<int>(layer->itemGroup()->size());
            return tr("Empty layer");
        case Qt::TextAlignmentRole: return Qt::AlignCenter;
        }
        return {};
    case Type:
        switch(role) {
        case Qt::DisplayRole: {
            if(layer->itemGroup()) {
                static const QString ar[]{tr("Normal"), tr("Paths")};
                return ar[static_cast<int>(layer->itemsType())];
            }
            return tr("Empty layer");
        }
        case Qt::EditRole         : return static_cast<int>(layer->itemsType());
        case Qt::TextAlignmentRole: return Qt::AlignCenter;
        }
        return {};
    default: return {};
    }
}

bool LayerModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    Layer* layer = layers_[index.row()];
    switch(index.column()) {
    case Visible:
        switch(role) {
        case Qt::CheckStateRole:
            layer->itemGroup()->setVisible(value.toInt());
            return true;
        case Qt::DecorationRole:
            layer->setColor(value.value<QColor>());
            return true;
        default: return false;
        }
    case EntityCount:
    case Type:
        switch(role) {
        case Qt::EditRole:
            layer->setItemsType(static_cast<ItemsType>(value.toInt()));
            return true;
        default: return {};
        }
    default: return false;
    }
}

QVariant LayerModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole) {
        if(orientation == Qt::Horizontal) {
            switch(section) {
            case Visible    : return tr("Visible\n& color");
            case EntityCount: return tr("Entity\ncount");
            case Type       : return tr("Visible\ntype");
            }
        } else {
            return layers_[section]->name();
        }
    }
    return QAbstractTableModel::headerData(section, orientation, role);
}

Qt::ItemFlags LayerModel::flags(const QModelIndex& index) const {
    auto flags = Qt::ItemIsEnabled;
    switch(index.column()) {
    case Visible    : return flags | Qt::ItemIsUserCheckable;
    case EntityCount: return flags;
    case Type       : return flags | (layers_[index.row()]->itemGroup() ? Qt::ItemIsEditable : Qt::NoItemFlags);
    default         : return flags;
    }
}

ItemsTypeDelegate::ItemsTypeDelegate(QObject* parent)
    : QStyledItemDelegate{parent} {
}

QWidget* ItemsTypeDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& /*option*/, const QModelIndex& /*index*/) const {
    auto* comboBox = new QComboBox{parent};
    comboBox->addItems({tr("Normal"), tr("Paths")});
    connect(comboBox, qOverload<int>(&QComboBox::activated), this, &ItemsTypeDelegate::emitCommitData);
    return comboBox;
}

void ItemsTypeDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const {
    auto* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox) return;
    comboBox->setCurrentIndex(index.data(Qt::EditRole).toInt());
    comboBox->showPopup();
}

void ItemsTypeDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const {
    auto* comboBox = qobject_cast<QComboBox*>(editor);
    if(!comboBox) return;
    model->setData(index, comboBox->currentIndex());
}

void ItemsTypeDelegate::emitCommitData() { emit commitData(qobject_cast<QWidget*>(sender())); }

} // namespace TopoR

#include "moc_topor_layermodel.cpp"

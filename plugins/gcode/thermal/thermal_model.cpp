// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "thermal_model.h"
#include "thermal_node.h"

#include <QIcon>

namespace Thermal {

QIcon Model::repaint(QColor color, const QIcon& icon) const {
    QImage image(icon.pixmap(24, 24).toImage());
    for(int x = 0; x < 24; ++x)
        for(int y = 0; y < 24; ++y) {
            color.setAlpha(image.pixelColor(x, y).alpha());
            image.setPixelColor(x, y, color);
        }
    return QIcon(QPixmap::fromImage(image));
}

Model::Model(QObject* parent)
    : QAbstractItemModel{parent}
    , rootItem(new Node{this}) {
}

Model::~Model() { delete rootItem; }

Node* Model::appendRow(const QIcon& icon, const QString& name, const ThParam& par) {
    data_.push_back(new Node{icon, name, par, this});
    rootItem->append(data_.back());
    return data_.back();
}

int Model::rowCount(const QModelIndex& parent) const {
    if(parent.column() > 0)
        return 0;
    return getItem(parent)->childCount();
}

int Model::columnCount(const QModelIndex& /*parent*/) const { return ColumnCount; }

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const {
    Node* childItem = getItem(parent)->child(row);
    if(childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex& index) const {

    if(!index.isValid())
        return QModelIndex();

    Node* parentItem = getItem(index)->parentItem();

    if(parentItem == rootItem)
        return QModelIndex();

    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant Model::data(const QModelIndex& index, int role) const {
    if(!index.isValid())
        return {};
    Node* item = getItem(index);
    return item->data(index, role);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(!index.isValid())
        return false;
    const bool result = getItem(index)->setData(index, value, role);
    return result;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
    static const QStringList horizontalLabel{tr("     Name|Angle|Tickness|Count").split('|')};
    switch(role) {
    case Qt::DisplayRole:
        if(orientation == Qt::Horizontal)
            return horizontalLabel[section];
        else
            return section + 1;
    default:
        return {};
    }
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    if(!index.isValid())
        return Qt::NoItemFlags;
    Node* item = getItem(index);
    return item->flags(index);
}

bool Model::removeRows(int row, int count, const QModelIndex& parent) {
    Node* item = nullptr;
    if(parent.isValid())
        item = static_cast<Node*>(parent.internalPointer());
    else
        return false;
    beginRemoveRows(parent, row, row + count - 1);
    while(count--)
        item->remove(row);
    endRemoveRows();
    resetInternalData();
    return true;
}

ThParam Model::thParam() { return data_.front()->getPar(); }

Node* Model::getItem(const QModelIndex& index) const {
    if(index.isValid()) {
        auto* item = static_cast<Node*>(index.internalPointer());
        if(item)
            return item;
    }
    return rootItem;
}

} // namespace Thermal

#include "moc_thermal_model.cpp"

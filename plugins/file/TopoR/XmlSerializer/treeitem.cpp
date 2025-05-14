#include "treeitem.h"
#include <QColor>

// TreeItem::TreeItem(const QVector<QVariant>& data, TreeItem* parent)
//     : itemData(data)
//     , parentItem(parent) { }

TreeItem::~TreeItem() { qDeleteAll(childItems); }

QVariant TreeItem::data(uint column, int role) const {
    if(column >= itemData.size()) return {};
    if(role == Qt::DisplayRole || role == Qt::EditRole)
        return itemData.at(column);
    if(role == Qt::BackgroundRole && itemData.at(column).toString().startsWith('#')) {
        return QColor::fromString(itemData.at(column).toString());
    }
    return {};
}

TreeItem* TreeItem::child(uint number) {
    if(number >= childItems.size()) return nullptr;
    return childItems.at(number);
}

TreeItem* TreeItem::parent() { return parentItem; }

// bool TreeItem::insertChildren(int position, int count, int columns) {
//     if(position < 0 || position > childItems.size()) return false;
//     for(int row = 0; row < count; ++row) {
//         QVector<QVariant> data(columns);
//         TreeItem* item = new TreeItem(data, this);
//         childItems.insert(position, item);
//     }
//     return true;
// }

// bool TreeItem::insertColumns(int position, int columns) {
//     if(position < 0 || position > itemData.size()) return false;
//     for(int column = 0; column < columns; ++column) itemData.insert(position, QVariant());
//     for(TreeItem* child: std::as_const(childItems)) child->insertColumns(position, columns);
//     return true;
// }

// bool TreeItem::removeChildren(int position, int count) {
//     if(position < 0 || position + count > childItems.size()) return false;
//     for(int row = 0; row < count; ++row) delete childItems.takeAt(position);
//     return true;
// }

// bool TreeItem::removeColumns(int position, int columns) {
//     if(position < 0 || position + columns > itemData.size()) return false;
//     for(int column = 0; column < columns; ++column) itemData.erase(itemData.begin() + position);
//     for(TreeItem* child: std::as_const(childItems)) child->removeColumns(position, columns);
//     return true;
// }

bool TreeItem::setData(uint column, const QVariant& value) {
    if(column >= itemData.size()) return false;
    itemData[column] = value;
    return true;
}

int TreeItem::childCount() const { return childItems.size(); }

int TreeItem::childNumber() const {
    if(parentItem)
        if(auto it = std::ranges::find(parentItem->childItems, this); it != parentItem->childItems.end())
            return std::distance(parentItem->childItems.begin(), it);
    return 0;
}

int TreeItem::columnCount() const { return itemData.size(); }

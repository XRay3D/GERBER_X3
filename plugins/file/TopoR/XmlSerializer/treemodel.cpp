
#include "treemodel.h"
#include "treeitem.h"

#include <QtWidgets>

TreeModel::TreeModel(TreeItem* rootItem, const QStringList& /*headers*/, QObject* parent)
    : QAbstractItemModel{parent}
    , rootItem{rootItem} {
}

TreeModel::~TreeModel() { delete rootItem; }

int TreeModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return rootItem->columnCount();
}

QVariant TreeModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid())
        return {};

    // if(role != Qt::DisplayRole && role != Qt::EditRole)
    // return {};

    TreeItem* item = getItem(index);

    return item->data(index.column(), role);
}

Qt::ItemFlags TreeModel::flags(const QModelIndex& index) const {
    if(!index.isValid())
        return Qt::NoItemFlags;

    return Qt::ItemIsEditable | QAbstractItemModel::flags(index);
}

TreeItem* TreeModel::getItem(const QModelIndex& index) const {
    if(index.isValid()) {
        TreeItem* item = static_cast<TreeItem*>(index.internalPointer());
        if(item)
            return item;
    }
    return rootItem;
}

QVariant TreeModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(orientation == Qt::Horizontal && role == Qt::DisplayRole)
        return rootItem->data(section, role);
    return {};
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
    if(parent.isValid() && parent.column() != 0)
        return QModelIndex();

    TreeItem* parentItem = getItem(parent);
    if(!parentItem)
        return QModelIndex();

    TreeItem* childItem = parentItem->child(row);
    if(childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

// bool TreeModel::insertColumns(int position, int columns, const QModelIndex& parent) {
//     beginInsertColumns(parent, position, position + columns - 1);
//     const bool success = rootItem->insertColumns(position, columns);
//     endInsertColumns();

//     return success;
// }

// bool TreeModel::insertRows(int position, int rows, const QModelIndex& parent) {
//     TreeItem* parentItem = getItem(parent);
//     if(!parentItem)
//         return false;

//     beginInsertRows(parent, position, position + rows - 1);
//     const bool success = parentItem->insertChildren(position,
//         rows,
//         rootItem->columnCount());
//     endInsertRows();

//     return success;
// }

QModelIndex TreeModel::parent(const QModelIndex& index) const {
    if(!index.isValid())
        return QModelIndex();

    TreeItem* childItem = getItem(index);
    TreeItem* parentItem = childItem ? childItem->parent() : nullptr;

    if(parentItem == rootItem || !parentItem)
        return QModelIndex();

    return createIndex(parentItem->childNumber(), 0, parentItem);
}

// bool TreeModel::removeColumns(int position, int columns, const QModelIndex& parent) {
//     beginRemoveColumns(parent, position, position + columns - 1);
//     const bool success = rootItem->removeColumns(position, columns);
//     endRemoveColumns();

//     if(rootItem->columnCount() == 0)
//         removeRows(0, rowCount());

//     return success;
// }

// bool TreeModel::removeRows(int position, int rows, const QModelIndex& parent) {
//     TreeItem* parentItem = getItem(parent);
//     if(!parentItem)
//         return false;

//     beginRemoveRows(parent, position, position + rows - 1);
//     const bool success = parentItem->removeChildren(position, rows);
//     endRemoveRows();

//     return success;
// }

int TreeModel::rowCount(const QModelIndex& parent) const {
    if(parent.isValid() && parent.column() > 0)
        return 0;

    const TreeItem* parentItem = getItem(parent);

    return parentItem ? parentItem->childCount() : 0;
}

bool TreeModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(role != Qt::EditRole)
        return false;

    TreeItem* item = getItem(index);
    bool result = item->setData(index.column(), value);

    if(result)
        emit dataChanged(index, index, {Qt::DisplayRole, Qt::EditRole});

    return result;
}

bool TreeModel::setHeaderData(int section, Qt::Orientation orientation,
    const QVariant& value, int role) {
    if(role != Qt::EditRole || orientation != Qt::Horizontal)
        return false;

    const bool result = rootItem->setData(section, value);

    if(result)
        emit headerDataChanged(orientation, section, section);

    return result;
}

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "ft_model.h"
#include "ft_foldernode.h"

#include "abstract_file.h"
#include "abstract_fileplugin.h"
#include "abstract_shape.h"
#include "gc_types.h"
#include "md5.h"
#include "project.h"

#include <QTimer>

namespace FileTree {

using TreeItem = Node;

Model::Model(QObject* parent)
    : QAbstractItemModel{parent}
    , rootItem(new FolderNode{u"rootItem"_s})
    , mimeType(u"application/GCodeItem"_s) {
    App::setFileModel(this);
}

Model::~Model() {
    delete rootItem;
    App::setFileModel(nullptr);
}

QModelIndex Model::nodeIndex(Node* node) const {
    if(!node || node == rootItem)
        return {}; // корень дерева — невалидный индекс
    return createIndex(node->row(), 0, node);
}

Node* Model::folderNode(uint32_t type, const QString& name) {
    auto& itemFolder = fileFolders[type];
    if(!itemFolder) {
        auto folder = new FolderNode{name, static_cast<int32_t>(type)};
        appendChild(rootItem, folder);
        itemFolder = folder;
    }
    return itemFolder;
}

int Model::appendChild(Node* folder, Node* node) {
    int rowCount = folder->childCount();
    beginInsertRows(nodeIndex(folder), rowCount, rowCount);
    folder->addChild(node);
    endInsertRows();
    return rowCount;
}

int Model::addFile(Node* item, AbstractFile* file) {
    return appendChild(item, file->node());
}

void Model::addFile(AbstractFile* file) {
    if(!file)
        return;
    uint32_t type = file->type();
    Node* node = file->node();
    if(App::filePlugins().contains(type)) {
        Node* itemFolder = folderNode(type, App::filePlugin(type)->folderName());
        int rowCount = addFile(itemFolder, file);
        App::filePlugin(type)->updateFileModel(file);
        emit select(createIndex(rowCount, 0, node));
    } else if(App::gCodePlugins().contains(type)) {
        type = G_CODE;
        Node* itemFolder = folderNode(type, tr("GCode"));
        int rowCount = addFile(itemFolder, file);
        emit select(createIndex(rowCount, 0, node));
    } else if(type == GC_DBG_FILE) {
        Node* itemFolder = folderNode(type, u"GCode Debug"_s);
        int rowCount = addFile(itemFolder, file);
        emit select(createIndex(rowCount, 0, node));
    } else if(type == Gi::Type::Debug) {
        Node* itemFolder = folderNode(type, u"Gi::Debug"_s);
        int rowCount = appendChild(itemFolder, node);
        emit select(createIndex(rowCount, 0, node));
    }
}

void Model::addShape(Shapes::AbstractShape* shape) {
    if(!shape)
        return;

    static constexpr uint32_t type = "Shapes"_hash32;
    auto si = App::shapePlugins().begin()->second;
    appendChild(folderNode(type, si->folderName()), shape);
    // emit select(createIndex(rowCount, 0, shape /*->node()*/));
}

void Model::addItem(Gi::Item* item) {
    if(!item)
        return;

    static constexpr uint32_t type = "Gi::Item"_hash32;
    auto si = App::shapePlugins().begin()->second;
    auto node = new ItemNode{item};
    int rowCount = appendChild(folderNode(type, si->folderName()), node);
    emit select(createIndex(rowCount, 0, node));
}

void Model::closeProject() {
    Node* item;
    for(int i{}; i < rootItem->childCount(); ++i) {
        item = rootItem->child(i);
        QModelIndex index = createIndex(i, 0, item);
        int rowCount = item->childCount();
        if(rowCount) {
            beginRemoveRows(index, 0, rowCount - 1);
            for(int j{}; j < rowCount; ++j)
                item->remove(0);
            endRemoveRows();
        }
    }
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const {
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;

    parentItem = !parent.isValid() ? rootItem : static_cast<TreeItem*>(parent.internalPointer());

    TreeItem* childItem = parentItem->child(row);
    if(childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
    // Node* childItem = getItem(parent)->child(row);
    // if (childItem)
    // return createIndex(row, column, childItem);
    // return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex& index) const {
    // if (!index.isValid())
    // return QModelIndex();
    // TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    // TreeItem* parentItem = childItem->parent();
    // if (parentItem == rootItem)
    // return QModelIndex();
    // return createIndex(parentItem->row(), 0, parentItem);

    if(!index.isValid())
        return QModelIndex();
    Node* parentItem = getItem(index)->parent();
    if(parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), /*index.column()*/ 0, parentItem);
}

QVariant Model::data(const QModelIndex& index, int role) const {
    if(!index.isValid())
        return {};
    return getItem(index)->data(index, role);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    bool ok = getItem(index)->setData(index, value, role);
    if(ok)
        App::project().setChanged();
    return ok;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if((role == Qt::DisplayRole || role == Qt::ToolTipRole) && orientation == Qt::Horizontal) {
        static const QStringList names{tr("Name"), tr("Side"), tr("Type")};
        return names[section];
    }
    return {};
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    if(!index.isValid()) {
        qDebug() << index;
        return Qt::NoItemFlags;
    }
    return getItem(index)->flags(index);
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

int Model::columnCount(const QModelIndex& /*parent*/) const {
    return int(Column::Count);
}

int Model::rowCount(const QModelIndex& parent) const {
    TreeItem* parentItem;
    if(parent.column() > 0)
        return 0;

    parentItem = !parent.isValid() ? rootItem : static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
    // if (parent.column() > 0)
    // return 0;
    // return getItem(parent)->childCount();
}

Node* Model::getItem(const QModelIndex& index) const {
    if(index.isValid()) {
        auto* item = static_cast<Node*>(index.internalPointer());
        if(item) return item;
    }
    return rootItem;
}

} // namespace FileTree

#include "moc_ft_model.cpp"

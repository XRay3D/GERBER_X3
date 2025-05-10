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
#include "ft_model.h"
#include "ft_foldernode.h"

#include "abstract_file.h"
#include "abstract_fileplugin.h"
#include "gc_types.h"
#include "md5.h"
#include "project.h"
#include "shape.h"

#include <QTimer>

namespace FileTree {

using TreeItem = Node;

Model::Model(QObject* parent)
    : QAbstractItemModel{parent}
    , rootItem(new FolderNode{"rootItem"})
    , mimeType(u"application/GCodeItem"_s) {
    App::setFileModel(this);
}

Model::~Model() {
    delete rootItem;
    App::setFileModel(nullptr);
}

int Model::addFile(Node* item, AbstractFile* file) {
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    beginInsertRows(index, rowCount, rowCount);
    item->addChild(file->node());
    endInsertRows();
    return rowCount;
}

void Model::addFile(AbstractFile* file) {
    if(!file)
        return;
    uint32_t type = file->type();
    if(App::filePlugins().contains(type)) {
        auto& itemFolder = fileFolders[type];
        if(!itemFolder) {
            QModelIndex index = createIndex(0, 0, rootItem);
            int rowCount = rootItem->childCount();
            beginInsertRows(index, rowCount, rowCount);
            itemFolder = new FolderNode{App::filePlugin(type)->folderName(), static_cast<int32_t>(type)};
            rootItem->addChild(itemFolder);
            endInsertRows();
        }
        int rowCount = addFile(itemFolder, file);
        App::filePlugin(type)->updateFileModel(file);
        emit select(createIndex(rowCount, 0, file->node()));
    } else if(App::gCodePlugins().contains(type)) {
        type = G_CODE;
        auto& itemFolder = fileFolders[type];
        if(!itemFolder) {
            QModelIndex index = createIndex(0, 0, rootItem);
            int rowCount = rootItem->childCount();
            beginInsertRows(index, rowCount, rowCount);
            itemFolder = new FolderNode{tr("GCode"), static_cast<int32_t>(type)};
            rootItem->addChild(itemFolder);
            endInsertRows();
        }
        int rowCount = addFile(itemFolder, file);
        emit select(createIndex(rowCount, 0, file->node()));
    } else if(type == GC_DBG_FILE) {
        auto& itemFolder = fileFolders[type];
        if(!itemFolder) {
            QModelIndex index = createIndex(0, 0, rootItem);
            int rowCount = rootItem->childCount();
            beginInsertRows(index, rowCount, rowCount);
            itemFolder = new FolderNode{"GCode Debug", static_cast<int32_t>(type)};
            rootItem->addChild(itemFolder);
            endInsertRows();
        }
        int rowCount = addFile(itemFolder, file);
        emit select(createIndex(rowCount, 0, file->node()));
    }
}

void Model::addShape(Shapes::AbstractShape* shape) {
    if(!shape)
        return;

    static constexpr uint32_t type = md5::hash32("Shapes");
    auto& itemFolder = fileFolders[type];
    if(!itemFolder) {
        QModelIndex index = createIndex(0, 0, rootItem);
        int rowCount = rootItem->childCount();
        beginInsertRows(index, rowCount, rowCount);
        auto si = App::shapePlugins().begin()->second;
        itemFolder = new FolderNode{si->folderName(), type};
        rootItem->addChild(itemFolder);
        endInsertRows();
    }

    QModelIndex index = createIndex(0, 0, itemFolder);
    int rowCount = itemFolder->childCount();
    beginInsertRows(index, rowCount, rowCount);
    itemFolder->addChild(shape); //, Node::DontDelete);

    endInsertRows();
    // emit select(createIndex(rowCount, 0, shape /*->node()*/));
}

void Model::addItem(Gi::Item* item) {
    if(!item)
        return;

    static constexpr uint32_t type = md5::hash32("Gi::Item");
    auto& itemFolder = fileFolders[type];
    if(!itemFolder) {
        QModelIndex index = createIndex(0, 0, rootItem);
        int rowCount = rootItem->childCount();
        beginInsertRows(index, rowCount, rowCount);
        auto si = App::shapePlugins().begin()->second;
        itemFolder = new FolderNode{si->folderName(), static_cast<int32_t>(type)};
        rootItem->addChild(itemFolder);
        endInsertRows();
    }

    QModelIndex index = createIndex(0, 0, itemFolder);
    int rowCount = itemFolder->childCount();
    beginInsertRows(index, rowCount, rowCount);

    auto node = new ItemNode{item};

    itemFolder->addChild(node);
    endInsertRows();

    emit select(createIndex(rowCount, 0, node));

    //    QModelIndex index = createIndex(0, 0, itemFolder);
    //    int rowCount = itemFolder->childCount();
    //    beginInsertRows(index, rowCount, rowCount);
    //    itemFolder->addChild(item); //, Node::DontDelete);
    //    endInsertRows();
    // emit select(createIndex(rowCount, 0, shape /*->node()*/));
}

void Model::closeProject() {
    Node* item;
    for(int i = 0; i < rootItem->childCount(); ++i) {
        item = rootItem->child(i);
        QModelIndex index = createIndex(i, 0, item);
        int rowCount = item->childCount();
        if(rowCount) {
            beginRemoveRows(index, 0, rowCount - 1);
            for(int j = 0; j < rowCount; ++j)
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
    //    Node* childItem = getItem(parent)->child(row);
    //    if (childItem)
    //        return createIndex(row, column, childItem);
    //    return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex& index) const {
    //    if (!index.isValid())
    //        return QModelIndex();
    //    TreeItem* childItem = static_cast<TreeItem*>(index.internalPointer());
    //    TreeItem* parentItem = childItem->parent();
    //    if (parentItem == rootItem)
    //        return QModelIndex();
    //    return createIndex(parentItem->row(), 0, parentItem);

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
    if((role == Qt::DisplayRole || role == Qt::ToolTipRole) && orientation == Qt::Horizontal)
        switch(section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Side");
        case 2:
            return tr("Type");
        default:
            return QString("");
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
    //    if (parent.column() > 0)
    //        return 0;
    //    return getItem(parent)->childCount();
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

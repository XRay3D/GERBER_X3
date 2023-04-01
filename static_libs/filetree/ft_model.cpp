// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "ft_model.h"
#include "ft_foldernode.h"

#include "abstract_file.h"
#include "abstract_fileplugin.h"
// #include "gc_file.h"
// #include "gc_plugin.h"
#include "md5.h"
#include "project.h"
// #include "shapepluginin.h"
// #include "shnode.h"

#include <QTimer>

namespace FileTree {

using TreeItem = Node;

Model::Model(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new FolderNode("rootItem"))
    , mimeType(QStringLiteral("application/GCodeItem")) {
    App::setFileModel(this);
}

Model::~Model() {
    delete rootItem;
    App::setFileModel(nullptr);
}

int Model::addFile(uint32_t type, AbstractFile* file) {
    Node* item(fileFolders[type]);
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    beginInsertRows(index, rowCount, rowCount);
    fileFolders[type]->addChild(file->node());
    endInsertRows();
    return rowCount;
}

void Model::addFile(AbstractFile* file) {
    if (!file)
        return;

    uint32_t type = file->type();

    if (App::filePlugins().contains(type)) {

        if (fileFolders.find(type) == fileFolders.end()) {
            QModelIndex index = createIndex(0, 0, rootItem);
            int rowCount = rootItem->childCount();
            beginInsertRows(index, rowCount, rowCount);
            auto it = fileFolders[type] = new FolderNode(App::filePlugin(type)->folderName(), type);
            rootItem->addChild(it);
            endInsertRows();
        }

        int rowCount = addFile(type, file);
        App::filePlugin(type)->updateFileModel(file);
        emit select(createIndex(rowCount, 0, file->node()));
    } else if (App::gCodePlugins().contains(type)) {
        type = G_CODE;
        if (fileFolders.find(type) == fileFolders.end()) {
            QModelIndex index = createIndex(0, 0, rootItem);
            int rowCount = rootItem->childCount();
            beginInsertRows(index, rowCount, rowCount);
            auto it = fileFolders[type] = new FolderNode(tr("GCode"), type);
            rootItem->addChild(it);
            endInsertRows();
        }

        int rowCount = addFile(type, file);
        //    App::gCodePlugin(type)->updateFileModel(file);
        emit select(createIndex(rowCount, 0, file->node()));
    } else if (type == G_CODE) {
        type = G_CODE + 1;
        if (fileFolders.find(type) == fileFolders.end()) {
            QModelIndex index = createIndex(0, 0, rootItem);
            int rowCount = rootItem->childCount();
            beginInsertRows(index, rowCount, rowCount);
            auto it = fileFolders[type] = new FolderNode("GCode Debug", type);
            rootItem->addChild(it);
            endInsertRows();
        }
        int rowCount = addFile(type, file);
        emit select(createIndex(rowCount, 0, file->node()));
    }
}

void Model::addShape(Shapes::AbstractShape* shape) {
    if (!shape)
        return;

    // FIXME   uint32_t type = FileType::Shapes_;

    //    if (fileFolders.find(type) == fileFolders.end()) {
    //        QModelIndex index = createIndex(0, 0, rootItem);
    //        int rowCount = rootItem->childCount();
    //        beginInsertRows(index, rowCount, rowCount);
    //        auto si = App::shapePlugins().begin()->second;
    //        auto it = fileFolders[type] = new FolderNode(si->folderName(), type);
    //        rootItem->addChild(it);
    //        endInsertRows();
    //    }

    //    Node* item(fileFolders[type]);
    //    QModelIndex index = createIndex(0, 0, item);
    //    int rowCount = item->childCount();
    //    beginInsertRows(index, rowCount, rowCount);
    //    item->addChild(shape->node());
    //    endInsertRows();

    //    emit select(createIndex(rowCount, 0, shape->node()));
}

void Model::closeProject() {
    Node* item;
    for (int i = 0; i < rootItem->childCount(); ++i) {
        item = rootItem->child(i);
        QModelIndex index = createIndex(i, 0, item);
        int rowCount = item->childCount();
        if (rowCount) {
            beginRemoveRows(index, 0, rowCount - 1);
            for (int j = 0; j < rowCount; ++j)
                item->remove(0);
            endRemoveRows();
        }
    }
}

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const {
    if (!hasIndex(row, column, parent))
        return QModelIndex();

    TreeItem* parentItem;

    parentItem = !parent.isValid() ? rootItem : static_cast<TreeItem*>(parent.internalPointer());

    TreeItem* childItem = parentItem->child(row);
    if (childItem)
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

    if (!index.isValid())
        return QModelIndex();
    Node* parentItem = getItem(index)->parent();
    if (parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), /*index.column()*/ 0, parentItem);
}

QVariant Model::data(const QModelIndex& index, int role) const {
    if (!index.isValid())
        return QVariant();
    return getItem(index)->data(index, role);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role) {
    bool ok = getItem(index)->setData(index, value, role);
    if (ok)
        App::project()->setChanged();
    return ok;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const {
    if ((role == Qt::DisplayRole || role == Qt::ToolTipRole) && orientation == Qt::Horizontal)
        switch (section) {
        case 0:
            return tr("Name");
        case 1:
            return tr("Side");
        case 2:
            return tr("Type");
        default:
            return QString("");
        }
    return QVariant();
}

Qt::ItemFlags Model::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        qDebug() << index;
        return Qt::NoItemFlags;
    }
    return getItem(index)->flags(index);
}

bool Model::removeRows(int row, int count, const QModelIndex& parent) {
    Node* item = nullptr;
    if (parent.isValid())
        item = static_cast<Node*>(parent.internalPointer());
    else
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    while (count--)
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
    if (parent.column() > 0)
        return 0;

    parentItem = !parent.isValid() ? rootItem : static_cast<TreeItem*>(parent.internalPointer());

    return parentItem->childCount();
    //    if (parent.column() > 0)
    //        return 0;
    //    return getItem(parent)->childCount();
}

Node* Model::getItem(const QModelIndex& index) const {
    if (index.isValid()) {
        auto* item = static_cast<Node*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

} // namespace FileTree

#include "moc_ft_model.cpp"

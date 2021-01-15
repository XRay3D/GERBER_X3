// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "ft_model.h"

#include "ft_foldernode.h"
#include "interfaces/file.h"
#include "interfaces/pluginfile.h"
#include "interfaces/shapepluginin.h"
#include "shheaders.h"

#include <QTimer>

#include "leakdetector.h"

namespace FileTree {

Model::Model(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new FolderNode("rootItem", rotId))
    , mimeType(QStringLiteral("application/GCodeItem"))
{
    App::setFileModel(this);
}

Model::~Model()
{
    delete rootItem;
    App::setFileModel(nullptr);
}

void Model::addFile(FileInterface* file)
{
    if (!file)
        return;

    const int type = int(file->type());

    if (mapNode.find(type) == mapNode.end()) {
        QModelIndex index = createIndex(0, 0, rootItem);
        int rowCount = rootItem->childCount();
        beginInsertRows(index, rowCount, rowCount);
        mapNode.emplace(type, Pair { nullptr, type });
        rootItem->addChild(mapNode[type].node = new FolderNode(App::fileInterface(type)->folderName(), mapNode[type].type));
        endInsertRows();
    }

    FileTree::Node* item(mapNode[type].node);
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    FileTree::Node* newItem;
    beginInsertRows(index, rowCount, rowCount);
    mapNode[type].node->addChild(newItem = file->node());
    endInsertRows();

    assert(newItem);

    QModelIndex selectIndex = createIndex(rowCount, 0, newItem);
    file->setFileIndex(selectIndex);
    App::fileInterface(type)->updateFileModel(file);
    emit select(selectIndex);
}

void Model::addShape(Shapes::Shape* shape)
{
    if (!shape)
        return;

    const int type = int(FileType::Shapes);

    if (mapNode.find(type) == mapNode.end()) {
        QModelIndex index = createIndex(0, 0, rootItem);
        int rowCount = rootItem->childCount();
        beginInsertRows(index, rowCount, rowCount);
        mapNode.emplace(type, Pair { nullptr, type });
        auto si = std::get<0>(App::shapeInterfaces().begin()->second);
        rootItem->addChild(mapNode[type].node = new FolderNode(si->folderName(), mapNode[type].type));
        endInsertRows();
    }

    FileTree::Node* item(mapNode[type].node);
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();
    beginInsertRows(index, rowCount, rowCount);
    item->addChild(shape->node());
    endInsertRows();
    QModelIndex selectIndex = createIndex(rowCount, 0, shape);
    qDebug() << __FUNCTION__ << selectIndex;
    emit select(selectIndex);
    emit select(createIndex(rowCount, 0, shape));
}

void Model::closeProject()
{
    FileTree::Node* item;
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

QModelIndex Model::index(int row, int column, const QModelIndex& parent) const
{
    FileTree::Node* childItem = getItem(parent)->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex Model::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();
    FileTree::Node* parentItem = getItem(index)->parent();
    if (parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant Model::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    FileTree::Node* item = getItem(index);

    return item->data(index, role);
}

bool Model::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool ok = getItem(index)->setData(index, value, role);
    //    if (ok)
    //        App::project()->setChanged();
    return ok;
}

QVariant Model::headerData(int section, Qt::Orientation orientation, int role) const
{
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

Qt::ItemFlags Model::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return getItem(index)->flags(index);
}

bool Model::removeRows(int row, int count, const QModelIndex& parent)
{
    FileTree::Node* item = nullptr;
    if (parent.isValid())
        item = static_cast<FileTree::Node*>(parent.internalPointer());
    else
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    while (count--)
        item->remove(row);
    endRemoveRows();
    resetInternalData();
    return true;
}

int Model::columnCount(const QModelIndex& /*parent*/) const
{
    return int(Column::Count);
}

int Model::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;
    return getItem(parent)->childCount();
}

//QStringList FileModel::mimeTypes() const
//{
//    QStringList types;
//    types << mimeType;
//    return types;
//}

//QMimeData* FileModel::mimeData(const QModelIndexList& indexes) const
//{
//    //    QMimeData* mimeData = new QMimeData();
//    //    QByteArray encodedData;
//    //    int noCopy = -1;
//    //    for (const QModelIndex& index : indexes) {
//    //        if (noCopy != index.row()) {
//    //            noCopy = index.row();
//    //            ToolItem* item = static_cast<ToolItem*>(index.parent().internalPointer());
//    //            if (!item)
//    //                item = rootItem;
//    //            if (index.isValid()) {
//    //                encodedData.push_back(tr("%1,%2").arg(index.row()).arg((quint64)item /*index.internalPointer()*/).toLocal8Bit());
//    //                encodedData.push_back("|");
//    //            }
//    //        }
//    //    }
//    //    mimeData->setData(myModelMimeType(), encodedData);
//    //    return mimeData;
//    QMimeData* mimeData = new QMimeData();
//    QByteArray encodedData;
//    int noCopy = -1;
//    for (const QModelIndex& index : indexes) {
//        if (noCopy != index.row()) {
//            noCopy = index.row();
//            if (index.isValid()) {
//                encodedData.push_back(QString().setNum((quint64)index.internalPointer()).toLocal8Bit());
//                encodedData.push_back("|");
//            }
//        }
//    }
//    mimeData->setData(mimeType, encodedData);
//    return mimeData;
//}

//bool FileModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent)
//{
//    return false;
//    //    if (action == Qt::IgnoreAction)
//    //        return true;

//    //    if (!data->hasFormat(mimeType))
//    //        return false;

//    //    if (column > 0)
//    //        return false;

//    //    int beginRow;

//    //    if (row != -1)
//    //        beginRow = row;
//    //    else if (parent.isValid())
//    //        beginRow = parent.row();
//    //    else
//    //        beginRow = rowCount(QModelIndex());

//    //    QString encodedData = data->data(mimeType);
//    //    QList<QString> list = encodedData.split('|', QString::SkipEmptyParts);

//    //    //    for (QString& item : list) {
//    //    //        QList<QString> d = item.split(',', QString::SkipEmptyParts);
//    //    //        if (d.size() < 2)
//    //    //            return false;
//    //    //        int srcRow = d.at(0).toInt();
//    //    //        ToolItem* ti = reinterpret_cast<ToolItem*>(d.at(1).toLongLong());
//    //    //        QModelIndex index = createIndex(srcRow, 0, ti);
//    //    //        moveRows(index, srcRow, 1, parent, parent.row() > -1 ? parent.row() : 0);
//    //    //    }

//    //    for (QString& item : list) {
//    //        FileTree::Node* copyItem = reinterpret_cast<FileTree::Node*>(item.toLongLong());
//    //        FileTree::Node* parentItem = static_cast<FileTree::Node*>(parent.internalPointer());
//    //        if (copyItem) {
//    //            if (!parentItem)
//    //                parentItem = rootItem;
//    //            insertRows(beginRow, list.size(), parent);
//    //            if (parentItem->childCount() > beginRow)
//    //                parentItem->setChild(beginRow, new FileTree::Node(*copyItem));
//    //            else
//    //                parentItem->setChild(parentItem->childCount() - 1, new FileTree::Node(*copyItem));
//    //        }
//    //        ++beginRow;
//    //    }
//    //    return true;
//}

//Qt::DropActions FileModel::supportedDragActions() const
//{
//    return Qt::MoveAction | Qt::TargetMoveAction;
//}

//Qt::DropActions FileModel::supportedDropActions() const
//{
//    return Qt::MoveAction | Qt::TargetMoveAction;
//}

//bool FileModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild)
//{
//    return false;
//    //    beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
//    //    FileTree::Node* srcItem = static_cast<FileTree::Node*>(sourceParent.internalPointer());
//    //    FileTree::Node* dstItem = static_cast<FileTree::Node*>(destinationParent.internalPointer());
//    //    if (!srcItem)
//    //        srcItem = rootItem;
//    //    if (!dstItem)
//    //        dstItem = rootItem;
//    //    for (int r = 0; r < count; ++r) {
//    //        dstItem->insertChild(destinationChild + r, srcItem->takeChild(sourceRow));
//    //    }
//    //    endMoveRows();
//    //    return true;
//}

Node* Model::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        auto* item = static_cast<FileTree::Node*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

}

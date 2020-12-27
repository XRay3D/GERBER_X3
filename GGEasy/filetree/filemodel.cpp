// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "filemodel.h"

#include "dxf_file.h"
#include "dxf_node.h"
#include "dxf_types.h"
#include "tables/dxf_layer.h"

#include "exnode.h"
#include "foldernode.h"
#include "gbrnode.h"
#include "gcnode.h"
#include "project.h"
#include "shheaders.h"

#include "leakdetector.h"

#include <QTimer>

FileModel::FileModel(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new FolderNode("rootItem"))
    , mimeType(QStringLiteral("application/GCodeItem"))
{
    rootItem->append(new FolderNode(tr("Gerber Files")));
    rootItem->append(new FolderNode(tr("Excellon")));
    rootItem->append(new FolderNode(tr("Tool Paths")));
    rootItem->append(new FolderNode(tr("Dxf Files")));
    rootItem->append(new FolderNode(tr("Shapes")));
    App::m_fileModel = this;
}

FileModel::~FileModel()
{
    delete rootItem;
    App::m_fileModel = nullptr;
}

void FileModel::addFile(AbstractFile* file)
{
    if (!file)
        return;
    AbstractNode* item(rootItem->child(static_cast<int>(file->type())));
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();

    AbstractNode* newItem;
    beginInsertRows(index, rowCount, rowCount);
    switch (file->type()) {
    case FileType::Gerber:
        item->append(newItem = new Gerber::Node(file->id()));
        break;
    case FileType::Excellon:
        item->append(newItem = new Excellon::Node(file->id()));
        break;
    case FileType::GCode:
        item->append(newItem = new GCode::Node(file->id()));
        break;
    case FileType::Dxf:
        item->append(newItem = new Dxf::Node(file->id()));
        break;
    default:
        break;
    }
    endInsertRows();

    QModelIndex selectIndex = createIndex(rowCount, 0, newItem);
    qDebug() << __FUNCTION__ << selectIndex;

    file->setFileIndex(selectIndex);
    emit select(selectIndex);
    updateFile(selectIndex);
}

void FileModel::updateFile(const QModelIndex& fileIndex)
{
    int id = fileIndex.data(Qt::UserRole).toInt();
    if (!App::project()->file(id))
        throw QString("App::project()->file(id) is null: id %1!").arg(id);
    if (fileIndex.isValid() && id > -1) {
        switch (App::project()->file(id)->type()) {
        case FileType::Gerber:
            break;
        case FileType::Excellon:
            break;
        case FileType::GCode:
            break;
        case FileType::Dxf: {
            const QModelIndex index = createIndex(0, 0, fileIndex.internalId());
            // clean before insert new layers
            if (int count = getItem(fileIndex)->childCount(); count) {
                beginRemoveRows(index, 0, count - 1);
                auto item = getItem(index);
                do {
                    item->remove(--count);
                } while (count);
                endRemoveRows();
            }
            Dxf::Layers layers;
            for (auto& [name, layer] : reinterpret_cast<Dxf::File*>(App::project()->file(id))->layers()) {
                if (!layer->isEmpty())
                    layers[name] = layer;
            }
            beginInsertRows(index, 0, int(layers.size() - 1));
            for (auto& [name, layer] : layers) {
                getItem(index)->append(new Dxf::NodeLayer(name, layer));
            }
            endInsertRows();
        } break;
        default:
            break;
        }
    }
}

void FileModel::addShape(Shapes::Shape* sh)
{
    if (sh == nullptr)
        return;

    AbstractNode* item(rootItem->child(static_cast<int>(FileModel::Shapes)));
    QModelIndex index = createIndex(0, 0, item);
    int rowCount = item->childCount();

    beginInsertRows(index, rowCount, rowCount);
    auto node = new Shapes::Node(sh->id());
    item->append(node);
    endInsertRows();

    emit select(createIndex(rowCount, 0, node));
}

void FileModel::closeProject()
{
    AbstractNode* item;
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

QModelIndex FileModel::index(int row, int column, const QModelIndex& parent) const
{
    AbstractNode* childItem = getItem(parent)->child(row);
    if (childItem)
        return createIndex(row, column, childItem);
    return QModelIndex();
}

QModelIndex FileModel::parent(const QModelIndex& index) const
{
    if (!index.isValid())
        return QModelIndex();
    AbstractNode* parentItem = getItem(index)->parentItem();
    if (parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

QVariant FileModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    AbstractNode* item = getItem(index);

    return item->data(index, role);
}

bool FileModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool ok = getItem(index)->setData(index, value, role);
    if (ok)
        App::project()->setChanged();
    return ok;
}

QVariant FileModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal)
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

Qt::ItemFlags FileModel::flags(const QModelIndex& index) const
{
    if (!index.isValid())
        return Qt::NoItemFlags;
    return getItem(index)->flags(index);
}

bool FileModel::removeRows(int row, int count, const QModelIndex& parent)
{
    AbstractNode* item = nullptr;
    if (parent.isValid())
        item = static_cast<AbstractNode*>(parent.internalPointer());
    else
        return false;

    beginRemoveRows(parent, row, row + count - 1);
    while (count--)
        item->remove(row);
    endRemoveRows();
    resetInternalData();
    return true;
}

int FileModel::columnCount(const QModelIndex& /*parent*/) const
{
    return int(AbstractNode::Column::Count);
}

int FileModel::rowCount(const QModelIndex& parent) const
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
//    //                encodedData.append(tr("%1,%2").arg(index.row()).arg((quint64)item /*index.internalPointer()*/).toLocal8Bit());
//    //                encodedData.append("|");
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
//                encodedData.append(QString().setNum((quint64)index.internalPointer()).toLocal8Bit());
//                encodedData.append("|");
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
//    //        AbstractNode* copyItem = reinterpret_cast<AbstractNode*>(item.toLongLong());
//    //        AbstractNode* parentItem = static_cast<AbstractNode*>(parent.internalPointer());
//    //        if (copyItem) {
//    //            if (!parentItem)
//    //                parentItem = rootItem;
//    //            insertRows(beginRow, list.size(), parent);
//    //            if (parentItem->childCount() > beginRow)
//    //                parentItem->setChild(beginRow, new AbstractNode(*copyItem));
//    //            else
//    //                parentItem->setChild(parentItem->childCount() - 1, new AbstractNode(*copyItem));
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
//    //    AbstractNode* srcItem = static_cast<AbstractNode*>(sourceParent.internalPointer());
//    //    AbstractNode* dstItem = static_cast<AbstractNode*>(destinationParent.internalPointer());
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

AbstractNode* FileModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        auto* item = static_cast<AbstractNode*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

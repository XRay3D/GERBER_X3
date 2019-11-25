#include "filemodel.h"
#include "drillnode.h"
#include "foldernode.h"
#include "gcodenode.h"
#include "gerbernode.h"
#include <QDebug>
#include <QFile>
#include <QMimeData>

FileModel* FileModel::m_self = nullptr;

FileModel::FileModel(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new FolderNode("rootItem"))
    , mimeType(QStringLiteral("application/GCodeItem"))
{
    m_self = this;
    rootItem->append(new FolderNode(tr("Gerber Files")));
    rootItem->append(new FolderNode(tr("Excellon")));
    rootItem->append(new FolderNode(tr("Tool Paths")));
    rootItem->append(new FolderNode(tr("Special")));
}

FileModel::~FileModel()
{
    delete rootItem;
    m_self = nullptr;
}

void FileModel::addFile(AbstractFile* file)
{
    if (!m_self || !file)
        return;

    AbstractNode* item(m_self->rootItem->child(static_cast<int>(file->type())));
    QModelIndex index = m_self->createIndex(0, 0, item);
    int rowCount = item->childCount();

    m_self->beginInsertRows(index, rowCount, rowCount);
    switch (file->type()) {
    case FileType ::Gerber:
        item->append(new GerberNode(file->id()));
        break;
    case FileType ::Drill:
        item->append(new DrillNode(file->id()));
        break;
    case FileType::GCode:
        item->append(new GcodeNode(file->id()));
        break;
    default:
        break;
    }
    m_self->endInsertRows();

    QModelIndex selectIndex = m_self->createIndex(rowCount, 0, item->child(rowCount));
    emit m_self->select(selectIndex);
}

void FileModel::closeProject()
{
    if (m_self) {
        AbstractNode* item;
        for (int i = 0; i < m_self->rootItem->childCount(); ++i) {
            item = m_self->rootItem->child(i);
            QModelIndex index = m_self->createIndex(i, 0, item);
            int rowCount = item->childCount();
            if (rowCount) {
                m_self->beginRemoveRows(index, 0, rowCount - 1);
                for (int i = 0; i < rowCount; ++i)
                    item->remove(0);
                m_self->endRemoveRows();
            }
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
    return getItem(index)->setData(index, value, role);
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
            return tr("C");
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
    return 2;
}

int FileModel::rowCount(const QModelIndex& parent) const
{
    if (parent.column() > 0)
        return 0;
    return getItem(parent)->childCount();
}

FileModel* FileModel::self() { return m_self; }

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

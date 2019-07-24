#include "filemodel.h"
#include "drillnode.h"
#include "foldernode.h"
#include "gcodenode.h"
#include "gerbernode.h"
#include <QDebug>
#include <QFile>

FileModel* FileModel::m_self = nullptr;

FileModel::FileModel(QObject* parent)
    : QAbstractItemModel(parent)
    , rootItem(new FolderNode("rootItem"))
{
    m_self = this;
    rootItem->append(new FolderNode(tr("Gerber Files")));
    rootItem->append(new FolderNode(tr("Excellon")));
    rootItem->append(new FolderNode(tr("Tool Paths")));
    rootItem->append(new FolderNode(tr("Special")));
}

//FileModel::FileModel(Gerber::File* file)
//{
//    if (self && file) {
//        AbstractNode* item = rootItem->child(NodeGerberFiles);
//        QModelIndex index = createIndex(0, 0, rootItem);
//        int rowCount = item->childCount();
//        beginInsertRows(index, rowCount, rowCount);
//        item->append(new GerberNode(file));
//        endInsertRows();
//    }
//}
//FileModel::FileModel(Excellon::File* file)
//{
//    if (self && file) {
//        AbstractNode* item{ rootItem->child(NodeDrillFiles) };
//        QModelIndex index = createIndex(0, 0, item);
//        int rowCount = item->childCount();
//        beginInsertRows(index, rowCount, rowCount);
//        item->append(new DrillNode(file));
//        endInsertRows();
//    }
//}
//FileModel::FileModel(GCodeFile* file)
//{
//    if (self && file) {
//        AbstractNode* item{ rootItem->child(NodeToolPath) };
//        QModelIndex index = createIndex(0, 0, item);
//        int rowCount = item->childCount();
//        beginInsertRows(index, rowCount, rowCount);
//        item->append(new GcodeNode(file));
//        endInsertRows();
//    }
//}

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
    //    if (!index.isValid())
    //        return QVariant();

    //    AbstractNode* item = getItem(index);

    //    return item->data(index, role);
}

bool FileModel::setData(const QModelIndex& index, const QVariant& value, int role)
{
    bool result = getItem(index)->setData(index, value, role);

    //    if (result)
    //        emit dataChanged(index, index);

    return result;
    //    if (!index.isValid())
    //        return false;
    //    AbstractNode* item = static_cast<AbstractNode*>(index.internalPointer());
    //    return item->setData(index, value, role);
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

    //    AbstractNode* parentItem;

    //    if (!parent.isValid())
    //        parentItem = rootItem;
    //    else
    //        parentItem = static_cast<AbstractNode*>(parent.internalPointer());

    //    return parentItem->childCount();
}

FileModel* FileModel::self() { return m_self; }

AbstractNode* FileModel::getItem(const QModelIndex& index) const
{
    if (index.isValid()) {
        AbstractNode* item = static_cast<AbstractNode*>(index.internalPointer());
        if (item)
            return item;
    }
    return rootItem;
}

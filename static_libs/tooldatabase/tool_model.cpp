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
#include "tool_model.h"
#include "app.h"
#include "tool_item.h"

#include <QApplication>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMimeData>

ToolModel::ToolModel(QObject* parent)
    : QAbstractItemModel{parent}
    , rootItem(new ToolItem())
    , mimeType(u"application/ToolItem"_s) {
    loadTools();
}

ToolModel::~ToolModel() {
    saveTools();
    rootItem->setDeleteEnable(false);
    delete rootItem;
}

bool ToolModel::insertRows(int row, int count, const QModelIndex& parent) {

    //    return false;
    beginInsertRows(parent, row, row + count - 1);
    ToolItem* parentItem = static_cast<ToolItem*>(parent.internalPointer());
    if(!parentItem)
        parentItem = rootItem;

    if(parentItem->childCount() > row)
        parentItem->insertChild(row, new ToolItem());
    else
        parentItem->addChild(new ToolItem());

    endInsertRows();
    return true;
}

bool ToolModel::removeRows(int row, int count, const QModelIndex& parent) {

    //    return false;
    beginRemoveRows(parent, row, row + count - 1);
    ToolItem* parentItem = static_cast<ToolItem*>(parent.internalPointer());
    if(!parentItem)
        parentItem = rootItem;
    parentItem->removeChild(row);
    endRemoveRows();
    return true;
}

bool ToolModel::moveRows(const QModelIndex& sourceParent, int sourceRow, int count, const QModelIndex& destinationParent, int destinationChild) {

    qDebug() << "sourceParent" << sourceParent;
    qDebug() << "sourceRow" << sourceRow;
    qDebug() << "count" << count;
    qDebug() << "destinationParen" << destinationParent;
    qDebug() << "destinationChild" << destinationChild;

    return false;
    //     beginMoveRows(sourceParent, sourceRow, sourceRow + count - 1, destinationParent, destinationChild);
    //     ToolItem* srcItem = static_cast<ToolItem*>(sourceParent.internalPointer());
    //     ToolItem* dstItem = static_cast<ToolItem*>(destinationParent.internalPointer());
    //     if (!srcItem)
    //         srcItem = rootItem;
    //     if (!dstItem)
    //         dstItem = rootItem;
    //     for (int r = 0; r < count; ++r) {
    //         dstItem->insertChild(destinationChild + r, srcItem->takeChild(sourceRow));
    //     }
    //     endMoveRows();
    //     return true;
}

int ToolModel::columnCount(const QModelIndex& /*parent*/) const { return 3; }

int ToolModel::rowCount(const QModelIndex& parent) const {
    ToolItem* parentItem;
    if(parent.column() > 0)
        return 0;

    if(!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ToolItem*>(parent.internalPointer());

    return parentItem->childCount();
}

QModelIndex ToolModel::index(int row, int column, const QModelIndex& parent) const {
    if(!hasIndex(row, column, parent))
        return QModelIndex();

    ToolItem* parentItem;

    if(!parent.isValid())
        parentItem = rootItem;
    else
        parentItem = static_cast<ToolItem*>(parent.internalPointer());

    ToolItem* childItem = parentItem->child(row);
    if(childItem)
        return createIndex(row, column, childItem);
    else
        return QModelIndex();
}

QModelIndex ToolModel::parent(const QModelIndex& child) const {
    if(!child.isValid())
        return QModelIndex();
    ToolItem* parentItem = static_cast<ToolItem*>(child.internalPointer())->parent();
    if(parentItem == rootItem)
        return QModelIndex();
    return createIndex(parentItem->row(), 0, parentItem);
}

Qt::ItemFlags ToolModel::flags(const QModelIndex& index) const {
    if(!index.isValid())
        return rootItem->flags(index);
    return getItem(index)->flags(index);
}

bool ToolModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    return getItem(index)->setData(index, value, role);
}

QVariant ToolModel::data(const QModelIndex& index, int role) const {
    if(!index.isValid())
        return {};
    return getItem(index)->data(index, role);
}

QVariant ToolModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if(role == Qt::DisplayRole && orientation == Qt::Horizontal)
        return tr("Name|Note|Id").split('|')[section];
    if(role == Qt::TextAlignmentRole && orientation == Qt::Horizontal)
        return Qt::AlignHCenter;
    return {};
}

QStringList ToolModel::mimeTypes() const {
    QStringList types;
    types << mimeType;
    return types;
}

QMimeData* ToolModel::mimeData(const QModelIndexList& indexes) const {
    QMimeData* mimeData = new QMimeData();
    QByteArray encodedData;
    int noCopy = -1;
    for(const QModelIndex& index: indexes) {
        if(noCopy != index.row()) {
            noCopy = index.row();
            if(index.isValid()) {
                encodedData.push_back(QString().setNum((quint64)index.internalPointer()).toUtf8());
                encodedData.push_back("|");
            }
        }
    }
    mimeData->setData(mimeType, encodedData);
    return mimeData;
}

bool ToolModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int row, int column, const QModelIndex& parent) {
    if(action == Qt::IgnoreAction)
        return true;

    if(!data->hasFormat(mimeType))
        return false;

    if(column > 0)
        return false;

    int beginRow;

    if(row != -1)
        beginRow = row;
    else if(parent.isValid())
        beginRow = parent.row();
    else
        beginRow = rowCount(QModelIndex());

    QString encodedData = data->data(mimeType);
    QList<QString> list = encodedData.split('|', Qt::SkipEmptyParts);
    for(QString& item: list) {
        ToolItem* copyItem = reinterpret_cast<ToolItem*>(item.toLongLong());
        ToolItem* parentItem = static_cast<ToolItem*>(parent.internalPointer());
        if(copyItem) {
            if(!parentItem)
                parentItem = rootItem;
            insertRows(beginRow, list.size(), parent);
            if(parentItem->childCount() > beginRow)
                parentItem->setChild(beginRow, new ToolItem{*copyItem});
            else
                parentItem->setChild(parentItem->childCount() - 1, new ToolItem{*copyItem});
        }
        ++beginRow;
    }
    return true;
}

Qt::DropActions ToolModel::supportedDragActions() const { return Qt::MoveAction | Qt::TargetMoveAction; }

Qt::DropActions ToolModel::supportedDropActions() const { return Qt::MoveAction | Qt::TargetMoveAction; }

void ToolModel::saveTools() {
    QFile file(App::settingsPath() + u"/tools.json"_s);
    if(!file.open(QIODevice::WriteOnly)) {
        qDebug() << __FUNCTION__ << file.errorString();
        return;
    }

    QJsonObject jsonObject;
    App::toolHolder().writeTools(jsonObject);

    ToolItem* item;
    QList<ToolItem*> stack;
    QList<int> row;

    QJsonArray treeArray;

    stack.push_back(rootItem);
    row.push_back(0);

    while(stack.size()) {
        if(stack.last()->childCount() && row.last()) {
            stack.pop_back();
            row.pop_back();
            if(!stack.size())
                break;
            ++row.last();
        } else if(stack.last() == rootItem && stack.last()->childCount() == 0) {
            break;
        }
        while(stack.last()->childCount() > row.last()) {
            item = stack.last()->child(row.last());
            QJsonObject treeNode;
            if(item->isTool()) {
                treeNode["id"] = item->toolId();
            } else {
                treeNode["name"] = item->name();
                treeNode["note"] = item->note();
            }
            treeNode["tool"] = item->isTool();
            treeNode["tab"] = row.size() - 1;
            treeArray.push_back(treeNode);
            if(item->childCount()) {
                stack.push_back(item);
                row.push_back(0);
                break;
            }
            ++row.last();
        }
    }
    jsonObject["tree"] = QJsonValue{treeArray};
    QJsonDocument saveDoc(jsonObject);
    file.write(saveDoc.toJson());
}

void ToolModel::loadTools() {
    QJsonDocument loadDoc;

    QFile file(App::settingsPath() + u"/tools.json"_s);

    if(!file.exists())
        file.setFileName(qApp->applicationDirPath() + "/tools.json");
    if(file.exists() && file.open(QIODevice::ReadOnly))
        loadDoc = QJsonDocument::fromJson(file.readAll());
    else {
        qDebug() << __FUNCTION__ << file.errorString();
        return;
    }

    App::toolHolder().readTools(loadDoc.object());

    QList<ToolItem*> parentsStack;
    QList<int> nestingStack;
    parentsStack << rootItem;
    nestingStack << 0;

    QJsonArray treeArray = loadDoc.object()["tree"].toArray();
    for(int treelIndex = 0; treelIndex < treeArray.size(); ++treelIndex) {

        QJsonObject json = treeArray[treelIndex].toObject();
        int nesting = json["tab"].toInt();

        if(nesting > nestingStack.last()) {
            // The last child of the current parent is now the new parent unless the current parent has no children.
            if(parentsStack.last()->childCount() > 0) {
                parentsStack.push_back(parentsStack.last()->child(parentsStack.last()->childCount() - 1));
                nestingStack.push_back(nesting);
            }
        } else {
            while(nesting < nestingStack.last() && parentsStack.count() > 0) {
                parentsStack.pop_back();
                nestingStack.pop_back();
            }
        }

        // Append a new item to the current parent's list of children.
        ToolItem* parent = parentsStack.last();
        ToolItem* item;
        if(json["tool"].toBool())
            item = new ToolItem{json["id"].toInt()};
        else {
            item = new ToolItem();
            item->setName(json["name"].toString());
            item->setNote(json["note"].toString());
        }
        parent->insertChild(parent->childCount(), item);
    }
}

#include "moc_tool_model.cpp"

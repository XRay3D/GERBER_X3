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
#include "toolitem.h"
#include <QIcon>
#include <QModelIndex>
#include <QVariant>

#include "leakdetector.h"

bool ToolItem::m_deleteEnable = false;

ToolItem::ToolItem(const ToolItem& item)
{
    if (item.m_toolId) {
        m_toolId = item.m_toolId;
        item.m_toolId = 0;
    } else {
        m_name = item.m_name;
        m_note = item.m_note;
    }
    for (ToolItem* i : item.childItems) {
        addChild(new ToolItem(*i));
    }
}

ToolItem::ToolItem(int toolId)
    : m_toolId(toolId)
{
}

ToolItem::ToolItem()
{
}

ToolItem::~ToolItem()
{
    if (m_toolId && m_deleteEnable)
        ToolHolder::m_tools.remove(m_toolId);
    qDeleteAll(childItems);
}

//void ToolItem::read(const QJsonObject& json)
//{
//    QJsonArray toolArray = json["tools"].toArray();
//    for (int treeIndex = 0; treeIndex < toolArray.size(); ++treeIndex) {
//        Tool tool;
//        QJsonObject toolObject = toolArray[treeIndex].toObject();
//        tool.read(toolObject);
//        tool.id = toolObject["id"].toInt();
//        ToolHolder::tools[tool.id] = tool;
//    }
//}

//void ToolItem::write(QJsonObject& json)
//{
//    QJsonArray toolArray;
//    QMap<int, Tool>::iterator i = ToolHolder::tools.begin();
//    while (i != ToolHolder::tools.constEnd()) {
//        QJsonObject toolObject;
//        i.value().write(toolObject);
//        toolObject["id"] = i.key();
//        toolArray.append(toolObject);
//        ++i;
//    }
//    json["tools"] = toolArray;
//}

int ToolItem::row() const
{
    if (parentItem != nullptr)
        return parentItem->childItems.indexOf(const_cast<ToolItem*>(this));
    return 0;
}

int ToolItem::childCount() const
{
    return childItems.size();
}

ToolItem* ToolItem::child(int row) const
{
    return childItems.at(row);
}

ToolItem* ToolItem::lastChild() const
{
    if (childItems.size())
        return childItems.last();
    return nullptr;
}

ToolItem* ToolItem::takeChild(int row)
{
    return childItems.takeAt(row);
}

void ToolItem::setChild(int row, ToolItem* item)
{
    if (item)
        item->parentItem = this;

    if (row < childItems.size()) {
        delete childItems[row];
        childItems[row] = item;
    }
}

bool ToolItem::setData(const QModelIndex& index, const QVariant& value, int role)
{
    if (index.isValid()) {
        switch (role) {
        case Qt::EditRole:
            switch (index.column()) {
            case 0:
                setName(value.toString());
                return true;
            case 1:
                setNote(value.toString());
                return true;
            default:
                return false;
            }
        default:
            return false;
        }
    }
    return false;
}

QVariant ToolItem::data(const QModelIndex& index, int role) const
{
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return name();
        case 1:
            return note();
        case 2:
            return m_toolId ? QVariant(m_toolId) : QVariant();
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (m_toolId)
                return ToolHolder::tool(m_toolId).icon();
            else
                return QIcon::fromTheme("folder-sync");
        }
        return QVariant();
    case Qt::UserRole:
        return m_toolId;
    case Qt::UserRole + 1:
        return childCount();
    case Qt::TextAlignmentRole:
        if (index.column() == 2)
            return Qt::AlignCenter;
        return QVariant();
    default:
        return QVariant();
    }
}

Qt::ItemFlags ToolItem::flags(const QModelIndex& /*index*/) const
{
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    if (!m_toolId)
        flags |= Qt::ItemIsDropEnabled;
    else
        flags |= Qt::ItemNeverHasChildren;
    return flags;
}

int ToolItem::toolId() const
{
    return m_toolId;
}

Tool& ToolItem::tool()
{
    if (ToolHolder::tools().contains(m_toolId))
        return ToolHolder::m_tools[m_toolId];
    static Tool tmp;
    return tmp;
}

bool ToolItem::isTool() const
{
    return m_toolId;
}

void ToolItem::setIsTool()
{
    if (ToolHolder::m_tools.size())
        m_toolId = ToolHolder::m_tools.lastKey() + 1;
    else
        m_toolId = 1;
    ToolHolder::m_tools[m_toolId].setId(m_toolId);
}

QString ToolItem::note() const
{
    return m_toolId ? ToolHolder::m_tools[m_toolId].note() : m_note;
}

void ToolItem::setNote(const QString& value)
{
    if (m_toolId)
        ToolHolder::m_tools[m_toolId].setNote(value);
    else
        m_note = value;
}

void ToolItem::setDeleteEnable(bool deleteEnable)
{
    m_deleteEnable = deleteEnable;
}

QString ToolItem::name() const
{
    return m_toolId ? ToolHolder::tool(m_toolId).name() : m_name;
}

void ToolItem::setName(const QString& value)
{
    if (m_toolId)
        ToolHolder::m_tools[m_toolId].setName(value);
    else
        m_name = value;
}

void ToolItem::addChild(ToolItem* item)
{
    if (item)
        item->parentItem = this;
    childItems.append(item);
}

void ToolItem::insertChild(int row, ToolItem* item)
{
    if (item)
        item->parentItem = this;
    if (row < childItems.size())
        childItems.insert(row, item);
    else if (row == childItems.size())
        childItems.append(item);
}

void ToolItem::removeChild(int row)
{
    m_deleteEnable = true;
    delete childItems.at(row);
    childItems.removeAt(row);
    m_deleteEnable = false;
}

ToolItem* ToolItem::parent() { return parentItem; }

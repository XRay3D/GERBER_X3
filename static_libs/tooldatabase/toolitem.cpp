// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "toolitem.h"

#include "app.h"

#include <QIcon>
#include <QModelIndex>
#include <QVariant>

#include "leakdetector.h"

ToolItem::ToolItem(const ToolItem& item)
{
    if (item.m_toolId > 0) {
        m_toolId = item.m_toolId;
        item.m_toolId = 0;
    } else {
        m_name = item.m_name;
        m_note = item.m_note;
    }
    for (ToolItem* i : item.childItems)
        addChild(new ToolItem(*i));
}

ToolItem::ToolItem(int toolId)
    : m_toolId(toolId)
{
}

ToolItem::ToolItem() { }

ToolItem::~ToolItem()
{
    if (m_toolId && m_deleteEnable)
        App::toolHolder().m_tools.erase(m_toolId);
    qDeleteAll(childItems);
}

int ToolItem::row() const
{
    if (parentItem != nullptr)
        return parentItem->childItems.indexOf(const_cast<ToolItem*>(this));
    return 0;
}

int ToolItem::childCount() const { return childItems.size(); }

ToolItem* ToolItem::child(int row) const { return childItems.at(row); }

ToolItem* ToolItem::lastChild() const
{
    if (childItems.size())
        return childItems.last();
    return nullptr;
}

ToolItem* ToolItem::takeChild(int row) { return childItems.takeAt(row); }

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
                return App::toolHolder().tool(m_toolId).icon();
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

int ToolItem::toolId() const { return m_toolId; }

Tool& ToolItem::tool()
{
    if (App::toolHolder().tools().contains(m_toolId))
        return App::toolHolder().m_tools[m_toolId];
    static Tool tmp;
    return tmp;
}

bool ToolItem::isTool() const { return m_toolId > 0; }

void ToolItem::setIsTool()
{
    if (App::toolHolder().m_tools.size())
        m_toolId = App::toolHolder().m_tools.begin()->first + 1;
    else
        m_toolId = 1;
    App::toolHolder().m_tools[m_toolId].setId(m_toolId);
}

QString ToolItem::note() const
{
    return m_toolId ? App::toolHolder().m_tools[m_toolId].note() : m_note;
}

void ToolItem::setNote(const QString& value)
{
    if (m_toolId)
        App::toolHolder().m_tools[m_toolId].setNote(value);
    else
        m_note = value;
}

void ToolItem::setDeleteEnable(bool deleteEnable) { m_deleteEnable = deleteEnable; }

QString ToolItem::name() const { return m_toolId ? App::toolHolder().tool(m_toolId).name() : m_name; }

void ToolItem::setName(const QString& value)
{
    if (m_toolId)
        App::toolHolder().m_tools[m_toolId].setName(value);
    else
        m_name = value;
}

void ToolItem::addChild(ToolItem* item)
{
    if (item)
        item->parentItem = this;
    childItems.push_back(item);
}

void ToolItem::insertChild(int row, ToolItem* item)
{
    if (item)
        item->parentItem = this;
    if (row < childItems.size())
        childItems.insert(row, item);
    else if (row == childItems.size())
        childItems.push_back(item);
}

void ToolItem::removeChild(int row)
{
    m_deleteEnable = true;
    delete childItems.at(row);
    childItems.removeAt(row);
    m_deleteEnable = false;
}

ToolItem* ToolItem::parent() { return parentItem; }

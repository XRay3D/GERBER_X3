// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "tool_item.h"

#include "app.h"

#include <QIcon>
#include <QModelIndex>
#include <QVariant>

ToolItem::ToolItem(const ToolItem& item) {
    if (item.toolId_ > 0) {
        toolId_ = item.toolId_;
        item.toolId_ = 0;
    } else {
        name_ = item.name_;
        note_ = item.note_;
    }
    for (ToolItem* i : item.childItems)
        addChild(new ToolItem(*i));
}

ToolItem::ToolItem(int toolId)
    : toolId_(toolId) {
}

ToolItem::ToolItem() { }

ToolItem::~ToolItem() {
    if (toolId_ && deleteEnable_)
        App::toolHolder().tools_.erase(toolId_);
    qDeleteAll(childItems);
}

int ToolItem::row() const {
    if (parentItem != nullptr)
        return parentItem->childItems.indexOf(const_cast<ToolItem*>(this));
    return 0;
}

int ToolItem::childCount() const { return childItems.size(); }

ToolItem* ToolItem::child(int row) const { return childItems.at(row); }

ToolItem* ToolItem::lastChild() const {
    if (childItems.size())
        return childItems.last();
    return nullptr;
}

ToolItem* ToolItem::takeChild(int row) { return childItems.takeAt(row); }

void ToolItem::setChild(int row, ToolItem* item) {
    if (item)
        item->parentItem = this;

    if (row < childItems.size()) {
        delete childItems[row];
        childItems[row] = item;
    }
}

bool ToolItem::setData(const QModelIndex& index, const QVariant& value, int role) {
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

QVariant ToolItem::data(const QModelIndex& index, int role) const {
    switch (role) {
    case Qt::DisplayRole:
        switch (index.column()) {
        case 0:
            return name();
        case 1:
            return note();
        case 2:
            return toolId_ ? QVariant(toolId_) : QVariant();
        default:
            return QVariant();
        }
    case Qt::DecorationRole:
        if (index.column() == 0) {
            if (toolId_)
                return App::toolHolder().tool(toolId_).icon();
            else
                return QIcon::fromTheme("folder-sync");
        }
        return QVariant();
    case Qt::UserRole:
        return toolId_;
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

Qt::ItemFlags ToolItem::flags(const QModelIndex& /*index*/) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    if (!toolId_)
        flags |= Qt::ItemIsDropEnabled;
    else
        flags |= Qt::ItemNeverHasChildren;
    return flags;
}

int ToolItem::toolId() const { return toolId_; }

Tool& ToolItem::tool() {
    if (App::toolHolder().tools().contains(toolId_))
        return App::toolHolder().tools_[toolId_];
    static Tool tmp;
    return tmp;
}

bool ToolItem::isTool() const { return toolId_ > 0; }

void ToolItem::setIsTool() {
    if (App::toolHolder().tools_.size())
        toolId_ = App::toolHolder().tools_.begin()->first + 1;
    else
        toolId_ = 1;
    App::toolHolder().tools_[toolId_].setId(toolId_);
}

QString ToolItem::note() const {
    return toolId_ ? App::toolHolder().tools_[toolId_].note() : note_;
}

void ToolItem::setNote(const QString& value) {
    if (toolId_)
        App::toolHolder().tools_[toolId_].setNote(value);
    else
        note_ = value;
}

void ToolItem::setDeleteEnable(bool deleteEnable) { deleteEnable_ = deleteEnable; }

QString ToolItem::name() const { return toolId_ ? App::toolHolder().tool(toolId_).name() : name_; }

void ToolItem::setName(const QString& value) {
    if (toolId_)
        App::toolHolder().tools_[toolId_].setName(value);
    else
        name_ = value;
}

void ToolItem::addChild(ToolItem* item) {
    if (item)
        item->parentItem = this;
    childItems.push_back(item);
}

void ToolItem::insertChild(int row, ToolItem* item) {
    if (item)
        item->parentItem = this;
    if (row < childItems.size())
        childItems.insert(row, item);
    else if (row == childItems.size())
        childItems.push_back(item);
}

void ToolItem::removeChild(int row) {
    deleteEnable_ = true;
    delete childItems.at(row);
    childItems.removeAt(row);
    deleteEnable_ = false;
}

ToolItem* ToolItem::parent() { return parentItem; }

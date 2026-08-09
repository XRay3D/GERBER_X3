/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "tool_item.h"

#include "app.h"

#include <QIcon>
#include <QModelIndex>
#include <QVariant>

ToolItem::ToolItem(const ToolItem& item) {
    if(item.id > Tool::ID{}) {
        id = item.id;
        item.id = Tool::ID::Folder;
    } else {
        name_ = item.name_;
        note_ = item.note_;
    }
    for(ToolItem* i: item.childItems)
        addChild(new ToolItem{*i});
}

ToolItem::ToolItem(Tool::ID toolId)
    // : id{toolId == Tool::ID{} ? Tool::ID::Folder : toolId} {
    : id{toolId} {
}

ToolItem::~ToolItem() {
    if(+id && deleteEnable_)
        App::toolHolder().tools_.erase(id);
    qDeleteAll(childItems);
}

qsizetype ToolItem::row() const {
    if(parentItem != nullptr)
        return parentItem->childItems.indexOf(const_cast<ToolItem*>(this));
    return 0;
}

qsizetype ToolItem::childCount() const { return childItems.size(); }

ToolItem* ToolItem::child(int row) const { return childItems.at(row); }

ToolItem* ToolItem::lastChild() const {
    if(childItems.size())
        return childItems.last();
    return nullptr;
}

ToolItem* ToolItem::takeChild(int row) { return childItems.takeAt(row); }

void ToolItem::setChild(int row, ToolItem* item) {
    if(item)
        item->parentItem = this;

    if(row < childItems.size()) {
        delete childItems[row];
        childItems[row] = item;
    }
}

bool ToolItem::setData(const QModelIndex& index, const QVariant& value, int role) {
    if(index.isValid()) {
        switch(role) {
        case Qt::EditRole:
            switch(index.column()) {
            case 0:
                setName(value.toString());
                return true;
            case 1:
                setNote(value.toString());
                return true;
            default: return false;
            }
        default: return false;
        }
    }
    return false;
}

QVariant ToolItem::data(const QModelIndex& index, int role) const {
    switch(role) {
    case Qt::DisplayRole:
        switch(index.column()) {
        case 0 : return name();
        case 1 : return note();
        case 2 : return +id ? QVariant{+id} : QVariant{};
        default: return {};
        }
    case Qt::DecorationRole:
        if(index.column() == 0) {
            if(+id)
                return App::toolHolder().tool(id).icon();
            else
                return QIcon::fromTheme(u"folder-sync"_s);
        }
        return {};
    case Qt::UserRole: return +id;
    case Qt::UserRole + 1:
        return childCount();
    case Qt::TextAlignmentRole:
        if(index.column() == 2)
            return Qt::AlignCenter;
        return {};
    default: return {};
    }
}

Qt::ItemFlags ToolItem::flags(const QModelIndex& /*index*/) const {
    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsDragEnabled | Qt::ItemIsSelectable;
    if(id == Tool::ID::Folder)
        flags |= Qt::ItemIsDropEnabled;
    else
        flags |= Qt::ItemNeverHasChildren;
    return flags;
}

Tool::ID ToolItem::toolId() const { return id; }

Tool& ToolItem::tool() {
    if(App::toolHolder().tools().contains(id))
        return App::toolHolder().tools_[id];
    static Tool tmp;
    return tmp;
}

bool ToolItem::isTool() const { return id > Tool::ID{}; }

void ToolItem::setIsTool() {
    if(App::toolHolder().tools_.size())
        id = App::toolHolder().tools_.begin()->first + Tool::ID::Tool;
    else
        id = Tool::ID::Tool;
    App::toolHolder().tools_[id].setId(id);
}

QString ToolItem::note() const {
    return +id ? App::toolHolder().tools_[id].note() : note_;
}

void ToolItem::setNote(const QString& value) {
    if(+id)
        App::toolHolder().tools_[id].setNote(value);
    else
        note_ = value;
}

void ToolItem::setDeleteEnable(bool deleteEnable) { deleteEnable_ = deleteEnable; }

QString ToolItem::name() const { return +id ? App::toolHolder().tool(id).name() : name_; }

void ToolItem::setName(const QString& value) {
    if(+id)
        App::toolHolder().tools_[id].setName(value);
    else
        name_ = value;
}

void ToolItem::addChild(ToolItem* item) {
    if(item)
        item->parentItem = this;
    childItems.push_back(item);
}

void ToolItem::insertChild(int row, ToolItem* item) {
    if(item)
        item->parentItem = this;
    if(row < childItems.size())
        childItems.insert(row, item);
    else if(row == childItems.size())
        childItems.push_back(item);
}

void ToolItem::removeChild(int row) {
    deleteEnable_ = true;
    delete childItems.at(row);
    childItems.removeAt(row);
    deleteEnable_ = false;
}

ToolItem* ToolItem::parent() { return parentItem; }

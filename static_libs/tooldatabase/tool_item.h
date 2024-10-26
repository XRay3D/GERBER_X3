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
#pragma once
#include "tool.h"

class ToolItem {

public:
    ToolItem() = default;
    ToolItem(int toolId);
    ToolItem(const ToolItem& item);
    ~ToolItem();

    int row() const;
    int childCount() const;
    ToolItem* child(int row) const;
    ToolItem* lastChild() const;
    ToolItem* takeChild(int row);
    ToolItem* parent();

    void addChild(ToolItem* item);
    void insertChild(int row, ToolItem* item);
    void removeChild(int row);
    void setChild(int row, ToolItem* item);

    bool setData(const QModelIndex& index, const QVariant& value, int role);

    QVariant data(const QModelIndex& index, int role) const;

    Qt::ItemFlags flags(const QModelIndex&) const;

    int toolId() const;

    Tool& tool();

    bool isTool() const;
    void setIsTool();

    QString name() const;
    void setName(const QString& value);

    QString note() const;
    void setNote(const QString& value);

    static void setDeleteEnable(bool deleteEnable);

    ToolItem& operator=(const ToolItem&) = delete;

private:
    static inline bool deleteEnable_ = false;
    ToolItem* parentItem = nullptr;
    QList<ToolItem*> childItems;
    QString name_;
    QString note_;
    mutable int toolId_ = 0;
};

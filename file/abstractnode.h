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
#pragma once

#include "app.h"
#include "project.h"
#include <QAbstractItemModel>
#include <QSharedPointer>
#include <QVariant>

class AbstractFile;
class TreeView;
namespace Shapes {
class Shape;
}
class QMenu;

QPixmap decoration(QColor color, QChar chr = {});

class AbstractNode {
public:
    explicit AbstractNode(int id, int type = 0);
    virtual ~AbstractNode();

    AbstractNode* child(int row) const;
    AbstractNode* parentItem() const;

    void setChild(int row, AbstractNode* item);

    int childCount() const;
    int row() const;

    void append(AbstractNode* item);
    void remove(int row);

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual QVariant data(const QModelIndex& index, int role) const = 0;
    virtual void menu(QMenu* menu, TreeView* tv) const = 0;

    enum class Column {
        NameColorVisible,
        SideType,
        ItrmsType,
        Count
    };

    AbstractNode(const AbstractNode&) = delete;
    AbstractNode& operator=(const AbstractNode&) = delete;
    QModelIndex index() const;

protected:
    const int m_id;
    const int m_type;

    const QStringList sideStrList;
    AbstractNode* m_parentItem = nullptr;
    QList<QSharedPointer<AbstractNode>> childItems;
    inline AbstractFile* file() const { return App::project()->file(m_id); }
    inline Shapes::Shape* shape() const { return App::project()->aShape(m_id); }
};

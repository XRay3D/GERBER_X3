/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "project.h"
#include "filemodel.h"

#include <QAbstractItemModel>
#include <QApplication>
#include <QPainter>
#include <QSharedPointer>
#include <QStyle>
#include <QVariant>

class FileInterface;
//class FileModel;
class FileTreeView;
class QMenu;

namespace Shapes {
class Shape;
}

enum class NodeColumn : int {
    NameColorVisible,
    SideType,
    ItemsType,
    Count
};

enum class NodeR : int {
    IdRole = Qt::UserRole,
    SelectRole
};

inline QPixmap decoration(QColor color, QChar chr = {})
{
    //    qDebug() << __FUNCTION__ << QApplication::style()->pixelMetric(QStyle::PM_SmallIconSize);
    QPixmap pixmap(22, 22);
    pixmap.fill(Qt::transparent);
    QPainter p(&pixmap);
    color.setAlpha(255);
    p.setBrush(color);
    p.drawRect(2, 2, 18, 18);
    if (!chr.isNull()) {
        QFont f;
        f.setBold(true);
        f.setPixelSize(18);
        p.setFont(f);
        //p.setPen(Qt::white);
        p.drawText(QRect(2, 2, 18, 18), Qt::AlignCenter, { chr });
    }
    return pixmap;
}

class NodeInterface {
public:
    enum Role : int {
        IdRole = Qt::UserRole,
        SelectRole
    };
    explicit NodeInterface(const int& id, int type = 0)
        : m_id(id)
        , m_type(type)
        , sideStrList(QObject::tr("Top|Bottom").split('|'))
    {
    }
    virtual ~NodeInterface()
    {
        if (m_id > -1 && m_type > -1) {
            if (m_type) {
                App::project()->deleteShape(m_id);
            } else
                App::project()->deleteFile(m_id);
        }
        childItems.clear();
    }

    NodeInterface* child(int row) const { return childItems.value(row).data(); }
    NodeInterface* parentItem() const { return m_parentItem; }

    void setChild(int row, NodeInterface* item)
    {
        if (item)
            item->m_parentItem = this;
        if (row < childItems.size()) {
            childItems[row].reset(item);
        }
    }

    int childCount() const { return childItems.count(); }
    int row() const
    {
        if (m_parentItem)
            for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
                if (m_parentItem->childItems[i] == this)
                    return i;
        return 0;
    }

    void addNode(NodeInterface* item)
    {
        item->m_parentItem = this;
        childItems.push_back(QSharedPointer<NodeInterface>(item));
    }
    void remove(int row) { childItems.removeAt(row); }

    virtual bool setData(const QModelIndex& index, const QVariant& value, int role) = 0;
    virtual Qt::ItemFlags flags(const QModelIndex& index) const = 0;
    virtual QVariant data(const QModelIndex& index, int role) const = 0;
    virtual void menu(QMenu& menu, FileTreeView* tv) const = 0;

    enum class NodeColumn {
        NameColorVisible,
        SideType,
        ItemsType,
        Count
    };

    NodeInterface(const NodeInterface&) = delete;
    NodeInterface& operator=(const NodeInterface&) = delete;
    QModelIndex index(int column = 0) const { return App::fileModel()->createIndex(row(), column, reinterpret_cast<quintptr>(this)); }
    const QStringList sideStrList;

protected:
    const int& m_id;
    const int m_type;

    NodeInterface* m_parentItem = nullptr;
    QList<QSharedPointer<NodeInterface>> childItems;
    inline FileInterface* file() const { return App::project()->file(m_id); }
    inline Shapes::Shape* shape() const { return App::project()->shape(m_id); }
};

//#define NodeInterface_iid "ru.xray3d.XrSoft.GGEasy.NodeInterface"

//Q_DECLARE_INTERFACE(NodeInterface, NodeInterface_iid)

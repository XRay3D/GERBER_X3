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

#include "abstractnode.h"
#include "app.h"
#include "filetree/filemodel.h"
#include "mainwindow.h"
#include "project.h"

AbstractNode::AbstractNode(int id, int type)
    : m_id(id)
    , m_type(type)
    , sideStrList(QObject::tr("Top|Bottom").split('|'))
{
}

AbstractNode::~AbstractNode()
{
    if (m_id != -1) {
        if (m_type)
            App::project()->deleteShape(m_id);
        else
            App::project()->deleteFile(m_id);
    }
    childItems.clear();
}

int AbstractNode::row() const
{
    if (m_parentItem)
        for (int i = 0, size = m_parentItem->childItems.size(); i < size; ++i)
            if (m_parentItem->childItems[i] == this)
                return i;
    return 0;
}

AbstractNode* AbstractNode::child(int row) const { return childItems.value(row).data(); }

AbstractNode* AbstractNode::parentItem() const { return m_parentItem; }

void AbstractNode::setChild(int row, AbstractNode* item)
{
    if (item)
        item->m_parentItem = this;
    if (row < childItems.size()) {
        childItems[row].reset(item);
    }
}

void AbstractNode::append(AbstractNode* item)
{
    item->m_parentItem = this;
    childItems.append(QSharedPointer<AbstractNode>(item));
}

void AbstractNode::remove(int row) { childItems.removeAt(row); }

QModelIndex AbstractNode::index() const { return App::fileModel()->createIndex(row(), 0, reinterpret_cast<quintptr>(this)); }

int AbstractNode::childCount() const { return childItems.count(); }

QPixmap decoration(QColor color, QChar chr)
{
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

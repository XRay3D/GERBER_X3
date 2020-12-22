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
#include "itemgroup.h"
#include "scene.h"

ItemGroup::~ItemGroup()
{
    //qDebug("~ItemGroup()");
    if (App::scene()->items().size())
        qDeleteAll(*this);
}

void ItemGroup::append(GraphicsItem* item)
{
    item->m_id = QList::size() ? QList::last()->m_id + 1 : 0;
    item->setToolTip((item->toolTip().isEmpty() ? QString() : item->toolTip() + '\n') + QString("ID(%1): %2").arg(item->type()).arg(item->m_id));
    item->setVisible(m_visible);
    QList::append(item);
}

void ItemGroup::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        for (GraphicsItem* item : *this)
            item->setVisible(m_visible);
    }
}

void ItemGroup::setSelected(const QVector<int>& ids)
{
    for (GraphicsItem* item : *this)
        item->setSelected(ids.contains(item->id()));
}

void ItemGroup::addToScene()
{
    for (auto item : *this)
        App::scene()->addItem(item);
}

void ItemGroup::setBrushColor(const QColor& color)
{
    if (m_brushColor != color) {
        m_brushColor = color;
        for (GraphicsItem* item : *this)
            item->setColorP(&m_brushColor);
    }
}

void ItemGroup::setPen(const QPen& pen)
{
    if (m_pen != pen) {
        m_pen = pen;
        for (GraphicsItem* item : *this)
            item->setPen(m_pen);
    }
}

void ItemGroup::setBrushColorP(QColor* col)
{
    for (GraphicsItem* item : *this)
        item->setColorP(col);
}

void ItemGroup::setPenColor(QColor* col)
{
    for (GraphicsItem* item : *this)
        item->setPenColor(col);
}

void ItemGroup::setZValue(qreal z)
{
    for (GraphicsItem* item : *this)
        item->setZValue(z);
}

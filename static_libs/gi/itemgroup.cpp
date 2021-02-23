// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
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
    if (App::scene()->items().size())
        qDeleteAll(*this);
}

void ItemGroup::push_back(GraphicsItem* item)
{
    item->m_giId = mvector::size() ? mvector::back()->m_giId + 1 : 0;
    item->setToolTip((item->toolTip().isEmpty() ? QString() : item->toolTip() + '\n') + QString("ID(%1): %2").arg(item->type()).arg(item->m_giId));
    item->setVisible(m_visible);
    item->itemGroup = this;
    mvector::push_back(item);
}

void ItemGroup::setVisible(bool visible)
{
    if (m_visible != visible) {
        m_visible = visible;
        for (GraphicsItem* item : *this)
            item->setVisible(m_visible);
    }
}

void ItemGroup::setSelected(const mvector<int>& ids)
{
    for (GraphicsItem* item : *this)
        item->setSelected(ids.contains(item->id()));
}

void ItemGroup::addToScene(QGraphicsScene* scene)
{
    for (auto item : *this)
        (scene ? scene : App::scene())->addItem(item);
}

void ItemGroup::setBrushColor(const QColor& color)
{
    if (m_brushColor != color) {
        m_brushColor = color;
        for (GraphicsItem* item : *this)
            item->setColorPtr(&m_brushColor);
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
        item->setColorPtr(col);
}

void ItemGroup::setPenColor(QColor* col)
{
    for (GraphicsItem* item : *this)
        item->setPenColorPtr(col);
}

void ItemGroup::setZValue(qreal z)
{
    for (GraphicsItem* item : *this)
        item->setZValue(z);
}

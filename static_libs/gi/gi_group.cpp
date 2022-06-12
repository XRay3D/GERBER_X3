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
#include "gi_group.h"
#include "scene.h"

GiGroup::~GiGroup() {
    if (App::scene()->items().size())
        qDeleteAll(*this);
}

void GiGroup::push_back(GraphicsItem* item) {
    item->m_giId = mvector::size() ? mvector::back()->m_giId + 1 : 0;
    item->setToolTip((item->toolTip().isEmpty() ? QString() : item->toolTip() + '\n') + QString("ID(%1): %2").arg(item->type()).arg(item->m_giId));
    item->setVisible(m_visible);
    item->itemGroup = this;
    mvector::push_back(item);
}

void GiGroup::setVisible(bool visible) {
    if (m_visible != visible) {
        m_visible = visible;
        for (GraphicsItem* item : *this)
            item->setVisible(m_visible);
    }
}

void GiGroup::setSelected(const mvector<int>& ids) {
    for (GraphicsItem* item : *this)
        item->setSelected(ids.contains(item->id()));
}

void GiGroup::addToScene(QGraphicsScene* scene) {
    for (auto item : *this)
        (scene ? scene : App::scene())->addItem(item);
}

void GiGroup::setBrushColor(const QColor& color) {
    if (m_brushColor != color) {
        m_brushColor = color;
        for (GraphicsItem* item : *this)
            item->setColorPtr(&m_brushColor);
    }
}

void GiGroup::setPen(const QPen& pen) {
    if (m_pen != pen) {
        m_pen = pen;
        for (GraphicsItem* item : *this)
            item->setPen(m_pen);
    }
}

void GiGroup::setBrushColorP(QColor* col) {
    for (GraphicsItem* item : *this)
        item->setColorPtr(col);
}

void GiGroup::setPenColor(QColor* col) {
    for (GraphicsItem* item : *this)
        item->setPenColorPtr(col);
}

void GiGroup::setZValue(double z) {
    for (GraphicsItem* item : *this)
        item->setZValue(z);
}

void GiGroup::setPos(QPointF offset) {
    for (GraphicsItem* item : *this)
        item->setPos(offset);
}

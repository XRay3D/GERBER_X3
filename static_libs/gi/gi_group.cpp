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
    if (App::scene() && App::scene()->items().size())
        qDeleteAll(*this);
}

void GiGroup::push_back(GraphicsItem* item) {
    item->id_ = mvector::size() ? mvector::back()->id_ + 1 : 0;
    item->setToolTip((item->toolTip().isEmpty() ? QString() : item->toolTip() + '\n') + QString("ID(%1): %2").arg(item->type()).arg(item->id_));
    item->setVisible(visible_);
    item->itemGroup = this;
    mvector::push_back(item);
}

void GiGroup::setVisible(bool visible) {
    if (visible_ != visible) {
        visible_ = visible;
        for (GraphicsItem* item : *this)
            item->setVisible(visible_);
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
    if (brushColor_ != color) {
        brushColor_ = color;
        for (GraphicsItem* item : *this)
            item->setColorPtr(&brushColor_);
    }
}

void GiGroup::setPen(const QPen& pen) {
    if (pen_ != pen) {
        pen_ = pen;
        for (GraphicsItem* item : *this)
            item->setPen(pen_);
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

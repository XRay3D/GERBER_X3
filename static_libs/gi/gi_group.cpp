/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_group.h"
#include "graphicsview.h"

namespace Gi {

Group::~Group() {
    auto scene{App::grView().scene()};
    if(scene && scene->items().size())
        qDeleteAll(*this);
}

void Group::push_back(Item* item) {
    item->id_ = mvector::size() ? mvector::back()->id_ + 1 : 0;
    item->setToolTip((item->toolTip().isEmpty() ? QString() : item->toolTip() + '\n') + QString("ID(%1): %2").arg(item->type()).arg(item->id_));
    item->setVisible(visible_);
    item->itemGroup = this;
    mvector::push_back(item);
}

void Group::setVisible(bool visible) {
    if(visible_ != visible) {
        visible_ = visible;
        for(Item* item: *this)
            item->setVisible(visible_);
    }
}

void Group::setSelected(const mvector<int>& ids) {
    for(Item* item: *this)
        item->setSelected(ids.contains(item->id()));
}

void Group::addToScene(QGraphicsScene* scene) {
    if(!scene)
        scene = App::grView().scene();
    for(auto* item: *this)
        scene->addItem(item);
}

void Group::setBrushColor(const QColor& color) {
    if(brushColor_ != color) {
        brushColor_ = color;
        for(Item* item: *this)
            item->setColorPtr(&brushColor_);
    }
}

void Group::setPen(const QPen& pen) {
    if(pen_ != pen) {
        pen_ = pen;
        for(Item* item: *this)
            item->setPen(pen_);
    }
}

void Group::setBrushColorP(QColor* col) {
    for(Item* item: *this)
        item->setColorPtr(col);
}

void Group::setPenColor(QColor* col) {
    for(Item* item: *this)
        item->setPenColorPtr(col);
}

void Group::setZValue(double z) {
    for(Item* item: *this)
        item->setZValue(z);
}

void Group::setPos(QPointF offset) {
    for(Item* item: *this)
        item->setPos(offset);
}

} // namespace Gi

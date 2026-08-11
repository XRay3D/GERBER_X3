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
#include "gi_group.h"
#include "graphicsview.h"

namespace Gi {

// Group::~Group() {
//     auto scene{App::grView().scene()};
//     if(scene && scene->items().size())
//         qDeleteAll(*this);
// }

void Group::push_back(Item* item) {
    item->id_ = size() ? back()->id_ + 1 : 0;
    item->setToolTip((item->toolTip().isEmpty() ? QString() : item->toolTip() + u'\n') + u"ID(%1): %2"_s.arg(item->type()).arg(item->id_));
    item->setVisible(visible_);
    item->itemGroup = this;
    emplace_back(item);
}

void Group::setVisible(bool visible) {
    if(visible_ != visible) {
        visible_ = visible;
        for(auto&& item: *this)
            item->setVisible(visible_);
    }
}

void Group::setSelected(const std::vector<int>& ids) {
    for(auto&& item: *this)
        item->setSelected(r::contains(ids, item->id()));
}

void Group::addToScene(QGraphicsScene* scene) {
    auto* view = App::grViewPtr();
    if(!scene) {
        if(!view) return;
        scene = view->scene();
    }
    // Пакетная вставка: на каждый addItem индекс BSP делает поддержку, а так
    // он перестраивается один раз.
    const auto old = scene->itemIndexMethod();
    scene->setItemIndexMethod(QGraphicsScene::NoIndex);
    for(auto&& item: *this)
        scene->addItem(item.get());
    scene->setItemIndexMethod(old);
    // sceneRect должен покрывать новые данные -- отложенно, чтобы загрузка
    // десяти слоёв не пересчитывала габарит всей сцены десять раз.
    if(view) view->scheduleSceneRectUpdate();
}

void Group::setBrushColor(const QColor& color) {
    if(brushColor_ != color) {
        brushColor_ = color;
        for(auto&& item: *this)
            item->setColorPtr(&brushColor_);
    }
}

void Group::setPen(const QPen& pen) {
    if(pen_ != pen) {
        pen_ = pen;
        for(auto&& item: *this)
            item->setPen(pen_);
    }
}

void Group::setBrushColorP(QColor* col) {
    for(auto&& item: *this)
        item->setColorPtr(col);
}

void Group::setPenColor(QColor* col) {
    for(auto&& item: *this)
        item->setPenColorPtr(col);
}

void Group::setZValue(double z) {
    for(auto&& item: *this)
        item->setZValue(z);
}

void Group::setPos(QPointF offset) {
    for(auto&& item: *this)
        item->setPos(offset);
}

} // namespace Gi

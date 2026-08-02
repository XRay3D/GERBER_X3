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
#include "gi_datapath.h"

#include "abstract_file.h"
#include "project.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <set>

namespace Gi {

static QPainterPathStroker str{
    {Qt::transparent, 1., Qt::SolidLine, Qt::SquareCap, Qt::BevelJoin}
};

void DataPath::updateSelection() const {
    const double scale = scaleFactor();
    if(qFuzzyCompare(scale_, scale)) return;
    constexpr auto width{5}; // screen pixels
    scale_ = scale;
#if 1
    str.setWidth(width * scale);
    selectionShape_ = str.createStroke(shape_);
    boundingRect_ = selectionShape_.boundingRect();
#else
    auto tmpPpath = Inflate(toPaths(curves_), width * scale * uScale, JoinType::Miter, EndType::Square);
    selectionShape_.clear();
    for(Path& path: tmpPpath) selectionShape_.addPolygon(~path);
    boundingRect_ = selectionShape_.boundingRect();
    assert(tmpPpath.size());
    assert(tmpPpath.front().size());
#endif
    // qWarning() << shape_;
    // qWarning() << selectionShape_;
    // FIXME dont create empty assert(shape_.elementCount());
    // FIXME dont create empty assert(selectionShape_.elementCount() > 1);
}

DataPath::DataPath(Curves curves, AbstractFile* file)
    : Item{file} {
    curves_ = std::move(curves);
    shape_ = toPPath(curves_);
    updateSelection();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setSelected(false);
}

QPainterPath DataPath::shape() const { return selectionShape_; }

QVariant DataPath::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if(change == ItemVisibleChange && value.toBool()) {
        updateSelection();
    } else if(change == ItemSelectedChange && App::settings().animSelection()) {
        if(value.toBool()) {
            updateSelection();
            // FIXME          timer.connect(&timer, &QTimer::timeout, this, &DataPath::redraw);
        } else {
            // FIXME         timer.disconnect(&timer, &QTimer::timeout, this, &DataPath::redraw);
            update();
        }
    }
    return value;
}

int DataPath::type() const { return Type::DataPath; }

void DataPath::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsItem::hoverEnterEvent(event);
    updateSelection();
}

void DataPath::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    Item::mouseReleaseEvent(event);

    // using v::filter;
    // using v::transform;
    // auto constexpr filter = v::filter([](auto* item) {
    // return item->type() == int(Type::DataPath);
    // });

    // auto constexpr transform = v::transform([](auto* item) {
    // return static_cast<Item*>(item);
    // });

    const auto glue = App::project().glue() /** uScale*/;

    // std::map<void*, Path> set;
    std::set<void*> set;
    std::function<void(Item*)> selector = [&](Item* item) {
        const auto& collidingItems = *itemGroup;
        // auto collidingItems = scene()->collidingItems(item, Qt::/*IntersectsItemBoundingRect*/ IntersectsItemBoundingRect/*IntersectsItemShape*/);
        auto pathFrom = item->curves().front();
        for(auto&& item: collidingItems /*| filter | transform*/) {
            auto pathTo = item->curves().front();
            // if(/*itemGroup->contains(item) &&*/ !set.contains(item)) {
            if(set.insert(item.get()).second) {
                // auto [i, _] = set.emplace(item, Path{pathTo.front(), pathTo.back()});

                const double min = r::min({
                    Length(pathFrom.back(), pathTo.back()),
                    Length(pathFrom.back(), pathTo.front()),
                    Length(pathFrom.front(), pathTo.back()),
                    Length(pathFrom.front(), pathTo.front()),
                });

                if(min > glue) continue;
                item->setSelected(true);
                selector(item.get());
            }
        }
    };

    if(event->modifiers() & Qt::ShiftModifier && itemGroup)
        selector(this);
}

void DataPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    if(pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if(colorPtr_)
        color_ = *colorPtr_;
    // pen_.setWidth(penWidth());
    updateSelection();

    QColor color{pen_.color()};
    QPen pen{pen_};

    if(option->state & (QStyle::State_MouseOver | QStyle::State_Selected)) {
        if(option->state & QStyle::State_Selected) {
            color.setAlpha(255);
            pen.setColor(color);
            pen.setDashOffset(App::dashOffset());
        }
        pen.setWidthF(2 * scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({pi * 2, pi * 2 - 1});
    }
    if(option->state & QStyle::State_MouseOver) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor{255, 255, 255, 128});
        painter->drawPath(selectionShape_);
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(shape_);
}

} // namespace Gi

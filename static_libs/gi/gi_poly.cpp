// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_poly.h"

#include "abstract_file.h"
#include "gc_types.h"
#include "graphicsview.h"
#include "project.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <qcolor.h>
#include <qnamespace.h>
#include <ranges>
#include <set>

namespace Gi {

PolyLine::PolyLine(const Poly& polyline, AbstractFile* file)
    : Item{file}, polylines{polyline} {
    shape_ = polyToPPath(polyline);
    boundingRect_ = shape_.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setSelected(false);
}

PolyLine::PolyLine(const Polys& polylines, AbstractFile* file)
    : Item{file}, polylines{polylines} {
    shape_ = polyToPPath(polylines);
    boundingRect_ = shape_.boundingRect();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setSelected(false);
}

void PolyLine::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    if(pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if(colorPtr_)
        color_ = *colorPtr_;

    QColor color{pen_.color()};
    QPen pen{pen_};
    constexpr double dl = 3;

    if(option->state & (QStyle::State_MouseOver | QStyle::State_Selected)) {
        if(option->state & QStyle::State_Selected) {
            color.setAlpha(255);
            pen.setColor(color);
            pen.setDashOffset(App::dashOffset());
        }
        pen.setWidthF(2.0 * scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({dl, dl - 1});
    }
    if(option->state & QStyle::State_MouseOver) {
        painter->setPen(Qt::NoPen);
        painter->setBrush(QColor{255, 255, 255, 128});
    }

    painter->setPen({Qt::red, 1.0 * scaleFactor()} /*pen*/);
    painter->setBrush(QColor{255, 0, 0, 32});
    painter->drawPath(shape_);
}

int PolyLine::type() const { return Type::PolyLine; }

QPainterPath PolyLine::shape() const { return shape_; }

void PolyLine::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    Item::mouseReleaseEvent(event);

    // // using std::views::filter;
    // // using std::views::transform;
    // auto constexpr filter = std::views::filter([](auto* item) {
    //     return item->type() == int(Type::PolyLine);
    // });

    // auto constexpr transform = std::views::transform([](auto* item) {
    //     return static_cast<Item*>(item);
    // });

    // const auto glue = App::project().glue() /** uScale*/;

    // // std::map<void*, Path> set;
    // std::set<void*> set;
    // std::function<void(Item*)> selector = [&](Item* item) {
    //     const auto& collidingItems = *itemGroup;
    //     //  auto collidingItems = scene()->collidingItems(item, Qt::/*IntersectsItemBoundingRect*/ IntersectsItemBoundingRect/*IntersectsItemShape*/);
    //     auto pathFrom = item->paths().front();
    //     for(auto* item: collidingItems /*| filter | transform*/) {
    //         auto pathTo = item->paths().front();
    //         // if(/*itemGroup->contains(item) &&*/ !set.contains(item)) {
    //         if(set.insert(item).second) {
    //             // auto [i, _] = set.emplace(item, Path{pathTo.front(), pathTo.back()});

    //             const double dists[]{
    //                 distTo(pathFrom.back(), pathTo.back()) * dScale,
    //                 distTo(pathFrom.back(), pathTo.front()) * dScale,
    //                 distTo(pathFrom.front(), pathTo.back()) * dScale,
    //                 distTo(pathFrom.front(), pathTo.front()) * dScale,
    //             };

    //             const double min = std::ranges::min(dists);

    //             if(min > glue) continue;
    //             item->setSelected(true);
    //             selector(item);
    //         }
    //     }
    // };

    // if(event->modifiers() & Qt::ShiftModifier && itemGroup)
    //     selector(this);
}

QVariant PolyLine::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if(change == ItemVisibleChange && value.toBool()) {
        // updateSelection();
    } else if(change == ItemSelectedChange && App::settings().animSelection()) {
        if(value.toBool()) {
            // updateSelection();
            //  FIXME          timer.connect(&timer, &QTimer::timeout, this, &Polyline::redraw);
        } else {
            //   FIXME         timer.disconnect(&timer, &QTimer::timeout, this, &Polyline::redraw);
            update();
        }
    }
    return value;
}

void PolyLine::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsItem::hoverEnterEvent(event);
    // updateSelection();
}

} // namespace Gi

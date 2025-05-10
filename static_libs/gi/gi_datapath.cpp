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
#include "gi_datapath.h"

#include "abstract_file.h"
#include "gc_types.h"
#include "graphicsview.h"
#include "project.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <ranges>
#include <set>

namespace Gi {

DataPath::DataPath(const Path& path, AbstractFile* file)
    : Item{file} {
    shape_.addPolygon(~path);
    updateSelection();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setSelected(false);
}

DataPath::DataPath(const Paths& paths, AbstractFile* file)
    : Item{file} {
    for(auto&& path: paths)
        shape_.addPolygon(~path);
    updateSelection();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setSelected(false);
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
        painter->drawPath(selectionShape_);
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(shape_);
}

int DataPath::type() const { return Type::DataPath; }

QPainterPath DataPath::shape() const { return selectionShape_; }

void DataPath::updateSelection() const {
    if(const double scale = scaleFactor(); !qFuzzyCompare(scale_, scale)) {
        scale_ = scale;
        selectionShape_ = {};    // reset QPainterPath
                                 // ClipperOffset offset;
                                 // offset.AddPath(Path{~shape_.toSubpathPolygons().front()}, JoinType::Square, EndType::Square);
                                 // auto tmpPpath{offset.Execute(5 * uScale * scale_)};
        constexpr auto width{5}; // screen pixels
        auto tmpPpath = Inflate({~shape_.toSubpathPolygons().front()}, width * uScale * scale_, JoinType::Miter, EndType::Square);
        for(auto&& path: tmpPpath)
            selectionShape_.addPolygon(~path);
        boundingRect_ = selectionShape_.boundingRect();
    }
}

void DataPath::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    Item::mouseReleaseEvent(event);

    // using std::views::filter;
    // using std::views::transform;
    // auto constexpr filter = std::views::filter([](auto* item) {
    //     return item->type() == int(Type::DataPath);
    // });

    // auto constexpr transform = std::views::transform([](auto* item) {
    //     return static_cast<Item*>(item);
    // });

    const auto glue = App::project().glue() /** uScale*/;

    // std::map<void*, Path> set;
    std::set<void*> set;
    std::function<void(Item*)> selector = [&](Item* item) {
        const auto& collidingItems = *itemGroup;
        //  auto collidingItems = scene()->collidingItems(item, Qt::/*IntersectsItemBoundingRect*/ IntersectsItemBoundingRect/*IntersectsItemShape*/);
        auto pathFrom = item->paths().front();
        for(auto* item: collidingItems /*| filter | transform*/) {
            auto pathTo = item->paths().front();
            // if(/*itemGroup->contains(item) &&*/ !set.contains(item)) {
            if(set.insert(item).second) {
                // auto [i, _] = set.emplace(item, Path{pathTo.front(), pathTo.back()});

                const double dists[]{
                    distTo(pathFrom.back(), pathTo.back()) * dScale,
                    distTo(pathFrom.back(), pathTo.front()) * dScale,
                    distTo(pathFrom.front(), pathTo.back()) * dScale,
                    distTo(pathFrom.front(), pathTo.front()) * dScale,
                };

                const double min = std::ranges::min(dists);

                if(min > glue) continue;
                item->setSelected(true);
                selector(item);
            }
        }
    };

    if(event->modifiers() & Qt::ShiftModifier && itemGroup)
        selector(this);
}

QVariant DataPath::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if(change == ItemVisibleChange && value.toBool()) {
        updateSelection();
    } else if(change == ItemSelectedChange && App::settings().animSelection()) {
        if(value.toBool()) {
            updateSelection();
            //  FIXME          timer.connect(&timer, &QTimer::timeout, this, &DataPath::redraw);
        } else {
            //   FIXME         timer.disconnect(&timer, &QTimer::timeout, this, &DataPath::redraw);
            update();
        }
    }
    return value;
}

void DataPath::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsItem::hoverEnterEvent(event);
    updateSelection();
}

} // namespace Gi

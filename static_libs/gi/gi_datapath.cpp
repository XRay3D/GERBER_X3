// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
#include "gi_datapath.h"

#include "abstract_file.h"
#include "graphicsview.h"
#include "project.h"
#include "settings.h"
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

    updateSelection();

    QColor color(pen_.color());
    QPen pen(pen_);
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

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(shape_);
}

int DataPath::type() const { return Type::DataPath; }

QPainterPath DataPath::shape() const { return selectionShape_; }

void DataPath::updateSelection() const {
    if(const double scale = scaleFactor(); !qFuzzyCompare(scale_, scale)) {
        scale_ = scale;
        selectionShape_ = QPainterPath();
        // ClipperOffset offset;
        // offset.AddPath(Path{~shape_.toSubpathPolygons().front()}, JoinType::Square, EndType::Square);
        // auto tmpPpath{offset.Execute(5 * uScale * scale_)};
        auto tmpPpath = InflatePaths({~shape_.toSubpathPolygons().front()}, 5 * uScale * scale_, JoinType::Square, EndType::Square);
        for(auto&& path: tmpPpath)
            selectionShape_.addPolygon(~path);
        boundingRect_ = selectionShape_.boundingRect();
    }
}

void DataPath::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    Item::mouseReleaseEvent(event);
    std::set<void*> set;
    std::function<void(QGraphicsItem*)> selector = [&](QGraphicsItem* item) {
        auto collidingItems = scene()->collidingItems(item, Qt::IntersectsItemShape);
        for(auto* item: collidingItems) {
            if(item->type() == int(Type::DataPath) && itemGroup->contains((Item*)item) && set.emplace(item).second) {
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

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
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
#include "gi_datapath.h"

#include "file.h"
#include "graphicsview.h"
#include "project.h"
#include "scene.h"
#include "settings.h"
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <ranges>

#ifdef __GNUC__
QTimer GiDataPath::timer;
#endif

GiDataPath::GiDataPath(const Path& path, FileInterface* file)
    : GraphicsItem(file) {
    shape_.addPolygon(path);
    updateSelection();
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setSelected(false);
    if (!timer.isActive()) {
        timer.start(50);
        connect(&timer, &QTimer::timeout, [] { ++GiDataPath::dashOffset; });
    }
}

void GiDataPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    if (pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if (colorPtr_)
        color_ = *colorPtr_;

    updateSelection();

    QColor color(pen_.color());
    QPen pen(pen_);
    constexpr double dl = 3;

    if (option->state & (QStyle::State_MouseOver | QStyle::State_Selected)) {
        if (option->state & QStyle::State_Selected) {
            color.setAlpha(255);
            pen.setColor(color);
            pen.setDashOffset(dashOffset);
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

int GiDataPath::type() const { return static_cast<int>(GiType::DataPath); }

QPainterPath GiDataPath::shape() const { return selectionShape_; }

void GiDataPath::updateSelection() const {
    if (const double scale = scaleFactor(); !qFuzzyCompare(scale_, scale)) {
        scale_ = scale;
        selectionShape_ = QPainterPath();
        Paths tmpPpath;
        ClipperOffset offset;
        offset.AddPath(shape_.toSubpathPolygons().front(), jtSquare, ClipperLib::etOpenSquare);
        offset.Execute(tmpPpath, 5 * uScale * scale_);
        for (const Path& path : qAsConst(tmpPpath))
            selectionShape_.addPolygon(path);
        boundingRect_ = selectionShape_.boundingRect();
    }
}

void GiDataPath::mouseReleaseEvent(QGraphicsSceneMouseEvent* event) {
    GraphicsItem::mouseReleaseEvent(event);
    std::set<void*> set;
    std::function<void(QGraphicsItem*)> selector = [&](QGraphicsItem* item) {
        auto collidingItems = scene()->collidingItems(item, Qt::IntersectsItemShape);
        for (auto* item : collidingItems) {
            if (item->type() == int(GiType::DataPath) && itemGroup->contains((GraphicsItem*)item) && set.emplace(item).second) {
                item->setSelected(true);
                selector(item);
            }
        }
    };

    if (event->modifiers() & Qt::ShiftModifier && itemGroup)
        selector(this);
}

QVariant GiDataPath::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if (change == ItemVisibleChange && value.toBool()) {
        updateSelection();
    } else if (change == ItemSelectedChange && App::settings().animSelection()) {
        if (value.toBool()) {
            updateSelection();
            connect(&timer, &QTimer::timeout, this, &GiDataPath::redraw);
        } else {
            disconnect(&timer, &QTimer::timeout, this, &GiDataPath::redraw);
            update();
        }
    }
    return value;
}

void GiDataPath::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    QGraphicsItem::hoverEnterEvent(event);
    updateSelection();
}

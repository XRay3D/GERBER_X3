// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gc_thermalpreviewitem.h"
#include "app.h"
#include "gc_thermalnode.h"
#include "graphicsview.h"
#include "tool_pch.h"
#include <QAnimationGroup>
#include <QElapsedTimer>
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMenu>
#include <QMutex>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

AbstractThermPrGi::AbstractThermPrGi(Tool& tool)
    : agr(this)
    , pa1(this, "bodyColor")
    , pa2(this, "pathColor")
    , tool(tool)
    , m_bodyColor(colors[(int)Colors::Default])
    , m_pathColor(colors[(int)Colors::UnUsed])

{
    agr.addAnimation(&pa1);
    agr.addAnimation(&pa2);

    pa1.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    pa1.setDuration(150);
    pa2.setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    pa2.setDuration(150);

    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setOpacity(0);
    setZValue(std::numeric_limits<double>::max() - 10);

    static QMutex m;
    m.lock();
    thpi.push_back(this);
    m.unlock();
}

AbstractThermPrGi::~AbstractThermPrGi() { thpi.clear(); }

void AbstractThermPrGi::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*) {
    if (m_pathColor.alpha()) {
        //        if (isEmpty > 0) {
        //            painter->setPen(QPen(App::settings().guiColor(GuiColors::ToolPath), 0.0));
        //            painter->setBrush(Qt::NoBrush);
        //            for (QPolygonF polygon : m_bridge) {
        //                polygon.append(polygon.first());
        //                painterPath.addPolygon(polygon);
        //                painter->drawPolyline(polygon);
        //            }
        //        } else {
        if (agr.state() == QAbstractAnimation::Running) {
            int a;
            QColor c1(App::settings().guiColor(GuiColors::CutArea));
            a = App::settings().guiColor(GuiColors::CutArea).alpha();
            c1.setAlpha(std::clamp(pathColor().alpha(), 0, a));
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(c1, diameter, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(painterPath);

            QColor c2(App::settings().guiColor(GuiColors::ToolPath));
            a = App::settings().guiColor(GuiColors::ToolPath).alpha();
            c2.setAlpha(std::clamp(pathColor().alpha(), 0, a));
            painter->setPen(QPen(c2, 2 * App::graphicsView()->scaleFactor()));
            painter->drawPath(painterPath);
        } else {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(App::settings().guiColor(GuiColors::CutArea), diameter, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(painterPath);
            QColor pc(m_bodyColor);
            pc.setAlpha(255);
            painter->setPen(QPen(App::settings().guiColor(GuiColors::ToolPath), 2 * App::graphicsView()->scaleFactor()));
            painter->drawPath(painterPath);
        }
        //        }
    }
    painter->setBrush(m_bodyColor);
    QColor p(m_bodyColor);
    p.setAlpha(255);
    painter->setPen(QPen(p, 0.0));
    painter->drawPath(sourcePath);
}

QRectF AbstractThermPrGi::boundingRect() const { return sourcePath.boundingRect().united(painterPath.boundingRect()); }

QPainterPath AbstractThermPrGi::shape() const { return sourcePath; }

int AbstractThermPrGi::type() const { return static_cast<int>(GiType::PrThermal); }

bool AbstractThermPrGi::isValid() const {
    return !previewPaths.empty() && m_node->isChecked();
}

void AbstractThermPrGi::changeColor() {
    pa1.setStartValue(m_bodyColor);
    if (colorState & Selected) {
        pa1.setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::SelectedHovered] : colors[(int)Colors::Selected]));
    } else {
        if (colorState & Used && !previewPaths.empty()) {
            pa1.setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::UsedHovered] : colors[(int)Colors::Used]));
        } else {
            pa1.setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::DefaultHovered] : colors[(int)Colors::Default]));
        }
    }
    pa2.setStartValue(m_pathColor);
    pa2.setEndValue(QColor((colorState & Used) ? colors[(int)Colors::Used] : colors[(int)Colors::UnUsed]));
    agr.start();
}

void AbstractThermPrGi::contextMenuEvent(QGraphicsSceneContextMenuEvent* event) {
    QMenu menu;
    if (m_node->isChecked())
        menu.addAction(QIcon::fromTheme("list-remove"), QObject::tr("Exclude from the calculation"), [this] {
            for (auto item : thpi)
                if ((item == this || item->isSelected()) && item->m_node->isChecked()) {
                    item->m_node->disable();
                    item->update();
                    item->mouseDoubleClickEvent(nullptr);
                }
        });
    else
        menu.addAction(QIcon::fromTheme("list-add"), QObject::tr("Include in the calculation"), [this] {
            for (auto item : thpi)
                if ((item == this || item->isSelected()) && !item->m_node->isChecked()) {
                    item->m_node->enable();
                    item->update();
                    item->mouseDoubleClickEvent(nullptr);
                }
        });
    menu.exec(event->screenPos());
}

void AbstractThermPrGi::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) {
    if (event) {
        QGraphicsItem::mouseDoubleClickEvent(event);
        m_node->isChecked() ? m_node->disable() : m_node->enable();
    }
    m_node->isChecked() ? colorState |= Used : colorState &= ~Used;
    changeColor();
}

void AbstractThermPrGi::hoverEnterEvent(QGraphicsSceneHoverEvent* event) {
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void AbstractThermPrGi::hoverLeaveEvent(QGraphicsSceneHoverEvent* event) {
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant AbstractThermPrGi::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value) {
    if (change == ItemSelectedChange) {
        if (value.toInt()) {
            colorState |= Selected;
            emit selectionChanged(m_node->index(), {});
        } else {
            colorState &= ~Selected;
            emit selectionChanged({}, m_node->index());
        }
        changeColor();
    } else if (change == ItemVisibleChange) {
        auto animation = new QPropertyAnimation(this, "opacity");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(200);
        animation->setStartValue(0.0);
        animation->setEndValue(1.0);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    return QGraphicsItem::itemChange(change, value);
}

ThermalPreviewItem::ThermalPreviewItem(const Paths* paths, const IntPoint pos, Tool& tool)
    : AbstractThermPrGi(tool)
    , paths_ { paths }
    , pos_ { pos } {
    for (QPolygonF polygon : *paths) {
        polygon.append(polygon.first());
        sourcePath.addPolygon(polygon);
    }
}

IntPoint ThermalPreviewItem::pos() const { return pos_; }

Paths ThermalPreviewItem::paths() const { return *paths_; }

void ThermalPreviewItem::redraw() {
    if (double d = tool.getDiameter(tool.depth()); cashedPath.empty() || !qFuzzyCompare(diameter, d)) {
        diameter = d;
        ClipperOffset offset;
        offset.AddPaths(*paths_, jtRound, etClosedPolygon);
        offset.Execute(cashedPath, diameter * uScale * 0.5); // toolpath
        offset.Clear();
        offset.AddPaths(cashedPath, jtMiter, etClosedLine);
        offset.Execute(cashedFrame, diameter * uScale * 0.1); // frame
        for (Path& path : cashedPath)
            path.push_back(path.front());
    }
    if (qFuzzyIsNull(m_node->tickness()) && m_node->count()) {
        m_bridge.clear();
    } else {
        Clipper clipper;
        clipper.AddPaths(cashedFrame, ptSubject, true);
        const auto rect(sourcePath.boundingRect());
        const IntPoint& center(rect.center());
        const double radius = sqrt((rect.width() + diameter) * (rect.height() + diameter)) * uScale;
        const auto fp(sourcePath.toFillPolygons());
        for (int i = 0; i < m_node->count(); ++i) { // Gaps
            ClipperOffset offset;
            double angle = i * 2 * pi / m_node->count() + qDegreesToRadians(m_node->angle());
            offset.AddPath({ center,
                               IntPoint(
                                   static_cast<cInt>((cos(angle) * radius) + center.X),
                                   static_cast<cInt>((sin(angle) * radius) + center.Y)) },
                jtSquare, etOpenButt);
            Paths paths;
            offset.Execute(paths, (m_node->tickness() + diameter) * uScale * 0.5);
            clipper.AddPath(paths.front(), ptClip, true);
        }
        clipper.Execute(ctIntersection, m_bridge, pftPositive);
    }
    { // cut
        Clipper clipper;
        clipper.AddPaths(cashedPath, ptSubject, false);
        clipper.AddPaths(m_bridge, ptClip, true);
        clipper.Execute(ctDifference, previewPaths, pftPositive);
    }
    painterPath = QPainterPath();
    for (QPolygonF polygon : previewPaths) {
        painterPath.moveTo(polygon.first());
        for (QPointF& pt : polygon)
            painterPath.lineTo(pt);
    }
    if (isEmpty == -1)
        isEmpty = previewPaths.empty();
    if (static_cast<bool>(isEmpty) != previewPaths.empty()) {
        isEmpty = previewPaths.empty();
        changeColor();
    }
    update();
}

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#ifdef GERBER
#include "thermalpreviewitem.h"

#include "gccreator.h"
#include "thermalnode.h"
#include "tooldatabase/tool.h"
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

#include "leakdetector.h"

QPainterPath ThermalPreviewItem::drawPoly(const Gerber::GraphicObject& go)
{
    QPainterPath painterPath;
    for (QPolygonF polygon : go.paths()) {
        polygon.append(polygon.first());
        painterPath.addPolygon(polygon);
    }
    return painterPath;
}

ThermalPreviewItem::ThermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool, double& depth)
    : agr(this)
    , pa1(this, "bodyColor")
    , pa2(this, "pathColor")
    , tool(tool)
    , m_depth(depth)
    , grob(&go)
    , sourcePath(drawPoly(go))
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
    thpi.append(this);
    m.unlock();
}

ThermalPreviewItem::~ThermalPreviewItem() { thpi.clear(); }

void ThermalPreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (tool.isValid() && m_pathColor.alpha()) {
        if (isEmpty > 0) {
            painter->setPen(QPen(m_pathColor, 0.0));
            painter->setBrush(Qt::NoBrush);
            for (QPolygonF polygon : m_bridge) {
                polygon.append(polygon.first());
                painterPath.addPolygon(polygon);
                painter->drawPolyline(polygon);
            }
        } else {
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(m_pathColor, diameter, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
            painter->drawPath(painterPath);
            QColor pc(m_bodyColor);
            pc.setAlpha(255);
            painter->setPen(QPen(Qt::red, 0.0));
            painter->drawPath(painterPath);
        }
    }
    painter->setBrush(m_bodyColor);
    QColor p(m_bodyColor);
    p.setAlpha(255);
    painter->setPen(QPen(p, 0.0));
    painter->drawPath(sourcePath);
}

QRectF ThermalPreviewItem::boundingRect() const { return sourcePath.boundingRect().united(painterPath.boundingRect()); }

QPainterPath ThermalPreviewItem::shape() const { return sourcePath; }

int ThermalPreviewItem::type() const { return static_cast<int>(GiType::ThermalPr); }

Point64 ThermalPreviewItem::pos() const { return grob->state().curPos(); }

Paths ThermalPreviewItem::bridge() const { return m_bridge; }

Paths ThermalPreviewItem::paths() const { return grob->paths(); }

void ThermalPreviewItem::redraw()
{

    if (double d = tool.getDiameter(m_depth); cashedPath.isEmpty() || !qFuzzyCompare(diameter, d)) {
        diameter = d;
        ClipperOffset offset;
        offset.AddPaths(grob->paths(), jtRound, etClosedPolygon);
        offset.Execute(cashedPath, diameter * uScale * 0.5); // toolpath
        offset.Clear();
        offset.AddPaths(cashedPath, jtMiter, etClosedLine);
        offset.Execute(cashedFrame, diameter * uScale * 0.1); // frame
        for (Path& path : cashedPath)
            path.append(path.first());
    }

    if (qFuzzyIsNull(m_node->tickness()) && m_node->count()) {
        m_bridge.clear();
    } else {
        Clipper clipper;
        clipper.AddPaths(cashedFrame, ptSubject, true);
        const auto rect(sourcePath.boundingRect());
        const Point64& center(rect.center());
        const double radius = sqrt((rect.width() + diameter) * (rect.height() + diameter)) * uScale;
        const auto fp(sourcePath.toFillPolygons());
        for (int i = 0; i < m_node->count(); ++i) { // Gaps
            ClipperOffset offset;
            double angle = i * 2 * M_PI / m_node->count() + qDegreesToRadians(m_node->angle());
            offset.AddPath({ center,
                               Point64(
                                   static_cast<cInt>((cos(angle) * radius) + center.X),
                                   static_cast<cInt>((sin(angle) * radius) + center.Y)) },
                jtSquare, etOpenButt);
            Paths paths;
            offset.Execute(paths, (m_node->tickness() + diameter) * uScale * 0.5);
            clipper.AddPath(paths.first(), ptClip, true);
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
        isEmpty = previewPaths.isEmpty();
    if (static_cast<bool>(isEmpty) != previewPaths.isEmpty()) {
        isEmpty = previewPaths.isEmpty();
        changeColor();
    }

    update();
}

bool ThermalPreviewItem::isValid() const { return !previewPaths.isEmpty() && m_node->isChecked(); }

void ThermalPreviewItem::changeColor()
{
    pa1.setStartValue(m_bodyColor);
    if (colorState & Selected) {
        pa1.setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::SelectedHovered] : colors[(int)Colors::Selected]));
    } else {
        if (colorState & Used && !previewPaths.isEmpty()) {
            pa1.setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::UsedHovered] : colors[(int)Colors::Used]));
        } else {
            pa1.setEndValue(QColor((colorState & Hovered) ? colors[(int)Colors::DefaultHovered] : colors[(int)Colors::Default]));
        }
    }
    pa2.setStartValue(m_pathColor);
    pa2.setEndValue(QColor((colorState & Used) ? colors[(int)Colors::Default] : colors[(int)Colors::UnUsed]));
    agr.start();
}

void ThermalPreviewItem::contextMenuEvent(QGraphicsSceneContextMenuEvent* event)
{
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

void ThermalPreviewItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    if (event) {
        QGraphicsItem::mouseDoubleClickEvent(event);
        m_node->isChecked() ? m_node->disable() : m_node->enable();
    }
    m_node->isChecked() ? colorState |= Used : colorState &= ~Used;
    changeColor();
}

void ThermalPreviewItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void ThermalPreviewItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant ThermalPreviewItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
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
#endif

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "thermalpreviewitem.h"
//#include "graphicsview.h"
//#include "settings.h"
#include "gccreator.h"
#include "thermalnode.h"
#include "tooldatabase/tool.h"
#include <QElapsedTimer>
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMenu>
#include <QMutex>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

#include "leakdetector.h"

QPainterPath ThermalPreviewItem::drawPoly(const Gerber::GraphicObject& go)
{
    QPainterPath painterPath;
    for (QPolygonF& polygon : toQPolygons(go.paths() /* go.gFile->apertures()->value(id)->draw(go.state)*/)) {
        polygon.append(polygon.first());
        painterPath.addPolygon(polygon);
    }
    //    const double hole = go.gFile->apertures()->value(id)->drillDiameter() * 0.5;
    //    if (hole)
    //        painterPath.addEllipse(go.state().curPos()), hole, hole);
    return painterPath;
}

ThermalPreviewItem::ThermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool, double& depth)
    : tool(tool)
    , m_depth(depth)
    , grob(&go)
    , sourcePath(drawPoly(go))
    , m_bodyColor(colors[(int)Colors::Default])
    , m_pathColor(colors[(int)Colors::UnUsed])
{
    static QMutex m;
    setZValue(std::numeric_limits<double>::max() - 10);
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    connect(this, &ThermalPreviewItem::colorChanged, [this] { update(); });
    m.lock();
    thpi.append(this);
    m.unlock();
}

ThermalPreviewItem::~ThermalPreviewItem() { thpi.clear(); }

void ThermalPreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    if (tool.isValid() && m_pathColor.alpha()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(m_pathColor, diameter, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(painterPath);

        QColor pc(m_pathColor);
        pc.setAlpha(pc.alpha() * 255.0 / light);
        painter->setPen(QPen(pc, 0.0));
        painter->drawPath(painterPath);
    }
    painter->setBrush(m_bodyColor);
    QColor p(m_bodyColor);
    p.setAlpha(255);
    painter->setPen(QPen(p, 0.0));
    painter->drawPath(sourcePath);
}

QRectF ThermalPreviewItem::boundingRect() const { return sourcePath.boundingRect().united(painterPath.boundingRect()); }

QPainterPath ThermalPreviewItem::shape() const { return sourcePath; }

int ThermalPreviewItem::type() const { return GiThermalPr; }

IntPoint ThermalPreviewItem::pos() const { return grob->state().curPos(); }

Paths ThermalPreviewItem::bridge() const { return m_bridge; }

Paths ThermalPreviewItem::paths() const { return grob->paths(); }

Paths ThermalPreviewItem::toolPath() const { return m_toolPath; }

void ThermalPreviewItem::redraw()
{
    QElapsedTimer t;
    t.start();

    diameter = tool.getDiameter(m_depth);
    {
        ClipperOffset offset;
        offset.AddPaths(grob->paths(), jtRound, etClosedPolygon);
        offset.Execute(m_toolPath, diameter * uScale * 0.5); // toolpath
        offset.Clear();
        offset.AddPaths(m_toolPath, jtMiter, etClosedLine);
        offset.Execute(m_bridge, diameter * uScale * 0.1); // frame
    }

    if (qFuzzyIsNull(m_node->tickness()) && m_node->count()) {
        m_bridge.clear();
    } else {
        Clipper clipper;
        clipper.AddPaths(m_bridge, ptSubject, true);
        const auto rect(sourcePath.boundingRect());
        const IntPoint& center(rect.center());
        const double radius = sqrt((rect.width() + diameter) * (rect.height() + diameter)) * uScale;
        const auto fp(sourcePath.toFillPolygons());
        for (int i = 0; i < m_node->count(); ++i) {
            ClipperOffset offset;
            double angle = i * 2 * M_PI / m_node->count() + qDegreesToRadians(m_node->angle());
            offset.AddPath({ center,
                               IntPoint(
                                   static_cast<cInt>((cos(angle) * radius) + center.X),
                                   static_cast<cInt>((sin(angle) * radius) + center.Y)) },
                jtSquare, etOpenButt);
            Paths paths;
            offset.Execute(paths, (m_node->tickness() + diameter) * uScale * 0.5);
            clipper.AddPath(paths.first(), ptClip, true);
        }
        clipper.Execute(ctIntersection, m_bridge, pftPositive);
    }
    {
        Clipper clipper;
        for (Path& path : m_toolPath)
            path.append(path.first());
        clipper.AddPaths(m_toolPath, ptSubject, false);
        clipper.AddPaths(m_bridge, ptClip, true);
        PolyTree polytree;
        clipper.Execute(ctDifference, polytree, pftPositive);
        PolyTreeToPaths(polytree, m_toolPath);
    }
    qDebug() << "redraw" << (t.nsecsElapsed() / 1000) << "us";
    m_isValid = !m_toolPath.isEmpty();
    painterPath = QPainterPath();
    for (QPolygonF& polygon : toQPolygons(m_toolPath)) {
        painterPath.moveTo(polygon.takeFirst());
        for (QPointF& pt : polygon)
            painterPath.lineTo(pt);
    }

    update();
}

bool ThermalPreviewItem::isValid() const { return m_isValid && m_node->isChecked(); }

void ThermalPreviewItem::changeColor()
{
    {
        auto animation = new QPropertyAnimation(this, "bodyColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(m_bodyColor);
        if (colorState & Selected) {
            animation->setEndValue((colorState & Hovered) ? colors[(int)Colors::SelectedHovered] : colors[(int)Colors::Selected]);
        } else {
            if (colorState & Used)
                animation->setEndValue((colorState & Hovered) ? colors[(int)Colors::UsedHovered] : colors[(int)Colors::Used]);
            else
                animation->setEndValue((colorState & Hovered) ? colors[(int)Colors::DefaultHovered] : colors[(int)Colors::Default]);
        }
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    {
        auto animation = new QPropertyAnimation(this, "pathColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(m_pathColor);
        animation->setEndValue((colorState & Used) ? colors[(int)Colors::Used] : colors[(int)Colors::UnUsed]);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
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
        if (m_node->isChecked())
            m_node->disable();
        else
            m_node->enable();
    }

    if (m_node->isChecked())
        colorState |= Used;
    else
        colorState &= ~Used;

    changeColor();
    update();
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
    }
    return QGraphicsItem::itemChange(change, value);
}

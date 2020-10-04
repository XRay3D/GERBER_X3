// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "thermalpreviewitem.h"
//#include "graphicsview.h"
//#include "settings.h"
#include "thermalnode.h"
#include "tooldatabase/tool.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMenu>
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
    , m_sourcePath(drawPoly(go))
    , m_bodyColor(colors[(int)Colors::Default])
    , m_pathColor(colors[(int)Colors::UnUsed])
{
    setZValue(std::numeric_limits<double>::max() - 10);
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    connect(this, &ThermalPreviewItem::colorChanged, [this] { update(); });
    hhh.append(this);
}

ThermalPreviewItem::~ThermalPreviewItem() { hhh.clear(); }

void ThermalPreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{

    painter->setBrush(m_bodyColor);
    QColor p(m_bodyColor);
    p.setAlpha(255);
    painter->setPen(QPen(p, 0.0));
    painter->drawPath(m_sourcePath);
    // draw hole
    if (tool.isValid() && m_pathColor.alpha()) {
        painter->setBrush(Qt::NoBrush);
        painter->setPen(QPen(m_pathColor, diameter, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        painter->drawPath(m_toolPath);

        QColor pc(m_pathColor);
        pc.setAlpha(pc.alpha() * 255 / 100.0);
        painter->setPen(QPen(pc, 0.0));
        painter->drawPath(m_toolPath);
    }
}

QRectF ThermalPreviewItem::boundingRect() const { return m_sourcePath.boundingRect().united(m_toolPath.boundingRect()); }

QPainterPath ThermalPreviewItem::shape() const { return m_sourcePath; }

int ThermalPreviewItem::type() const { return GiThermalPr; }

IntPoint ThermalPreviewItem::pos() const { return grob->state().curPos(); }

Paths ThermalPreviewItem::paths() const { return grob->paths(); }

Paths ThermalPreviewItem::bridge() const { return m_bridge; }

void ThermalPreviewItem::redraw()
{
    diameter = tool.getDiameter(m_depth);
    Paths paths;
    {
        ClipperOffset offset;
        offset.AddPaths(grob->paths(), jtRound, etClosedPolygon);
        offset.Execute(paths, diameter * uScale * 0.5);
    }
    Clipper clipper;
    for (Path& path : paths)
        path.append(path.first());
    clipper.AddPaths(paths, ptSubject, false);
    // create frame
    if (qFuzzyIsNull(m_node->tickness()) && m_node->count()) {
        m_bridge.clear();
    } else {
        {
            ClipperOffset offset;
            offset.AddPaths(paths, jtMiter, etClosedLine);
            offset.Execute(m_bridge, diameter * uScale * 0.1);
        }

        Clipper clipperBr;
        clipperBr.AddPaths(m_bridge, ptSubject, true);

        const IntPoint& center((m_sourcePath.boundingRect().center()));
        const double radius = (m_sourcePath.boundingRect().width() + m_sourcePath.boundingRect().height()) * uScale * 0.5;
        for (int i = 0; i < m_node->count(); ++i) {
            ClipperOffset offset;
            double angle = i * 2 * M_PI / m_node->count() + qDegreesToRadians(m_node->angle());
            Path path {
                center,
                IntPoint(
                    static_cast<cInt>((cos(angle) * radius) + center.X),
                    static_cast<cInt>((sin(angle) * radius) + center.Y))
            };
            offset.AddPath(path, jtSquare, etOpenSquare);
            Paths pathsBr;
            offset.Execute(pathsBr, (m_node->tickness() + diameter) * uScale * 0.5);
            clipperBr.AddPath(pathsBr.first(), ptClip, true);
        }
        clipperBr.Execute(ctIntersection, m_bridge, pftPositive);
    }
    clipper.AddPaths(m_bridge, ptClip, true);
    {
        PolyTree polytree;
        clipper.Execute(ctDifference, polytree, pftPositive);
        PolyTreeToPaths(polytree, paths);
    }
    m_isValid = !paths.isEmpty();
    m_toolPath = QPainterPath();
    for (QPolygonF& polygon : toQPolygons(paths)) {
        m_toolPath.moveTo(polygon.takeFirst());
        for (QPointF& pt : polygon)
            m_toolPath.lineTo(pt);
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
            for (auto item : hhh)
                if ((item == this || item->isSelected()) && item->m_node->isChecked()) {
                    item->m_node->disable();
                    item->update();
                    item->mouseDoubleClickEvent(nullptr);
                }
        });
    else
        menu.addAction(QIcon::fromTheme("list-add"), QObject::tr("Include in the calculation"), [this] {
            for (auto item : hhh)
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

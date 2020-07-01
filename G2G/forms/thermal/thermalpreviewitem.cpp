// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "thermalpreviewitem.h"
#include "tooldatabase/tool.h"
#include <QPainter>
#include <gbrfile.h>

QPainterPath ThermalPreviewItem::drawPoly(const Gerber::GraphicObject& go)
{
    QPainterPath painterPath;
    for (QPolygonF& polygon : toQPolygons(go.paths() /* go.gFile->apertures()->value(id)->draw(go.state)*/)) {
        polygon.append(polygon.first());
        painterPath.addPolygon(polygon);
    }
    //    const double hole = go.gFile->apertures()->value(id)->drillDiameter() * 0.5;
    //    if (hole)
    //        painterPath.addEllipse(toQPointF(go.state().curPos()), hole, hole);
    return painterPath;
}

ThermalPreviewItem::ThermalPreviewItem(const Gerber::GraphicObject& go, Tool& tool, double& depth)
    : tool(tool)
    , m_depth(depth)
    , grob(&go)
    , m_sourcePath(drawPoly(go))
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(QColor(255, 255, 255, 100))
{
    setZValue(std::numeric_limits<double>::max() - 10);
    setFlag(ItemIsSelectable, true);
    redraw();
}

void ThermalPreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    // draw source
    if (isSelected()) {
        m_pen.setColor(Qt::green);
        m_brush.setColor(QColor(0, 255, 0, 100));
    } else {
        m_pen.setColor(Qt::darkGray);
        m_brush.setColor(QColor(255, 255, 255, 100));
    }
    if (!(flags() & ItemIsSelectable))
        m_brush.setColor(QColor(0, 0, 0, 0));
    painter->setPen(m_pen);
    painter->setBrush(m_brush);
    painter->drawPath(m_sourcePath);
    // draw hole
    if (tool.isValid() && (flags() & ItemIsSelectable)) {
        //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
        //painter->setPen(QPen(Qt::red, 1.5 / scene()->views().first()->matrix().m11()));

        QPen pen(Qt::red, 0.0);
        QBrush br(QColor(255, 0, 0, 100));

        if (isSelected())
            painter->setPen(m_pen);
        else
            painter->setPen(pen);
        painter->setBrush(br);
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
    const double diameter = tool.getDiameter(m_depth);
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
    if (qFuzzyIsNull(m_tickness) && m_count) {
        m_bridge.clear();
    } else {
        ClipperOffset offset;
        offset.AddPaths(paths, jtMiter, etClosedLine);
        offset.Execute(m_bridge, diameter * uScale * 0.1);

        Clipper clipper;
        clipper.AddPaths(m_bridge, ptSubject, true);

        const IntPoint center(toIntPoint(m_sourcePath.boundingRect().center()));
        const double radius = (m_sourcePath.boundingRect().width() + m_sourcePath.boundingRect().height()) * uScale * 0.5;
        for (int i = 0; i < m_count; ++i) {
            ClipperOffset offset;
            Path path{
                center,
                IntPoint(
                    static_cast<cInt>((cos(i * 2 * M_PI / m_count + qDegreesToRadians(m_angle)) * radius) + center.X),
                    static_cast<cInt>((sin(i * 2 * M_PI / m_count + qDegreesToRadians(m_angle)) * radius) + center.Y))
            };
            offset.AddPath(path, jtSquare, etOpenSquare);
            Paths paths;
            offset.Execute(paths, (m_tickness + diameter) * uScale * 0.5);
            clipper.AddPath(paths.first(), ptClip, true);
        }
        clipper.Execute(ctIntersection, m_bridge, pftPositive);
    }
    clipper.AddPaths(m_bridge, ptClip, true);
    {
        PolyTree polytree;
        clipper.Execute(ctDifference, polytree, pftPositive);
        PolyTreeToPaths(polytree, paths);
    }
    {
        ClipperOffset offset;
        offset.AddPaths(paths, jtRound, etOpenRound);
        offset.Execute(paths, diameter * uScale * 0.5);
    }
    m_isValid = !paths.isEmpty();
    m_toolPath = QPainterPath();
    for (QPolygonF& polygon : toQPolygons(paths)) {
        polygon.append(polygon.first());
        m_toolPath.addPolygon(polygon);
    }
    update();
}

double ThermalPreviewItem::angle() const { return m_angle; }

void ThermalPreviewItem::setAngle(double angle)
{
    m_angle = angle;
    redraw();
}

double ThermalPreviewItem::tickness() const { return m_tickness; }

void ThermalPreviewItem::setTickness(double tickness)
{
    m_tickness = tickness;
    redraw();
}

int ThermalPreviewItem::count() const { return m_count; }

void ThermalPreviewItem::setCount(int count)
{
    m_count = count;
    redraw();
}

ThermalNode* ThermalPreviewItem::node() const
{
    return m_node;
}

bool ThermalPreviewItem::isValid() const
{
    return flags() & QGraphicsItem::ItemIsSelectable && m_isValid;
}

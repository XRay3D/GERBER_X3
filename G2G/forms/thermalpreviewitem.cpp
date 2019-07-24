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

ThermalPreviewItem::~ThermalPreviewItem() {}

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

int ThermalPreviewItem::type() const { return ThermalType; }

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
    clipper.AddPaths(paths, ptSubject, false);
    // create frame
    {
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
                    (cos(i * 2 * M_PI / m_count + qDegreesToRadians(m_angle)) * radius) + center.X,
                    (sin(i * 2 * M_PI / m_count + qDegreesToRadians(m_angle)) * radius) + center.Y)
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

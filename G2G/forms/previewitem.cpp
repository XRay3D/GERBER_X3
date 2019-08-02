#include "previewitem.h"
#include "tooldatabase/tool.h"
#include <QPainter>
#include <exfile.h>
#include <gbrfile.h>

extern Paths offset(const Path& /*path*/, double offset, bool fl = false);

QPainterPath PreviewItem::drawApetrure(const Gerber::GraphicObject& go, int id)
{
    QPainterPath painterPath;
    for (QPolygonF& polygon : toQPolygons(go.paths() /* go.gFile->apertures()->value(id)->draw(go.state)*/)) {
        polygon.append(polygon.first());
        painterPath.addPolygon(polygon);
    }
    const double hole = go.gFile()->apertures()->value(id)->drillDiameter() * 0.5;
    if (hole != 0.0)
        painterPath.addEllipse(toQPointF(go.state().curPos()), hole, hole);
    return painterPath;
}

QPainterPath PreviewItem::drawDrill(const Excellon::Hole& hole)
{
    QPainterPath painterPath;
    painterPath.addEllipse(hole.state.offsetedPos(), hole.state.currentToolDiameter() * 0.5, hole.state.currentToolDiameter() * 0.5);
    return painterPath;
}

QPainterPath PreviewItem::drawSlot(const Excellon::Hole& hole)
{
    QPainterPath painterPath;
    for (Path& path : offset(hole.item->paths().first(), hole.state.currentToolDiameter()))
        painterPath.addPolygon(toQPolygon(path));
    return painterPath;
}

PreviewItem::PreviewItem(const Gerber::GraphicObject& go, int id)
    : id(id)
    , grob(&go)
    , m_sourcePath(drawApetrure(go, id))
    , m_sourceDiameter(go.gFile()->apertures()->value(id)->drillDiameter() ? go.gFile()->apertures()->value(id)->drillDiameter() : go.gFile()->apertures()->value(id)->minSize())
    , m_type(ApetrureType)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
    setZValue(std::numeric_limits<double>::max() - 10);
    setFlag(ItemIsSelectable, true);
}

PreviewItem::PreviewItem(const Excellon::Hole& hole)
    : hole(&hole)
    , m_sourcePath(hole.state.path.isEmpty() ? drawDrill(hole) : drawSlot(hole))
    , m_sourceDiameter(hole.state.currentToolDiameter())
    , m_type(hole.state.path.isEmpty() ? DrillType : SlotType)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
    setZValue(std::numeric_limits<double>::max() - 10);
    setFlag(ItemIsSelectable, true);
}

void PreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    // draw source
    if (isSelected()) {
        m_pen.setColor(Qt::green);
        m_brush.setColor(Qt::green);
    } else {
        m_pen.setColor(Qt::darkGray);
        m_brush.setColor(Qt::darkGray);
    }
    painter->setPen(m_pen);
    painter->setBrush(m_brush);
    painter->drawPath(m_sourcePath);
    // draw hole
    if (m_toolId > -1) {
        //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
        //painter->setPen(QPen(Qt::red, 1.5 / scene()->views().first()->matrix().m11()));
        if (isSelected())
            painter->setPen(m_pen);
        else
            painter->setPen(QPen(Qt::red, 0.0));
        painter->setBrush(QBrush(QColor(255, 0, 0, 100)));
        painter->drawPath(m_toolPath);
    }
}

QRectF PreviewItem::boundingRect() const { return m_sourcePath.boundingRect().united(m_toolPath.boundingRect()); }

int PreviewItem::type() const { return m_type; }

double PreviewItem::sourceDiameter() const { return m_sourceDiameter; }

int PreviewItem::toolId() const { return m_toolId; }

void PreviewItem::setToolId(int toolId)
{
    m_toolId = toolId;
    if (m_toolId > -1) {
        m_toolPath = QPainterPath();
        const double diameter = ToolHolder::tools[m_toolId].diameter();
        switch (m_type) {
        case SlotType: {
            Paths tmpPpath;
            ClipperOffset offset;
            offset.AddPath(hole->item->paths().first(), jtRound, etOpenRound);
            offset.Execute(tmpPpath, diameter * 0.5 * uScale);
            for (Path& path : tmpPpath) {
                path.append(path.first());
                m_toolPath.addPolygon(toQPolygon(path));
            }
            Path path(hole->item->paths().first());
            if (path.size()) {
                for (IntPoint& pt : path) {
                    m_toolPath.moveTo(toQPointF(pt) - QPointF(0.0, diameter * 0.7));
                    m_toolPath.lineTo(toQPointF(pt) + QPointF(0.0, diameter * 0.7));
                    m_toolPath.moveTo(toQPointF(pt) - QPointF(diameter * 0.7, 0.0));
                    m_toolPath.lineTo(toQPointF(pt) + QPointF(diameter * 0.7, 0.0));
                }
                m_toolPath.moveTo(toQPointF(path.first()));
                for (IntPoint& pt : path) {
                    m_toolPath.lineTo(toQPointF(pt));
                }
            }
        } break;
        case DrillType:
            m_toolPath.addEllipse(hole->state.offsetedPos(), diameter * 0.5, diameter * 0.5);
            m_toolPath.moveTo(hole->state.offsetedPos() - QPointF(0.0, diameter * 0.7));
            m_toolPath.lineTo(hole->state.offsetedPos() + QPointF(0.0, diameter * 0.7));
            m_toolPath.moveTo(hole->state.offsetedPos() - QPointF(diameter * 0.7, 0.0));
            m_toolPath.lineTo(hole->state.offsetedPos() + QPointF(diameter * 0.7, 0.0));
            break;
        case ApetrureType:
            m_toolPath.addEllipse(toQPointF(grob->state().curPos()), diameter * 0.5, diameter * 0.5);
            m_toolPath.moveTo(toQPointF(grob->state().curPos()) - QPointF(0.0, diameter * 0.7));
            m_toolPath.lineTo(toQPointF(grob->state().curPos()) + QPointF(0.0, diameter * 0.7));
            m_toolPath.moveTo(toQPointF(grob->state().curPos()) - QPointF(diameter * 0.7, 0.0));
            m_toolPath.lineTo(toQPointF(grob->state().curPos()) + QPointF(diameter * 0.7, 0.0));
            break;
        }
    }
    update();
}

IntPoint PreviewItem::pos() const
{
    switch (m_type) {
    case SlotType:
        return toIntPoint(hole->state.offsetedPos());
    case DrillType:
        return toIntPoint(hole->state.offsetedPos());
    case ApetrureType:
        return grob->state().curPos();
    }
    return IntPoint();
}

Paths PreviewItem::paths() const
{
    switch (m_type) {
    case SlotType:
        return hole->item->paths();
    case DrillType: {
        Paths paths(hole->item->paths());
        return ReversePaths(paths);
    }
    case ApetrureType:
        return grob->paths();
    }
    return Paths();
}

bool PreviewItem::fit(double depth)
{
    switch (m_type) {
    case SlotType:
    case DrillType:
        return m_sourceDiameter > ToolHolder::tools[m_toolId].getDiameter(depth);
    case ApetrureType:
        return grob->gFile()->apertures()->value(id)->fit(ToolHolder::tools[m_toolId].getDiameter(depth));
    }
    return false;
}

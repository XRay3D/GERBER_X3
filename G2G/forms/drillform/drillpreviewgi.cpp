// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "drillpreviewgi.h"
#include "gbrfile.h"
#include "gi/drillitem.h"
#include "tooldatabase/tool.h"
#include <QPainter>

extern Paths offset(const Path& /*path*/, double offset, bool fl = false);

QPainterPath DrillPrGI::drawApetrure(const Gerber::GraphicObject& go, int id)
{
    QPainterPath painterPath;
    for (QPolygonF& polygon : toQPolygons(go.paths() /* go.gFile->apertures()->value(id)->draw(go.state)*/)) {
        polygon.append(polygon.first());
        painterPath.addPolygon(polygon);
    }
    const double hole = go.gFile()->apertures()->value(id)->drillDiameter() * 0.5;
    if (hole != 0.0)
        painterPath.addEllipse(go.state().curPos(), hole, hole);
    return painterPath;
}

QPainterPath DrillPrGI::drawDrill(const Excellon::Hole& hole)
{
    QPainterPath painterPath;
    painterPath.addEllipse(hole.state.offsetedPos(), hole.state.currentToolDiameter() * 0.5, hole.state.currentToolDiameter() * 0.5);
    return painterPath;
}

QPainterPath DrillPrGI::drawSlot(const Excellon::Hole& hole)
{
    QPainterPath painterPath;
    for (Path& path : offset(hole.item->paths().first(), hole.state.currentToolDiameter()))
        painterPath.addPolygon(toQPolygon(path));
    return painterPath;
}

DrillPrGI::DrillPrGI(const Gerber::GraphicObject& go, int id)
    : id(id)
    , grob(&go)
    , m_sourcePath(drawApetrure(go, id))
    , m_sourceDiameter(qFuzzyIsNull(go.gFile()->apertures()->value(id)->drillDiameter()) ? go.gFile()->apertures()->value(id)->minSize() : go.gFile()->apertures()->value(id)->drillDiameter())
    , m_type(GiApetrurePr)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
    setZValue(std::numeric_limits<double>::max() - 10);
    setFlag(ItemIsSelectable, true);
}

DrillPrGI::DrillPrGI(const Excellon::Hole& hole)
    : hole(&hole)
    , m_sourcePath(hole.state.path.isEmpty() ? drawDrill(hole) : drawSlot(hole))
    , m_sourceDiameter(hole.state.currentToolDiameter())
    , m_type(hole.state.path.isEmpty() ? GiDrillPr : GiSlotPr)
    , m_pen(Qt::darkGray, 0.0)
    , m_brush(Qt::darkGray)
{
    setZValue(std::numeric_limits<double>::max() - 10);
    setFlag(ItemIsSelectable, true);
}

void DrillPrGI::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
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

QRectF DrillPrGI::boundingRect() const { return m_sourcePath.boundingRect().united(m_toolPath.boundingRect()); }

int DrillPrGI::type() const { return m_type; }

double DrillPrGI::sourceDiameter() const { return m_sourceDiameter; }

int DrillPrGI::toolId() const { return m_toolId; }

void DrillPrGI::setToolId(int toolId)
{
    m_toolId = toolId;
    if (m_toolId > -1) {
        m_toolPath = QPainterPath(); //.clear();
        const double diameter = ToolHolder::tools[m_toolId].diameter();
        switch (m_type) {
        case GiSlotPr: {
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
                    m_toolPath.moveTo(pt - QPointF(0.0, diameter * 0.7));
                    m_toolPath.lineTo(pt + QPointF(0.0, diameter * 0.7));
                    m_toolPath.moveTo(pt - QPointF(diameter * 0.7, 0.0));
                    m_toolPath.lineTo(pt + QPointF(diameter * 0.7, 0.0));
                }
                m_toolPath.moveTo(path.first());
                for (IntPoint& pt : path) {
                    m_toolPath.lineTo(pt);
                }
            }
        } break;
        case GiDrillPr: {
            const auto offsetedPos = hole->state.offsetedPos();
            m_toolPath.addEllipse(offsetedPos, diameter * 0.5, diameter * 0.5);
            m_toolPath.moveTo(offsetedPos - QPointF(0.0, diameter * 0.7));
            m_toolPath.lineTo(offsetedPos + QPointF(0.0, diameter * 0.7));
            m_toolPath.moveTo(offsetedPos - QPointF(diameter * 0.7, 0.0));
            m_toolPath.lineTo(offsetedPos + QPointF(diameter * 0.7, 0.0));
        } break;
        case GiApetrurePr: {
            const auto curPos = grob->state().curPos();
            m_toolPath.addEllipse(curPos, diameter * 0.5, diameter * 0.5);
            m_toolPath.moveTo(curPos - QPointF(0.0, diameter * 0.7));
            m_toolPath.lineTo(curPos + QPointF(0.0, diameter * 0.7));
            m_toolPath.moveTo(curPos - QPointF(diameter * 0.7, 0.0));
            m_toolPath.lineTo(curPos + QPointF(diameter * 0.7, 0.0));
        } break;
        }
    }
    update();
}

IntPoint DrillPrGI::pos() const
{
    switch (m_type) {
    case GiSlotPr:
        return hole->state.offsetedPos();
    case GiDrillPr:
        return hole->state.offsetedPos();
    case GiApetrurePr:
        return grob->state().curPos();
    }
    return IntPoint();
}

Paths DrillPrGI::paths() const
{
    switch (m_type) {
    case GiSlotPr:
        return hole->item->paths();
    case GiDrillPr: {
        Paths paths(hole->item->paths());
        return ReversePaths(paths);
    }
    case GiApetrurePr:
        return grob->paths();
    }
    return Paths();
}

bool DrillPrGI::fit(double depth)
{
    switch (m_type) {
    case GiSlotPr:
    case GiDrillPr:
        return m_sourceDiameter > ToolHolder::tools[m_toolId].getDiameter(depth);
    case GiApetrurePr:
        return grob->gFile()->apertures()->value(id)->fit(ToolHolder::tools[m_toolId].getDiameter(depth));
    }
    return false;
}

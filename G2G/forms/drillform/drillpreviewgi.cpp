// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "drillpreviewgi.h"
#include "gbrfile.h"
#include "gi/drillitem.h"
#include "tooldatabase/tool.h"
#include <QPainter>
#include <QPropertyAnimation>

extern Paths offset(const Path& path, double offset, bool fl = false);

QPainterPath DrillPrGI::drawApetrure(const Gerber::GraphicObject& go, int id)
{
    QPainterPath painterPath;
    for (QPolygonF& polygon : toQPolygons(go.paths())) {
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
    , m_bodyColor(colors[(int)Colors::Default])
    , m_pathColor(colors[(int)Colors::UnUsed])
{
    connect(this, &DrillPrGI::colorChanged, [this] { update(); });
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setOpacity(0);
    setZValue(std::numeric_limits<double>::max() - 10);
}

DrillPrGI::DrillPrGI(const Excellon::Hole& hole)
    : hole(&hole)
    , m_sourcePath(hole.state.path.isEmpty() ? drawDrill(hole) : drawSlot(hole))
    , m_sourceDiameter(hole.state.currentToolDiameter())
    , m_type(hole.state.path.isEmpty() ? GiDrillPr : GiSlotPr)
    , m_bodyColor(colors[(int)Colors::Default])
    , m_pathColor(colors[(int)Colors::UnUsed])
{
    connect(this, &DrillPrGI::colorChanged, [this] { update(); });
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setOpacity(0);
    setZValue(std::numeric_limits<double>::max() - 10);
}

void DrillPrGI::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{
    painter->setPen({ m_bodyColor, 0.0 });
    painter->setBrush(m_bodyColor);
    painter->drawPath(m_sourcePath);
    // draw tool
    if (m_toolId > -1) {
        QColor p(m_pathColor);
        p.setAlpha(255);
        painter->setPen(QPen(p, 0.0));
        painter->setBrush(Qt::NoBrush /*m_pathColor*/);
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

    if (m_toolId > -1)
        colorState |= Tool;
    else
        colorState &= ~Tool;

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
    changeColor();
}

void DrillPrGI::setUsed(bool fl)
{
    if (fl)
        colorState |= Used;
    else
        colorState &= ~Used;
    changeColor();
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

void DrillPrGI::changeColor()
{
    {
        auto animation = new QPropertyAnimation(this, "bodyColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(200);
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
        animation->setEndValue((colorState & Tool) ? colors[(int)Colors::Tool] : colors[(int)Colors::UnUsed]);
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
}

void DrillPrGI::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    colorState |= Hovered;
    changeColor();
    QGraphicsItem::hoverEnterEvent(event);
}

void DrillPrGI::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    colorState &= ~Hovered;
    changeColor();
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant DrillPrGI::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        if (value.toInt()) {
            colorState |= Selected;
        } else {
            colorState &= ~Selected;
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

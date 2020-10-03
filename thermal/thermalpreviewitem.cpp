// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "thermalpreviewitem.h"
#include "thermalnode.h"
#include "tooldatabase/tool.h"
#include <QGraphicsSceneContextMenuEvent>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPropertyAnimation>
#include <QStyleOptionGraphicsItem>
#include <QtMath>

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
    , mbColor(QColor(255, 255, 255, 0))
    , mpColor(QColor(255, 0, 0, 0))
{
    setZValue(std::numeric_limits<double>::max() - 10);
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    redraw();
    connect(this, &ThermalPreviewItem::colorChanged, [this] { update(); });
    hhh.append(this);
}

ThermalPreviewItem::~ThermalPreviewItem()
{
    hhh.clear();
}

void ThermalPreviewItem::paint(QPainter* painter, const QStyleOptionGraphicsItem*, QWidget*)
{

    painter->setBrush(mbColor);
    QColor p(mbColor);
    p.setAlpha(255);
    painter->setPen(QPen(p, 0.0));
    painter->drawPath(m_sourcePath);
    // draw hole
    if (tool.isValid() && m_node->isChecked()) { //(flags() & ItemIsSelectable)) {
        //item->setBrush(QBrush(Qt::red, Qt::Dense4Pattern));
        //painter->setPen(QPen(Qt::red, 1.5 / scene()->views().first()->matrix().m11()));
        painter->setBrush(mpColor);
        QColor p(mpColor);
        p.setAlpha(255);
        painter->setPen(QPen(p, 0.0));
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
            double angle = i * 2 * M_PI / m_count + qDegreesToRadians(m_angle);
            Path path {
                center,
                IntPoint(
                    static_cast<cInt>((cos(angle) * radius) + center.X),
                    static_cast<cInt>((sin(angle) * radius) + center.Y))
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

bool ThermalPreviewItem::isValid() const { return m_isValid && m_node->isChecked(); }

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
    auto* animation = new QPropertyAnimation(this, "pColor");
    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    animation->setDuration(100);
    animation->setStartValue(mpColor);
    animation->setEndValue(m_node->isChecked() ? QColor(255, 0, 0, 100) : QColor(255, 0, 0, 0));
    connect(animation, &QPropertyAnimation::finished, [animation, this] {
        setProperty("pColor", animation->endValue());
        update();
    });
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    update();
}

void ThermalPreviewItem::hoverEnterEvent(QGraphicsSceneHoverEvent* event)
{
    auto* animation = new QPropertyAnimation(this, "bColor");
    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    animation->setDuration(100);
    animation->setStartValue(mbColor);
    if (isSelected()) {
        animation->setEndValue(QColor(0, 255, 0, 200));
    } else {
        animation->setEndValue(QColor(255, 255, 255, 100));
    }
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    hover = true;
    QGraphicsItem::hoverEnterEvent(event);
}

void ThermalPreviewItem::hoverLeaveEvent(QGraphicsSceneHoverEvent* event)
{
    auto* animation = new QPropertyAnimation(this, "bColor");
    animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
    animation->setDuration(100);
    animation->setStartValue(mbColor);
    if (isSelected()) {
        animation->setEndValue(QColor(0, 255, 0, 100));
    } else {
        animation->setEndValue(QColor(255, 255, 255, 0));
    }
    animation->start(QAbstractAnimation::DeleteWhenStopped);
    hover = false;
    QGraphicsItem::hoverLeaveEvent(event);
}

QVariant ThermalPreviewItem::itemChange(QGraphicsItem::GraphicsItemChange change, const QVariant& value)
{
    if (change == ItemSelectedChange) {
        auto* animation = new QPropertyAnimation(this, "bColor");
        animation->setEasingCurve(QEasingCurve(QEasingCurve::Linear));
        animation->setDuration(100);
        animation->setStartValue(mbColor);
        if (value.toInt()) {
            animation->setEndValue(hover ? QColor(0, 255, 0, 200) : QColor(0, 255, 0, 100));
            emit selectionChanged(m_node->index(), {});

        } else {
            animation->setEndValue(hover ? QColor(255, 255, 255, 100) : QColor(255, 255, 255, 0));
            emit selectionChanged({}, m_node->index());
        }
        animation->start(QAbstractAnimation::DeleteWhenStopped);
    }
    return QGraphicsItem::itemChange(change, value);
}

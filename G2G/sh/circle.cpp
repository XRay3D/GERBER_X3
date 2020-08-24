// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "circle.h"
#include "constructor.h"
#include "sh.h"
#include <QGraphicsScene>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QStyleOptionGraphicsItem>
#include <graphicsview.h>
#include <math.h>
#include <scene.h>
#include <settings.h>

namespace Shapes {
Circle::Circle(QPointF center, QPointF pt)
    : m_radius(QLineF(center, pt).length())
{
    m_paths.resize(1);
    sh = { new SH(this, true), new SH(this) };
    sh[Center]->setPos(center);
    sh[Point1]->setPos(pt);

    redraw();
    setFlags(ItemIsSelectable | ItemIsFocusable);
    setAcceptHoverEvents(true);
    setZValue(std::numeric_limits<double>::max());

    App::scene()->addItem(this);
    App::scene()->addItem(sh[Center]);
    App::scene()->addItem(sh[Point1]);
}

Circle::Circle(QDataStream& stream)
    : Shape(stream)
{
}

Circle::~Circle() { qDebug(Q_FUNC_INFO); }

void Circle::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_brColorPtr)
        m_brush.setColor(*m_brColorPtr);

    QColor color(m_pen.color());
    QPen pen(m_pen);

    if (option->state & QStyle::State_Selected) {
        color.setAlpha(255);
        pen.setColor(color);
        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
    }
    if (option->state & QStyle::State_MouseOver) {
        pen.setColor(Qt::red);
        //        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
        //        pen.setStyle(Qt::CustomDashLine);
        //        pen.setCapStyle(Qt::FlatCap);
        //        pen.setDashPattern({ 3.0, 3.0 });
    }

    painter->setPen(pen);
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(m_shape);
}

void Circle::redraw()
{
    m_radius = (QLineF(sh[Center]->pos(), sh[Point1]->pos()).length());
    const int intSteps = GlobalSettings::gbrGcCircleSegments(m_radius);
    const cInt radius = static_cast<cInt>(m_radius * uScale);
    const IntPoint center(toIntPoint(sh[Center]->pos()));
    const double delta_angle = (2.0 * M_PI) / intSteps;
    Path& path = m_paths.first();
    path.clear();
    for (int i = 0; i < intSteps; i++) {
        const double theta = delta_angle * i;
        path.append(IntPoint(
            static_cast<cInt>(radius * cos(theta)) + center.X,
            static_cast<cInt>(radius * sin(theta)) + center.Y));
    }
    path.append(path.first());
    m_shape = QPainterPath();
    m_shape.addPolygon(toQPolygon(path));
    m_rect = m_shape.boundingRect();
    setPos({ 1, 1 }); //костыли    //update();
    setPos({ 0, 0 });
}

void Circle::setPt(const QPointF& pt)
{
    if (sh[Point1]->pos() == pt)
        return;
    sh[Point1]->setPos(pt);
    redraw();
}

double Circle::radius() const
{
    return m_radius;
}

void Circle::setRadius(double radius)
{
    if (!qFuzzyCompare(m_radius, radius))
        return;
    m_radius = radius;
    redraw();
}
}

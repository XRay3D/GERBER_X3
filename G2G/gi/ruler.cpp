// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "ruler.h"
#include "graphicsitem.h"

#include <QDebug>
#include <QPainter>
#include <graphicsview.h>

Ruler::Ruler(const QPointF& point)
    : m_pt1(point)
    , m_pt2(point)
{
    setZValue(std::numeric_limits<double>::max());
    m_font.setPixelSize(16);
}

Ruler::~Ruler()
{
}

QRectF Ruler::boundingRect() const
{
    //return QRectF(-500, -500, 500, 500);
    const double k = GraphicsView::scaleFactor();
    const double width = (m_textRect.width() + 10) * k;
    const double height = (m_textRect.height() + 10) * k;

    return QRectF(
               QPointF(qMin(m_pt1.x(), m_pt2.x()), qMin(m_pt1.y(), m_pt2.y())),
               QPointF(qMax(m_pt1.x(), m_pt2.x()), qMax(m_pt1.y(), m_pt2.y())))
        + QMarginsF(width, height * 2, width * 2, height);
}

void Ruler::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    return;
    QLineF line(m_pt2, m_pt1);

    if (qFuzzyIsNull(line.length()))
        return;

    const double angle = line.angle();
    const double k = GraphicsView::scaleFactor();

    painter->save();
    painter->setBrush(QColor(127, 127, 127, 100));
    painter->setPen(QPen(Qt::green, 0.0)); //1.5 * k));
    painter->setRenderHint(QPainter::Antialiasing, false);
    // draw rect
    painter->drawRect(QRectF(
        QPointF(qMin(m_pt1.x(), m_pt2.x()), qMin(m_pt1.y(), m_pt2.y())),
        QPointF(qMax(m_pt1.x(), m_pt2.x()), qMax(m_pt1.y(), m_pt2.y()))));

    // draw cross
    const double length = 20.0 * k;
    painter->drawLine(QLineF::fromPolar(length, 0.000).translated(m_pt1));
    painter->drawLine(QLineF::fromPolar(length, 90.00).translated(m_pt1));
    painter->drawLine(QLineF::fromPolar(length, 180.0).translated(m_pt1));
    painter->drawLine(QLineF::fromPolar(length, 270.0).translated(m_pt1));

    painter->drawLine(QLineF::fromPolar(length, 0.000).translated(m_pt2));
    painter->drawLine(QLineF::fromPolar(length, 90.00).translated(m_pt2));
    painter->drawLine(QLineF::fromPolar(length, 180.0).translated(m_pt2));
    painter->drawLine(QLineF::fromPolar(length, 270.0).translated(m_pt2));
    painter->restore();

    // draw arrow
    painter->setPen(QPen(Qt::white, 0.0));
    painter->drawLine(line);
    line.setLength(20.0 * k);
    line.setAngle(angle + 10);
    painter->drawLine(line);
    line.setAngle(angle - 10);
    painter->drawLine(line);

    // draw text
    painter->setRenderHint(QPainter::Antialiasing, true);
    painter->setFont(m_font);
    painter->translate(m_pt2);
    painter->scale(k, -k);
    //const QPainter::CompositionMode cm = painter->compositionMode();
    //painter->setCompositionMode(QPainter::CompositionMode_Exclusion);
    painter->drawText(m_textRect, Qt::AlignLeft, m_text);
    //painter->setCompositionMode(cm);
    //    QPainterPath path;
    //    path.addText(0.0, 0.0, m_font, m_text);
    //    painter->setPen(Qt::NoPen);
    //    painter->setBrush(Qt::white);
    //    path.translate(m_pt2);
    //    for (const QPolygonF& poly : path.toFillPolygons()) {
    //        painter->drawPolygon(poly);
    //    }
}

void Ruler::setPoint2(const QPointF& point2)
{
    QPointF& pt1 = m_pt1;
    QPointF& pt2 = m_pt2;
    pt2 = point2;
    QLineF line(pt1, pt2);
    const double width = pt1.x() > pt2.x() ? pt1.x() - pt2.x() : pt2.x() - pt1.x();
    const double height = pt1.y() > pt2.y() ? pt1.y() - pt2.y() : pt2.y() - pt1.y();
    const double length = line.length();
    m_text = QString("    ∆X = %1 mm\n"
                     "    ∆Y = %2 mm\n"
                     "    ∆ / = %3 mm")
                 .arg(width, 4, 'f', 3, '0')
                 .arg(height, 4, 'f', 3, '0')
                 .arg(length, 4, 'f', 3, '0');
    m_textRect = QFontMetricsF(m_font).boundingRect(QRectF(), Qt::AlignLeft, m_text);
    update();
}

int Ruler::type() const { return RulerType; }

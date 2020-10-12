#include "erroritem.h"
#include <QPainter>
#include <QTime>
#include <QtMath>

ErrorItem::ErrorItem(const Paths& paths, double area)
    : m_area(area)
{
    for (auto& path : paths)
        m_shape.addPolygon(path);
    setFlag(ItemIsSelectable);
}

double ErrorItem::area() const { return m_area; }

QRectF ErrorItem::boundingRect() const { return m_shape.boundingRect(); }

void ErrorItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    painter->setPen(Qt::NoPen);
    if (isSelected()) {
        static QTime t(QTime::currentTime());
        painter->setBrush(QColor::fromHsv(cos(t.msecsTo(QTime::currentTime()) / (2 * M_PI * 8)) * 30 + 30, 255, 255, 255));
    } else {
        QBrush br(QColor(255, 0, 255));
        //        br.setStyle(Qt::Dense4Pattern);
        //        QMatrix matrix;
        //        matrix.scale(App::graphicsView()->scaleFactor() * 3, App::graphicsView()->scaleFactor() * 3);
        //        br.setMatrix(matrix);
        painter->setBrush(br);
    }

    painter->drawPath(m_shape);
}

QPainterPath ErrorItem::shape() const { return m_shape; }

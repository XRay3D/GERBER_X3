// This is an open source non-commercial project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "componentitem.h"

#include <QPainter>
#include <abstractfile.h>
#include <graphicsview.h>

ComponentItem::ComponentItem(const Gerber::Component& component, AbstractFile* file)
    : GraphicsItem(file)
    , m_component(component)
{
    pathPins.resize(m_component.pins.size());
    m_shape.addPolygon(m_component.footprint);
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setToolTip(m_component.toolTip());
}

QRectF ComponentItem::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath ComponentItem::shape() const
{
    if (!qFuzzyCompare(m_scale, App::graphicsView()->scaleFactor())) {
        m_scale = App::graphicsView()->scaleFactor();
        QFont font;
        font.setPixelSize(16);
        {
            const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, m_component.refdes);
            pt = m_shape.boundingRect().center();
            pt.rx() -= textRect.width() * m_scale * 0.5;
            pt.ry() += textRect.height() * m_scale * 0.5;
            pathRefDes = QPainterPath();
            pathRefDes.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height()), font, m_component.refdes);
        }
        int i = 0;
        for (auto [number, description, pos] : m_component.pins) {
            Q_UNUSED(number)
            QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, description);
            pathPins[i].second = pos;
            pathPins[i].second.rx() -= textRect.width() * m_scale * 0.5;
            pathPins[i].second.ry() += textRect.height() * m_scale * 0.5;
            pathPins[i].first = QPainterPath();
            pathPins[i++].first.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height()), font, description);
        }
    }
    return m_shape;
}

void ComponentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    auto c { m_file->color() };
    painter->setBrush(c);
    c.setAlpha(255);
    painter->setPen({ c, 2 * m_scale });
    painter->drawPath(m_shape);
    painter->setBrush(Qt::NoBrush);
    double k = m_scale * 20;
    k = std::min(1., k);
    painter->drawLine(
        m_component.referencePoint + QPointF { k, k },
        m_component.referencePoint - QPointF { k, k });
    painter->drawLine(
        m_component.referencePoint + QPointF { -k, k },
        m_component.referencePoint - QPointF { -k, k });
    painter->setBrush(Qt::NoBrush);
    painter->drawRect(
        { m_component.referencePoint + QPointF { k, k },
            m_component.referencePoint - QPointF { k, k } });

    if (m_scale < 0.05) {
        const double k = m_scale * 20; //m_scale > 0.05 ? 1./10 / m_scale : 10;
        for (auto [number, description, pos] : m_component.pins) {
            Q_UNUSED(number)
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(Qt::green, 2 * m_scale, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter->drawLine(
                pos + QPointF { k, k },
                pos - QPointF { k, k });
            painter->drawLine(
                pos + QPointF { -k, k },
                pos - QPointF { -k, k });
        }
        for (auto [path, pos] : pathPins) {
            painter->save();
            painter->translate(pos);
            painter->scale(m_scale, -m_scale);
            painter->setPen(QPen(Qt::black, 4.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::red);
            painter->drawPath(path);
            painter->restore();
        }
    }

    painter->translate(pt);
    painter->scale(m_scale, -m_scale);
    painter->setPen(QPen(Qt::black, 4.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
    painter->setBrush(Qt::NoBrush);
    painter->drawPath(pathRefDes);
    painter->setPen(Qt::NoPen);
    painter->setBrush(Qt::white);
    painter->drawPath(pathRefDes);
}

void ComponentItem::mouseMoveEvent(QGraphicsSceneMouseEvent* event)
{
    qDebug() << event;
    QGraphicsItem::mouseMoveEvent(event);
}

void ComponentItem::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
    QGraphicsItem::mouseDoubleClickEvent(event);
}

Paths ComponentItem::paths() const { return {}; }

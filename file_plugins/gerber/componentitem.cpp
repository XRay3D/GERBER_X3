// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "componentitem.h"

#include "graphicsview.h"
#include "interfaces/file.h"

#include <QPainter>
#include <QTimer>
#include <future>

QTimer t;

ComponentItem::ComponentItem(const Gerber::Component& component, FileInterface* file)
    : GraphicsItem(file)
    , m_component(component)
{
    pathPins.resize(m_component.pins.size());
    m_shape.addPolygon(m_component.footprint);
    setAcceptHoverEvents(true);
    setFlag(ItemIsSelectable, true);
    setToolTip(m_component.toolTip());
    t.setSingleShot(true);
    //    connect(&t, &QTimer::timeout, [this] {
    //        std::future<int> f2 = std::async(std::launch::async, [this] {
    //            qDebug() << __FUNCTION__;
    //            QFont font;
    //            font.setPixelSize(20);
    //            {
    //                const QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, m_component.refdes);
    //                pt = m_shape.boundingRect().center();
    //                pt.rx() -= textRect.width() * m_scale * 0.5;
    //                pt.ry() += textRect.height() * m_scale * 0.5;
    //                pathRefDes = QPainterPath();
    //                pathRefDes.addText(textRect.topLeft() + QPointF(textRect.left(), textRect.height() * 2), font, m_component.refdes);
    //            }
    //            int i = 0;
    //            for (auto [number, description, pos] : m_component.pins) {
    //                QString text(description + '(' + number + ')');
    //                QRectF textRect = QFontMetricsF(font).boundingRect(QRectF(), Qt::AlignLeft, text);
    //                pathPins[i].second.rx() = pos.x() - textRect.width() * m_scale * 0.5;
    //                pathPins[i].second.ry() = pos.y() + textRect.height() * m_scale * 0.5;
    //                pathPins[i].first = QPainterPath();
    //                pathPins[i++].first.addText(textRect.topLeft() + QPointF(textRect.left(), 0 /*textRect.height()*/), font, text);
    //            }
    //            return 0;
    //        });
    //    });
}

QRectF ComponentItem::boundingRect() const
{
    return shape().boundingRect();
}

QPainterPath ComponentItem::shape() const
{
    if (!qFuzzyCompare(m_scale, App::graphicsView()->scaleFactor())) {
        m_scale = App::graphicsView()->scaleFactor();
    }
    return m_shape;
}

void drawText(QPainter* painter, const QString& str, const QColor& color, QPointF pt, double scale)
{
    painter->save();
    static QFont f("Consolas");
    f.setPixelSize(20);
    f.setBold(true);
    const QRectF textRect = QFontMetricsF(f).boundingRect(QRectF(), Qt::AlignLeft, str);
    pt.rx() -= textRect.width() * scale * 0.5;
    pt.ry() += textRect.height() * scale * 0.5;
    painter->translate(pt);
    painter->scale(scale, -scale);
    painter->setPen(color);
    painter->setFont(f);
    painter->drawText(textRect.topLeft() + QPointF(textRect.left(), textRect.height() * 2), str);
    painter->restore();
}

void ComponentItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/)
{
    auto c { m_file->color() };
    painter->setBrush(c);
    c.setAlpha(255);
    painter->setPen({ c, 2 * m_scale });
    painter->drawPath(m_shape);
    painter->setBrush(Qt::NoBrush);
    double min = std::min(m_shape.boundingRect().width(), m_shape.boundingRect().height());
    double k = std::min(min, m_scale * 20);
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
        double size = std::min(min, m_scale * 20);
        for (auto [number, description, pos] : m_component.pins) {
            Q_UNUSED(number)
            Q_UNUSED(description)
            painter->setBrush(Qt::NoBrush);
            painter->setPen(QPen(c, 2 * m_scale, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter->drawLine(pos + QPointF { +size, +size }, pos - QPointF { +size, +size });
            painter->drawLine(pos + QPointF { -size, +size }, pos - QPointF { -size, +size });
        }
        for (auto [path, pos] : pathPins) {
            painter->save();
            painter->translate(pos);
            painter->scale(m_scale, -m_scale);
            painter->setPen(QPen(Qt::black, 4.0, Qt::SolidLine, Qt::SquareCap, Qt::MiterJoin));
            painter->setBrush(Qt::NoBrush);
            painter->drawPath(path);
            painter->setPen(Qt::NoPen);
            painter->setBrush(Qt::white);
            painter->drawPath(path);
            painter->restore();
        }
    }

    drawText(painter,
        m_component.refdes,
        Qt::red,
        m_shape.boundingRect().center(),
        m_scale);

    if (m_scale < 0.05)
        for (auto [number, description, pos] : m_component.pins)
            drawText(painter,
                QString(description + '(' + number + ')'),
                App::settings().guiColor(GuiColors::Background).rgb() ^ 0xFFFFFF,
                pos + QPointF(0, m_scale * 60),
                m_scale);
}

Paths ComponentItem::paths() const
{
    return {};
}

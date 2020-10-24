// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "pathitem.h"

#include "gcode.h"
#include "graphicsview.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

PathItem::PathItem(const Paths& paths, GCode::File* file)
    : m_gcFile(file)
{
    for (const Path& path : paths)
        m_shape.addPolygon(path);
    double k;
    if (m_gcFile)
        k = m_gcFile->m_gcp.getToolDiameter() * 0.5;
    else
        k = m_pen.widthF() * 0.5;
    m_rect = m_shape.boundingRect() + QMarginsF(k, k, k, k);
#ifdef QT_DEBUG
    //setAcceptHoverEvents(true);
#endif
}

PathItem::PathItem(const Path& path, GCode::File* file)
    : m_gcFile(file)
{
    m_shape.addPolygon(path);
    double k;
    if (m_gcFile)
        k = m_gcFile->m_gcp.getToolDiameter() * 0.5;
    else
        k = m_pen.widthF() * 0.5;
    m_rect = m_shape.boundingRect() + QMarginsF(k, k, k, k);
#ifdef QT_DEBUG
    //setAcceptHoverEvents(true);
#endif
}

QRectF PathItem::boundingRect() const { return m_rect; }

void PathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    Q_UNUSED(option)

    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_colorPtr)
        m_color = *m_colorPtr;

    if (m_pen.widthF() == 0) {
        QPen pen(m_pen);
        pen.setWidthF(1.5 * App::graphicsView()->scaleFactor());
        painter->setPen(pen);
    } else
        painter->setPen(m_pen);
#ifdef QT_DEBUG
    if (option->state & QStyle::State_MouseOver) {
        QPen pen(m_pen);
        pen.setWidthF(2.0 * App::graphicsView()->scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ 2.0, 2.0 });
        painter->setPen(pen);
    }
#endif
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(m_shape);

    //    painter->setPen(QPen(Qt::magenta, 0.0));
    //    painter->drawRect(m_rect);

    ////////////////////////////////////////////////////// for debug cut direction
#ifdef QT_DEBUG
    if (m_sc != App::graphicsView()->scaleFactor())
        updateArrows();
    painter->drawPath(m_arrows);
#endif
}

int PathItem::type() const { return static_cast<int>(GiType::Path); }

Paths PathItem::paths() const { return {} /*m_paths*/; }
#ifdef QT_DEBUG
void PathItem::updateArrows()
{
    m_sc = App::graphicsView()->scaleFactor();
    m_arrows = QPainterPath(); //.clear();
    if (qFuzzyIsNull(m_pen.widthF())) {
        for (const QPolygonF& path : m_shape.toSubpathPolygons()) {
            for (int i = 0; i < path.size() - 1; ++i) {
                QLineF line(path[i + 1], path[i]);
                double length = 20 * App::graphicsView()->scaleFactor();
                if (line.length() < length && i)
                    continue;
                if (length > 0.5)
                    length = 0.5;
                const double angle = line.angle();
                line.setLength(length);
                line.setAngle(angle + 8);
                m_arrows.moveTo(line.p1());
                m_arrows.lineTo(line.p2());
                //painter->drawLine(line);
                line.setAngle(angle - 8);
                m_arrows.moveTo(line.p1());
                m_arrows.lineTo(line.p2());
                //painter->drawLine(line);

                //                    if (0) {
                //                        painter->save();
                //                        const QString text = "   " + QString::number(i);
                //                        const QRectF textRect = QFontMetricsF(painter->font()).boundingRect(QRectF(), Qt::AlignLeft, text);
                //                        const double k = 1.0 / GraphicsView ::123->matrix().m11();
                //                        painter->translate(path[i]);
                //                        painter->scale(k, -k);
                //                        //painter->setBrush(QColor(127, 127, 127, 255));
                //                        //painter->drawRect(textRect);
                //                        painter->drawText(textRect, Qt::AlignLeft, text);
                //                        painter->restore();
                //                    }
            }
        }
    }
}
#endif

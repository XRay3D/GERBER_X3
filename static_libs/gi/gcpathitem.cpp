// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "gcpathitem.h"

#include "gcode/gcode.h"

#include "graphicsview.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#define QT_DEBUG
//#undef QT_DEBUG

GcPathItem::GcPathItem(const Paths& paths, GCode::File* file)
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

GcPathItem::GcPathItem(const Path& path, GCode::File* file)
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

QRectF GcPathItem::boundingRect() const { return m_rect; }

void GcPathItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/)
{
    Q_UNUSED(option)

    if (m_pnColorPrt)
        m_pen.setColor(*m_pnColorPrt);
    if (m_colorPtr)
        m_color = *m_colorPtr;

    if (m_pen.widthF() == 0) {
        QPen pen(m_pen);
        pen.setWidthF(1.5 * scaleFactor());
        painter->setPen(pen);
    } else
        painter->setPen(m_pen);
#ifdef QT_DEBUG
    if (option->state & QStyle::State_MouseOver) {
        QPen pen(m_pen);
        pen.setWidthF(2.0 * scaleFactor());
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
    if (m_sc != scaleFactor())
        updateArrows();
    painter->drawPath(m_arrows);
#endif
}

int GcPathItem::type() const { return static_cast<int>(GiType::Path); }

Paths GcPathItem::paths(int) const { return {} /*m_paths*/; }
#ifdef QT_DEBUG
void GcPathItem::updateArrows()
{
    m_sc = scaleFactor();
    m_arrows = QPainterPath(); //.clear();
    if (qFuzzyIsNull(m_pen.widthF())) {
        for (const QPolygonF& path : m_shape.toSubpathPolygons()) {
            for (int i = 0; i < path.size() - 1; ++i) {
                QLineF line(path[i + 1], path[i]);
                double length = 30 * scaleFactor();
                if (line.length() < length && i)
                    continue;
                if (length > 0.5)
                    length = 0.5;
                const double angle = line.angle();
                line.setLength(length);
                line.setAngle(angle + 10);
                m_arrows.moveTo(line.p1());
                m_arrows.lineTo(line.p2());
                //painter->drawLine(line);
                line.setAngle(angle - 10);
                m_arrows.moveTo(line.p1());
                m_arrows.lineTo(line.p2());
                //painter->drawLine(line);
            }
        }
    }
}
#endif

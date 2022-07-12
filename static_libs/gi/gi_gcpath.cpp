// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "gi_gcpath.h"

#include "gcode.h"

#include "graphicsview.h"
#include <QPainter>
#include <QStyleOptionGraphicsItem>

#define QT_DEBUG
//#undef QT_DEBUG

GiGcPath::GiGcPath(const Paths& paths, GCode::File* file)
    : m_gcFile(file) {
    for (const Path& path : paths)
        shape_.addPolygon(path);
    double k;
    if (m_gcFile)
        k = m_gcFile->gcp_.getToolDiameter() * 0.5;
    else
        k = pen_.widthF() * 0.5;
    boundingRect_ = shape_.boundingRect() + QMarginsF(k, k, k, k);
#ifdef QT_DEBUG
    // setAcceptHoverEvents(true);
#endif
}

GiGcPath::GiGcPath(const Path& path, GCode::File* file)
    : m_gcFile(file) {
    shape_.addPolygon(path);
    double k;
    if (m_gcFile)
        k = m_gcFile->gcp_.getToolDiameter() * 0.5;
    else
        k = pen_.widthF() * 0.5;
    boundingRect_ = shape_.boundingRect() + QMarginsF(k, k, k, k);
#ifdef QT_DEBUG
    // setAcceptHoverEvents(true);
#endif
}

QRectF GiGcPath::boundingRect() const { return boundingRect_; }

void GiGcPath::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* /*widget*/) {
    Q_UNUSED(option)

    if (pnColorPrt_)
        pen_.setColor(*pnColorPrt_);
    if (colorPtr_)
        color_ = *colorPtr_;

    if (pen_.widthF() == 0) {
        QPen pen(pen_);
        pen.setWidthF(1.5 * scaleFactor());
        painter->setPen(pen);
    } else
        painter->setPen(pen_);
#ifdef QT_DEBUG
    if (option->state & QStyle::State_MouseOver) {
        QPen pen(pen_);
        pen.setWidthF(2.0 * scaleFactor());
        pen.setStyle(Qt::CustomDashLine);
        pen.setCapStyle(Qt::FlatCap);
        pen.setDashPattern({ 2.0, 2.0 });
        painter->setPen(pen);
    }
#endif
    painter->setBrush(QBrush(Qt::NoBrush));
    painter->drawPath(shape_);

    //    painter->setPen(QPen(Qt::magenta, 0.0));
    //    painter->drawRect(m_rect);

    ////////////////////////////////////////////////////// for debug cut direction
#ifdef QT_DEBUG
    if (m_sc != scaleFactor())
        updateArrows();
    painter->drawPath(m_arrows);
#endif
}

int GiGcPath::type() const { return static_cast<int>(GiType::Path); }

Paths GiGcPath::paths(int) const { return {} /*m_paths*/; }
#ifdef QT_DEBUG
void GiGcPath::updateArrows() {
    m_sc = scaleFactor();
    m_arrows = QPainterPath(); //.clear();
    if (qFuzzyIsNull(pen_.widthF())) {
        for (const QPolygonF& path : shape_.toSubpathPolygons()) {
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
                // painter->drawLine(line);
                line.setAngle(angle - 10);
                m_arrows.moveTo(line.p1());
                m_arrows.lineTo(line.p2());
                // painter->drawLine(line);
            }
        }
    }
}
#endif

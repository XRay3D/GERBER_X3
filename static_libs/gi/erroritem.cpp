// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/*******************************************************************************
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2022                                          *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*******************************************************************************/
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
        //        matrix.scale(scaleFactor() * 3, scaleFactor() * 3);
        //        br.setMatrix(matrix);
        painter->setBrush(br);
    }

    painter->drawPath(m_shape);
}

QPainterPath ErrorItem::shape() const { return m_shape; }

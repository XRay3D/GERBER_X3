// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "gi_error.h"
#include <QPainter>
#include <QTime>
#include <QtMath>
#include <numbers>

GiError::GiError(const Paths& paths, double area)
    : area_(area) {
    for (auto& path : paths)
        shape_.addPolygon(path);
    setFlag(ItemIsSelectable);
    setZValue(std::numeric_limits<double>::max());
}

double GiError::area() const { return area_; }

QRectF GiError::boundingRect() const { return shape_.boundingRect(); }

void GiError::paint(QPainter* painter, const QStyleOptionGraphicsItem* /*option*/, QWidget* /*widget*/) {
    painter->setPen(Qt::NoPen);
    if (isSelected()) {
        static QTime t(QTime::currentTime());
        painter->setBrush(QColor::fromHsv(cos(t.msecsTo(QTime::currentTime()) / (2 * pi * 8)) * 30 + 30, 255, 255, 255));
    } else {
        QBrush br(QColor(255, 0, 255));
        //        br.setStyle(Qt::Dense4Pattern);
        //        QMatrix matrix;
        //        matrix.scale(scaleFactor() * 3, scaleFactor() * 3);
        //        br.setMatrix(matrix);
        painter->setBrush(br);
    }

    painter->drawPath(shape_);
}

QPainterPath GiError::shape() const { return shape_; }

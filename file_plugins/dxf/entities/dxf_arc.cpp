// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

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
#include "dxf_arc.h"
#include "dxf_insert.h"
#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"
#include "settings.h"
#include <QGraphicsEllipseItem>

#include <QPainter>

namespace Dxf {

Arc::Arc(SectionParser* sp)
    : Entity(sp)
{
}

//void Arc::draw(const InsertEntity* const i) const
//{
//    if (i) {
//        for (int r = 0; r < i->rowCount; ++r) {
//            for (int c = 0; c < i->colCount; ++c) {
//                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
//                GraphicObject go(toGo());
//                i->transform(go, tr);
//                i->attachToLayer(std::move(go));
//            }
//        }
//    } else {
//        attachToLayer(toGo());
//    }
//}

void Arc::parse(CodeData& code)
{
    do {
        data.push_back(code);
        switch (static_cast<DataEnum>(code.code())) {
        case SubclassMarker:
            break;
        case Thickness:
            thickness = code;
            break;
        case CenterPointX:
            centerPoint.rx() = code;
            break;
        case CenterPointY:
            centerPoint.ry() = code;
            break;
        case CenterPointZ:
            break;
        case Radius:
            radius = code;
            break;
        case StartAngle:
            startAngle = code;
            break;
        case EndAngle:
            endAngle = code;
            break;
        case ExtrusionDirectionX:
            break;
        case ExtrusionDirectionY:
            break;
        case ExtrusionDirectionZ:
            break;
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

Entity::Type Arc::type() const { return Type::ARC; }

GraphicObject Arc::toGo() const
{
    if (qFuzzyIsNull(radius) || (qFuzzyCompare(startAngle, endAngle)))
        return {};

    double aspan = endAngle - startAngle;

    if (const bool ccw = endAngle > startAngle;
        endAngle >= 0 && startAngle >= 0) {
        if (!ccw)
            aspan += 360;
    } else {
        if (aspan < -180 || (qFuzzyCompare(aspan, -180) && !ccw))
            aspan += 360;
        else if (aspan > 180 || (qFuzzyCompare(aspan, 180) && ccw))
            aspan -= 360;
    }

    QPainterPath path;
    path.moveTo(QLineF::fromPolar(radius, -startAngle).translated(centerPoint).p2());
    QPointF rad(radius, radius);
    path.arcTo(QRectF(centerPoint - rad, centerPoint + rad), -startAngle, -aspan);

#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    QMatrix m;
    m.scale(u, u);
    QPainterPath path2;
    for (auto& poly : path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QMatrix m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2).first());
#else
    QTransform m;
    m.scale(u, u);
    QPainterPath path2;
    for (auto& poly : path.toSubpathPolygons(m))
        path2.addPolygon(poly);
    QTransform m2;
    m2.scale(d, d);
    auto p(path2.toSubpathPolygons(m2).first());
#endif

    return { this, p, {} };
}

void Arc::write(QDataStream& stream) const
{
    stream << centerPoint;
    stream << thickness;
    stream << radius;
    stream << startAngle;
    stream << endAngle;
}

void Arc::read(QDataStream& stream)
{
    stream >> centerPoint;
    stream >> thickness;
    stream >> radius;
    stream >> startAngle;
    stream >> endAngle;
}

}

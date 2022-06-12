// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_ellipse.h"

#include "dxf_insert.h"
#include <QGraphicsEllipseItem>

#include "section/dxf_blocks.h"
#include "section/dxf_entities.h"

#include <QPainter>
#include <qmath.h>

namespace Dxf {
Ellipse::Ellipse(SectionParser* sp)
    : Entity(sp) {
}

// void Ellipse::draw(const InsertEntity* const i) const
//{
//     if (i) {
//         //        for (int r = 0; r < i->rowCount; ++r) {
//         //            for (int c = 0; c < i->colCount; ++c) {
//         //                QPointF tr(r * i->rowSpacing, r * i->colSpacing);
//         //                QPointF rad(radius, radius);
//         //                auto item = scene->addEllipse(QRectF(centerPoint - rad, centerPoint + rad), QPen(i->color(), thickness /*, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin*/), Qt::NoBrush);
//         //                item->setToolTip(layerName);
//         //                i->transform(item, tr);
//         //                i->attachToLayer(item);
//         //            }
//         //        }
//     } else {
//         QPainterPath path;
//         QLineF line(CenterPoint, EndpointOfMajorAxis);
//         const double angle = line.angle();
//         const double length = line.length() / 2;
//         //        const auto angle = qRadiansToDegrees(std::atan2(EndpointOfMajorAxis.y() - CenterPoint.y(), EndpointOfMajorAxis.x() - CenterPoint.x()));

//        double aspan = qRadiansToDegrees(endParameter) - qRadiansToDegrees(startParameter);
//        const bool ccw = endParameter > startParameter;
//        if (endParameter >= 0 && startParameter >= 0) {
//            if (!ccw)
//                aspan += 360;
//        } else {
//            if (aspan < -180 || (qFuzzyCompare(aspan, -180) && !ccw))
//                aspan += 360;
//            else if (aspan > 180 || (qFuzzyCompare(aspan, 180) && ccw))
//                aspan -= 360;
//        }
//        QPointF rad(length, length * ratioOfMinorAxisToMajorAxis);
//        path.arcTo(QRectF(-rad, +rad), -qRadiansToDegrees(startParameter), -aspan);
//        //        auto item(new QGraphicsPathItem(path));
//        //        item->setPen(QPen(color(), 0.0));
//        //        item->setBrush(Qt::NoBrush);
//        //        item->setRotation(angle);
//        //        item->setPos(CenterPoint);
//        //        path.arcTo(QRectF(CenterPoint - rad, CenterPoint + rad), -qRadiansToDegrees(startParameter), -aspan);
//        //        attachToLayer(scene->addPath(path, QPen(color(), 0.0), Qt::NoBrush));
//        //        scene->addItem(item);
//        //        attachToLayer(item);
//        auto item = scene->addEllipse(QRectF(-rad, +rad), QPen(color(), 0.0), Qt::NoBrush);
//        item->setStartAngle(-qRadiansToDegrees(startParameter) * 16);
//        item->setSpanAngle(-qRadiansToDegrees(aspan) * 16);
//        item->setRotation(angle);
//        item->setPos(CenterPoint);
//        attachToLayer(item);
//    }
//}

void Ellipse::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch (static_cast<DataEnum>(code.code())) {
        case SubclassMarker: // 100
            break;           //	100	Маркер подкласса (AcDbEllipse)

        case CenterPointX: // 10
            CenterPoint.setX(code);
            break;         //	10	Центральная точка (в МСК)
        case CenterPointY: // 20
            CenterPoint.setY(code);
            break;         //		Файл DXF: значение X; приложение: 3D-точка
        case CenterPointZ: // 30
            break;         //	20, 30	Файл DXF: значения Y и Z для центральной точки (в МСК)

        case EndpointOfMajorAxisX: // 11
            EndpointOfMajorAxis.setX(code);
            break;                 //	11	Конечная точка главной оси относительно центральной точки (в МСК)
        case EndpointOfMajorAxisY: // 21
            EndpointOfMajorAxis.setY(code);
            break;                 //		Файл DXF: значение X; приложение: 3D-точка
        case EndpointOfMajorAxisZ: // 31
            break;                 //	21, 31	Файл DXF: значения Y и Z для конечной точки главной оси относительно центральной точки (в МСК)

        case ExtrusionDirectionX: // 210
        case ExtrusionDirectionY: // 220
        case ExtrusionDirectionZ: // 230
            break;

        case RatioOfMinorAxisToMajorAxis: // 40
            ratioOfMinorAxisToMajorAxis = code;
            break; //	40	Соотношение малой и главной осей

        case StartParameter: // 41
            startParameter = code;
            break;         //	41	Начальный параметр (значение для полного эллипса — 0,0)
        case EndParameter: // 42
            endParameter = code;
            break; //	42	Конечный параметр (значение для полного эллипса — 2 пи)
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while (code.code() != 0);
}

Entity::Type Ellipse::type() const { return Entity::ELLIPSE; }

GraphicObject Ellipse::toGo() const { return {}; }

void Ellipse::write(QDataStream& stream) const {
    stream << startParameter;
    stream << endParameter;
    stream << ratioOfMinorAxisToMajorAxis;
    stream << CenterPoint;
    stream << EndpointOfMajorAxis;
}

void Ellipse::read(QDataStream& stream) {
    stream >> startParameter;
    stream >> endParameter;
    stream >> ratioOfMinorAxisToMajorAxis;
    stream >> CenterPoint;
    stream >> EndpointOfMajorAxis;
}

} // namespace Dxf

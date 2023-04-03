/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include <any>
#include <myclipper.h>

// struct Circle {
//     QPointF center;
//     double radius;
// };
// struct Ellipse {
//     QPointF center;
//     QPointF focus;
// };
// struct ArcCircle {
//     QPointF center;
//     double radius;
//     double angle1;
//     double angle2;
// };
// struct ArcEllipse {
//     QPointF center;
//     QPointF focus;
//     double angle1;
//     double angle2;
// };

// struct Polygon {
//     QPolygonF points;
//     bool open{};
// };

// struct Rectangle {
//     QPointF p1;
//     QPointF p2;
//     double angle;
// };

struct Transform {
    double angle {};
    QPointF translate {};
    QPointF scale {1, 1};

    friend QDataStream& operator<<(QDataStream& stream, const Transform& tr) {
        return Block(stream).write(tr);
    }
    friend QDataStream& operator>>(QDataStream& stream, Transform& tr) {
        return Block(stream).read(tr);
    }

    operator QTransform() const {
        QTransform t;
        t.translate(translate.x(), translate.y());
        t.rotate(angle);
        t.scale(scale.x(), scale.y());
        return t;
    }
};

struct GraphicObject {
    // clang-format off
    enum Type {
        Null,
        Arc       = 0b0'0000'0000'0000'0001, // 1
        Circle    = 0b0'0000'0000'0000'0010, // 2
        Elipse    = 0b0'0000'0000'0000'0100, // 3
        Line      = 0b0'0000'0000'0000'1000, // 4
        PolyLine  = 0b0'0000'0000'0001'0000, // 5
        Polygon   = 0b0'0000'0000'0010'0000, // 6
        Rect      = 0b0'0000'0000'0100'0000, // 7
        Square    = 0b0'0000'0000'1000'0000, // 8
        Text      = 0b0'0000'0001'0000'0000, // 9
        Composite = 0b0'0000'0010'0000'0000, // 10
        Dummy2    = 0b0'0000'0100'0000'0000, // 11
        Dummy3    = 0b0'0000'1000'0000'0000, // 12
        Dummy4    = 0b0'0001'0000'0000'0000, // 13
        Dummy6    = 0b0'0010'0000'0000'0000, // 14
        Dummy7    = 0b0'0100'0000'0000'0000, // 15
        Stamp     = 0b0'1000'0000'0000'0000,
        Drawn     = 0b1'0000'0000'0000'0000,
    };
    // clang-format on

    Paths fill;
    Path path;
    Point pos {std::numeric_limits<Point::Type>::lowest(), std::numeric_limits<Point::Type>::lowest()};
    QByteArray name;
    Type type {Null};
    int32_t id {-1};
    std::any raw;

    bool closed() const { return path.size() > 2 && path.front() == path.back(); }
    bool positive() const { return Clipper2Lib::IsPositive(path); }
};

enum Side {
    NullSide = -1,
    Top,
    Bottom
};

struct LayerType {
    int32_t id = -1;
    QString actName;
    QString actToolTip;
    QString shortActName() const { return actName; }

    friend QDataStream& operator<<(QDataStream& stream, const LayerType& layer) {
        return Block(stream).write(layer);
    }
    friend QDataStream& operator>>(QDataStream& stream, LayerType& layer) {
        return Block(stream).read(layer);
    }
};

Q_DECLARE_METATYPE(LayerType)

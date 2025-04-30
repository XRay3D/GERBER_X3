/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2025                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "datastream.h"
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
    double angle{};
    QPointF translate{};
    QPointF scale{1, 1};

    friend QDataStream& operator<<(QDataStream& stream, const Transform& tr) {
        return Block{stream}.write(tr);
    }
    friend QDataStream& operator>>(QDataStream& stream, Transform& tr) {
        return Block{stream}.read(tr);
    }

    QTransform toQTransform() const {
        QTransform t;
        t.translate(translate.x(), translate.y());
        t.rotate(angle);
        t.scale(scale.x(), scale.y());
        return t;
    }
    operator QTransform() const { return toQTransform(); }
};

enum class GCType {
    Null,
    Drill,
    Pocket,
    Profile,
    Thermal
};

struct GraphicObject {

    friend QDataStream& operator<<(QDataStream& stream, const GraphicObject& go) {
        return Block{stream}.write(go.id, go.type, go.pos, go.path, go.fill, go.name);
    }

    friend QDataStream& operator>>(QDataStream& stream, GraphicObject& go) {
        return Block{stream}.read(go.id, go.type, go.pos, go.path, go.fill, go.name);
    }

    // clang-format off
    enum Type:uint32_t {
        Null,
        Arc       , // 1
        Circle    , // 2
        Elipse    , // 3
        Line      , // 4
        PolyLine  , // 5
        Polygon   , // 6
        Rect      , // 7
        Square    , // 8
        Text      , // 9
        Composite , // 10
        Dummy2    , // 11
        Dummy3    , // 12
        Dummy4    , // 13
        Dummy6    , // 14
        Dummy7    , // 15
        FlStamp   = 0b01'0000'0000, // штамп по xy
        FlDrawn   = 0b10'0000'0000, // рисование по xy ... xNyN

    };
    // clang-format on

    Paths fill;
    Path path;
    Point pos{std::numeric_limits</*Point::Type*/ int32_t>::lowest(), std::numeric_limits</*Point::Type*/ int32_t>::lowest()};
    QByteArray name;
    Type type{Null};
    int32_t id{-1};
    std::any raw;

    inline bool isType(uint32_t t) const { return (t & 0xFF) ? (type & 0xFF) == (t & 0xFF) : true; }
    inline bool isFlags(uint32_t f) const { return (f & ~0xFF) ? (type & ~0xFF) & f : true; }
    inline bool test(uint32_t t) const { return isType(t) && isFlags(t); }
    inline bool closed() const { return path.size() > 2 && path.front() == path.back(); }
    bool positive() const { return Clipper2Lib::IsPositive(path); }
};

inline GraphicObject operator*(GraphicObject go, const QTransform& t) {
    for(auto& path: go.fill)
        path = ~t.map(~path);
    go.path = ~t.map(~go.path);
    go.pos = ~t.map(~go.pos);
    return go;
}

struct Range {
    double min{-std::numeric_limits<double>::max()};
    double max{+std::numeric_limits<double>::max()};
    bool operator()(double val) const { return min <= val && val <= max; }
    inline bool isNull() const {
        return min == -std::numeric_limits<double>::max()
            && max == +std::numeric_limits<double>::max();
    }
};

struct Criteria {
    std::vector<GraphicObject::Type> types;
    Range area{};
    Range length{};
    bool positiveOnly{}; /// NOTE
    bool test(const GraphicObject& go) const {
        bool fl{};
        for(auto type: types)
            if((fl = go.test(type)))
                break;
        if(fl && !length.isNull())
            fl &= length(Clipper2Lib::Length(go.path));
        if(fl && !area.isNull())
            fl &= area(Clipper2Lib::Area(go.fill));
        return fl;
    }
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
        return Block{stream}.write(layer);
    }
    friend QDataStream& operator>>(QDataStream& stream, LayerType& layer) {
        return Block{stream}.read(layer);
    }
};

Q_DECLARE_METATYPE(LayerType)

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

struct AbstrGraphicObject {
    Paths paths_;
    AbstrGraphicObject(const Paths& paths_)
        : paths_ {paths_} { }
    virtual ~AbstrGraphicObject() { }

    virtual Path line() const = 0;  //{ return {}; }
    virtual Path lineW() const = 0; //{ return {}; } // closed

    virtual Path polyLine() const = 0;   //{ return {}; }
    virtual Paths polyLineW() const = 0; //{ return {}; } // closed

    virtual Path elipse() const = 0;   //{ return {}; } // circle
    virtual Paths elipseW() const = 0; //{ return {}; }

    virtual Path arc() const = 0;  //{ return {}; } // part of elipse
    virtual Path arcW() const = 0; //{ return {}; }

    virtual Path polygon() const = 0;        //{ return {}; }
    virtual Paths polygonWholes() const = 0; //{ return {}; }

    virtual Path hole() const = 0;   //{ return {}; }
    virtual Paths holes() const = 0; //{ return {}; }

    virtual bool positive() const = 0; //{ return {}; } // not hole
    virtual bool closed() const = 0;   //{ return {}; } // front == back

    virtual const Path& path() const = 0;   //{ return {}; }
    virtual const Paths& paths() const = 0; //{ return {}; }

    virtual Path& rPath() = 0;
    virtual Paths& rPaths() = 0;
};

enum Side {
    NullSide = -1,
    Top,
    Bottom
};

struct LayerType {
    int id = -1;
    QString actName;
    QString actToolTip;
    QString shortActName() const { return actName; }

    friend QDataStream& operator<<(QDataStream& stream, const LayerType& layer) {
        return stream << layer.id << layer.actName << layer.actToolTip;
    }

    friend QDataStream& operator>>(QDataStream& stream, LayerType& layer) {
        return stream >> layer.id >> layer.actName >> layer.actToolTip;
    }
};

Q_DECLARE_METATYPE(LayerType)

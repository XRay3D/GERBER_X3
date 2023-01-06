/*******************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  03 October 2022                                                 *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
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
    PathsD paths_;
    AbstrGraphicObject(const PathsD& paths_)
        : paths_ {paths_} { }
    virtual ~AbstrGraphicObject() { }

    virtual PathD line() const = 0;  //{ return {}; }
    virtual PathD lineW() const = 0; //{ return {}; } // closed

    virtual PathD polyLine() const = 0;   //{ return {}; }
    virtual PathsD polyLineW() const = 0; //{ return {}; } // closed

    virtual PathD elipse() const = 0;   //{ return {}; } // circle
    virtual PathsD elipseW() const = 0; //{ return {}; }

    virtual PathD arc() const = 0;  //{ return {}; } // part of elipse
    virtual PathD arcW() const = 0; //{ return {}; }

    virtual PathD polygon() const = 0;        //{ return {}; }
    virtual PathsD polygonWholes() const = 0; //{ return {}; }

    virtual PathD hole() const = 0;   //{ return {}; }
    virtual PathsD holes() const = 0; //{ return {}; }

    virtual bool positive() const = 0; //{ return {}; } // not hole
    virtual bool closed() const = 0;   //{ return {}; } // front == back

    virtual const PathD& path() const = 0;   //{ return {}; }
    virtual const PathsD& paths() const = 0; //{ return {}; }

    virtual PathD& rPath() = 0;
    virtual PathsD& rPaths() = 0;
};

enum class FileType {
    Gerber,
    Excellon,
    GCode,
    Dxf,
    Hpgl,
    TopoR,
    Shapes = 100
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
};

Q_DECLARE_METATYPE(LayerType)

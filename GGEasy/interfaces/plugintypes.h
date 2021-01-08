/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "myclipper.h"

struct GraphicObject {
    GraphicObject() { }
    virtual ~GraphicObject() { }

    virtual Path line() const { return {}; }
    virtual Path lineW() const { return {}; } // closed

    virtual Path polyLine() const { return {}; }
    virtual Paths polyLineW() const { return {}; } // closed

    virtual Path elipse() const { return {}; } // circle
    virtual Paths elipseW() const { return {}; }

    virtual Path arc() const { return {}; } // part of elipse
    virtual Path arcW() const { return {}; }

    virtual Path polygon() const { return {}; }
    virtual Paths polygonWholes() const { return {}; }

    virtual Path hole() const { return {}; }
    virtual Paths holes() const { return {}; }

    virtual bool positive() const { return {}; } // not hole
    virtual bool closed() const { return {}; } // front == back
};

enum class FileType {
    Gerber,
    Excellon,
    GCode,
    Dxf,
    Shapes
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

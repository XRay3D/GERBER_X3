/*******************************************************************************
*                                                                              *
* Author    :  Bakiev Damir                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Bakiev Damir 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#include "datastream.h"
#include <QVariant>
#include <QVector>
#include <tooldatabase/tool.h>

using UsedItems = QMap<QPair<int, int>, QVector<int>>;

namespace GCode {

enum GCodeType {
    Null = -1,
    Profile,
    Pocket,
    Voronoi,
    Thermal,
    Drill,
    Raster,
    LaserHLDI,
    GCodeProperties = 100,
};

enum Code {
    GNull = -1,
    G00 = 0,
    G01 = 1,
    G02 = 2,
    G03 = 3,
};

enum SideOfMilling {
    On,
    Outer,
    Inner,
};

enum Direction {
    Climb,
    Conventional
};

enum Grouping {
    CopperPaths,
    CutoffPaths,
};

struct GCodeParams {
    enum Param {
        UseAngle, // need for Raster and LaserHLDI
        Depth,
        Pass, // need for Raster and LaserHLDI profile
        UseRaster,
        Steps, // need for Pocket
        Tolerance, // need for Voronoi
        Width, // need for Voronoi
        VorT, // need for Voronoi
        FileId,
        FrameOffset, // need for Voronoi
        AccDistance, // need for LaserHLDI
        Side,
        Convent,
        Fast,
        PocketIndex, // need for Pocket
        GrItems,
        Node,
        Bridges, // need for Profile
        BridgeLen, // need for Profile
        NotTile,
        Strip,
        IgnoreCopper // need for Thermal
    };

    GCodeParams() { }
    GCodeParams(const Tool& tool, double depth, GCodeType type)
    {
        tools.append(tool);
        params[GCodeParams::Depth] = depth;
        gcType = type;
    }

    QVector<Tool> tools;
    QMap<int, QVariant> params;
    GCodeType gcType = Null;
    mutable int fileId = -1;

    friend QDataStream& operator>>(QDataStream& stream, GCodeParams& type)
    {
        qRegisterMetaTypeStreamOperators<UsedItems>("QMap<QPair<int, int>, QVector<int>>");
        qRegisterMetaTypeStreamOperators<QVector<QPointF>>("QVector<QPointF>");
        stream >> type.tools;
        stream >> type.params;
        stream >> type.gcType;
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const GCodeParams& type)
    {
        qRegisterMetaTypeStreamOperators<UsedItems>("QMap<QPair<int, int>, QVector<int>>");
        qRegisterMetaTypeStreamOperators<QVector<QPointF>>("QVector<QPointF>");
        stream << type.tools;
        stream << type.params;
        stream << type.gcType;
        return stream;
    }

    const Tool& getTool() const { return tools[params.value(PocketIndex, int {}).toInt()]; }

    SideOfMilling side() const { return static_cast<SideOfMilling>(params[Side].toInt()); }
    bool convent() const { return params[Convent].toBool(); }
    double getToolDiameter() const { return tools.at(params[PocketIndex].toInt()).getDiameter(params[Depth].toInt()); }
    double getDepth() const { return params.value(Depth).toDouble(); }

    void setSide(SideOfMilling val) { params[Side] = val; }
    void setConvent(bool val) { params[Convent] = val; }
};
}

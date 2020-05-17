#pragma once
#ifndef GC_TYPES_H
#define GC_TYPES_H

#include <QVariant>
#include <datastream.h>
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
    GCodeProperties,
    Raster,
    LaserHLDI
};

enum Code {
    G_null = -1,
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
        UseAngle,
        Depth,
        Pass,
        UseRaster,
        Steps,
        Tolerance,
        Width,
        VorT,
        FileId,
        FrameOffset,
        AccDistance,
        Side,
        Convent,
        Fast,
        PocketIndex,
        GrItems,
        Node,
        Bridges,
        BridgeLen,
        NotTile
    };

    GCodeParams() {}
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

#endif // GC_TYPES_H

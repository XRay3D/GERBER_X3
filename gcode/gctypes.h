#ifndef GCVARS_H
#define GCVARS_H

#include <tooldatabase/tool.h>

#include <QVariant>

namespace GCode {

enum GCodeType {
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

enum Param {
    UseAngle,
    Depth,
    Pass,
    UseRaster,
    Steps,
    Tolerance,
    TwoTools,
    Width,
    VorT,
    FileId,
    MinArea,
    FrameOffset,
    AccDistance,
    Side,
    Convent,
    Fast
};

struct GCodeParams {
    QVector<Tool> tool;
    QMap<int, QVariant> dParam;

    SideOfMilling side() const { return static_cast<SideOfMilling>(dParam[Side].toInt()); }
    bool convent() const { return dParam[Convent].toBool(); }

    void setSide(SideOfMilling val) { dParam[Side] = val; }
    void setConvent(bool val) { dParam[Convent] = val; }
};
}

#endif // GCVARS_H

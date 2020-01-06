#ifndef GCVARS_H
#define GCVARS_H

#include <tooldatabase/tool.h>

namespace GCode {

enum GCodeType {
    Profile,
    Pocket,
    Voronoi,
    Thermal,
    Drill,
    GCodeProperties,
    Raster
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
};

struct GCodeParams {
    QMap<int, QVector<int>> src;
    QVector<Tool> tool;
    SideOfMilling side;
    bool convent;
    double dParam[20];
    //    double angle;
    //    double depth;
    //    double tolerance;
    //    double width;
    //    int dst;
    //    int pPass;
    //    int steps;
    //    int type;
};
}

#endif // GCVARS_H

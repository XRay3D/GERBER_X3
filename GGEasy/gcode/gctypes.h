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

#include "datastream.h"
#include "tool.h"

#include "mvector.h"
#include <QVariant>

using UsedItems = QMap<QPair<int, int>, mvector<int>>;

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

struct variant : std::variant<int, double, UsedItems> {
    friend QDataStream& operator>>(QDataStream& stream, variant& type)
    {
        size_t index;
        stream >> index;
        switch (index) {
        case 0:
            type = int {};
            break;
        case 1:
            type = double {};
            break;
        case 2:
            type = UsedItems {};
            break;
        }
        std::visit([&stream](auto&& val) { stream >> val; }, type);
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const variant& type)
    {
        stream << type.index();
        std::visit([&stream](auto&& val) { stream << val; }, type);
        return stream;
    }
    variant() { }

    template <class T>
    variant(const T& val)
    {
        *this = val;
    }

    template <class T>
    variant& operator=(const T& val)
    {
        using To = std::decay_t<T>;
        if constexpr /**/ (std::is_integral_v<To>) {
            *this = int(val);
        } else if constexpr (std::is_floating_point_v<To>) {
            *this = double(val);
        } else if constexpr (std::is_same_v<To, UsedItems>) {
            *this = val;
        }
        return *this;
    }

    int toInt() const { return std::get<int>(*this); }

    bool toBool() const { return std::get<int>(*this); }

    double toDouble() const { return std::get<double>(*this); }

    template <class T>
    void setValue(const T& val) { *this = val; }

    template <class T>
    void setValue(T&& val) { *this = val; }

    template <class T>
    auto value() const { return std::get<std::decay_t<T>>(*this); }
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
        tools.push_back(tool);
        params[GCodeParams::Depth] = depth;
        gcType = type;
    }

    mvector<Tool> tools;
    QMap<int, variant> params;
    GCodeType gcType = Null;
    mutable int fileId = -1;

    friend QDataStream& operator>>(QDataStream& stream, GCodeParams& type)
    {
        qRegisterMetaTypeStreamOperators<UsedItems>("QMap<QPair<int, int>, mvector<int>>");
        //        qRegisterMetaTypeStreamOperators<mvector<QPointF>>("mvector<QPointF>");
        stream >> type.tools;
        stream >> type.params;
        stream >> type.gcType;
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const GCodeParams& type)
    {
        qRegisterMetaTypeStreamOperators<UsedItems>("QMap<QPair<int, int>, mvector<int>>");
        //        qRegisterMetaTypeStreamOperators<mvector<QPointF>>("mvector<QPointF>");
        stream << type.tools;
        stream << type.params;
        stream << type.gcType;
        return stream;
    }

    const Tool& getTool() const { return tools[params.value(PocketIndex, 0).toInt()]; }

    SideOfMilling side() const { return static_cast<SideOfMilling>(params[Side].toInt()); }
    bool convent() const { return params[Convent].toBool(); }
    double getToolDiameter() const { return tools.at(params[PocketIndex].toInt()).getDiameter(params[Depth].toInt()); }
    double getDepth() const { return params.value(Depth).toDouble(); }

    void setSide(SideOfMilling val) { params[Side] = val; }
    void setConvent(bool val) { params[Convent] = val; }
};

class Settings {
protected:
    static inline QString m_fileExtension = { "tap" };
    static inline QString m_format { "G?X?Y?Z?F?S?" };
    static inline QString m_laserConstOn { "M3" };
    static inline QString m_laserDynamOn { "M4" };
    static inline QString m_spindleLaserOff { "M5" };
    static inline QString m_spindleOn { "M3" };

    static inline QString m_start { "G21 G17 G90\nM3 S?" };
    static inline QString m_end { "M5\nM30" };

    static inline QString m_laserStart { "G21 G17 G90" };
    static inline QString m_laserEnd { "M30" };

    static inline bool m_info { true };
    static inline bool m_sameFolder { true };

public:
    static QString fileExtension() { return m_fileExtension; }
    static QString format() { return m_format; }
    static QString laserConstOn() { return m_laserConstOn; }
    static QString laserDynamOn() { return m_laserDynamOn; }

    static QString spindleLaserOff() { return m_spindleLaserOff; }
    static QString spindleOn() { return m_spindleOn; }

    static QString laserStart() { return m_laserStart; }
    static QString laserEnd() { return m_laserEnd; }

    static QString start() { return m_start; }
    static QString end() { return m_end; }

    static bool info() { return m_info; }
    static bool sameFolder() { return m_sameFolder; }
};

}

/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  14 January 2021                                                 *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include "datastream.h"
#include <tool.h>

#include "mvector.h"
#include <QDebug>
#include <QVariant>
#include <variant>

using UsedItems = std::map<std::pair<int, int>, std::vector<int>>;
using V = std::variant<int, double, UsedItems>;

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
    Hatching,
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

struct variant {
    V var;
    friend QDataStream& operator>>(QDataStream& stream, variant& v)
    {
        uint8_t index;
        stream >> index;
        switch (index) {
        case 0:
            v = int {};
            break;
        case 1:
            v = double {};
            break;
        case 2:
            v = UsedItems {};
            break;
        }
        std::visit([&stream](auto&& val) { stream >> val; }, v.var);
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const variant& v)
    {
        stream << uint8_t(v.index());
        std::visit([&stream](auto&& val) { stream << val; }, v.var);
        return stream;
    }
    //    variant() = default;
    //    variant(variant&&) = default;
    //    variant(const variant&) = default;
    //    variant& operator=(variant&&) = default;
    //    variant& operator=(const variant&) = default;
    variant() { }

    template <class T>
    variant(const T& val)
        : var(val)
    {
    }

    //    template <class T>
    //    variant(T&& val)
    //        : var(std::move(val))
    //    {
    //    }

    size_t index() const { return var.index(); }

    int toInt() const
    {
        return std::visit([](auto&& val) -> int {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, UsedItems>) {
                return int{};
            } else {
                return int(val);
            } }, var);
        //        try {
        //            if(index())
        //            return std::get<int>(*this);
        //        } catch (const std::exception& e) {
        //            //qDebug() << e.what();
        //            return {};
        //        }
    }

    bool toBool() const
    {
        return std::visit([](auto&& val) -> bool {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, UsedItems>) {
                return bool{};
            } else {
                return bool(val);
            } }, var);
        //        try {
        //            return std::get<int>(*this);
        //        } catch (const std::exception& e) {
        //            //qDebug() <<
        //            e.what();
        //            return {};
        //        }
    }

    double toDouble() const
    {
        return std::visit([](auto&& val) -> double {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, UsedItems>) {
                return {};
            } else {
                return double(val);
            } }, var);
        //        try {
        //            return std::get<double>(*this);
        //        } catch (const std::exception& e) {
        //            //qDebug() << e.what();
        //            return {};
        //        }
    }

    template <class T>
    void setValue(const T& val) { var = val; }

    template <class T>
    void setValue(T&& val) { var = val; }

    template <class T>
    decltype(auto) value() const { return std::get<std::decay_t<T>>(var); }
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
        Trimming,
        CornerTrimming,
        IgnoreCopper, // need for Thermal
        HathStep // need for Hatching
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
        //qRegisterMetaTypeStreamOperators<UsedItems>("QMap<std::pair<int, int>, mvector<int>>");
        //        qRegisterMetaTypeStreamOperators<mvector<QPointF>>("mvector<QPointF>");
        stream >> type.tools;
        stream >> type.params;
        stream >> type.gcType;
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const GCodeParams& type)
    {
        //qRegisterMetaTypeStreamOperators<UsedItems>("QMap<std::pair<int, int>, mvector<int>>");
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
    static inline QString m_formatMilling { "G?X?Y?Z?F?S?" };
    static inline QString m_formatLaser { "G?X?Y?Z?F?S?" };
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

    static inline int m_profileSort = 0;

public:
    static QString fileExtension() { return m_fileExtension; }
    static QString formatMilling() { return m_formatMilling; }
    static QString formatLaser() { return m_formatLaser; }
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

    static int profileSort() { return m_profileSort; }
};

}

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
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
    Raster,
    Hatching,
    Voronoi,
    Thermal,
    Drill,
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

struct Variant : V {
    using V::V;

    friend QDataStream& operator>>(QDataStream& stream, V& v) {
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
        std::visit([&stream](auto&& val) { stream >> val; }, v);
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const V& v) {
        stream << uint8_t(v.index());
        std::visit([&stream](auto&& val) { stream << val; }, v);
        return stream;
    }

    Variant() { }

    template <class T>
    Variant(const T& val)
        : V(val) { }

    int toInt() const {
        return std::visit([](auto&& val) -> int {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, UsedItems>) {
                return int{};
            } else {
                return int(val);
            } }, (V&)*this);
    }

    bool toBool() const {
        return std::visit([](auto&& val) -> bool {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, UsedItems>) {
                return bool{};
            } else {
                return bool(val);
            } }, (V&)*this);
    }

    double toDouble() const {
        return std::visit([](auto&& val) -> double {
            using T = std::decay_t<decltype(val)>;
            if constexpr (std::is_same_v<T, UsedItems>) {
                return {};
            } else {
                return double(val);
            } }, (V&)*this);
    }

    template <class T>
    void setValue(const T& val) { *this = val; }

    template <class T>
    void setValue(T&& val) { *this = val; }

    template <class T>
    decltype(auto) value() const { return std::get<std::decay_t<T>>(*this); }
};

struct GCodeParams {
    Q_GADGET
public:
    enum Param {
        UseAngle, // need for Raster and LaserHLDI
        Depth,
        Pass, // need for Raster and LaserHLDI profile
        UseRaster,
        Steps,     // need for Pocket
        Tolerance, // need for Voronoi
        Width,     // need for Voronoi
        VorT,      // need for Voronoi
        FileId,
        FrameOffset, // need for Voronoi
        AccDistance, // need for LaserHLDI
        Side,
        Convent,
        Fast,
        PocketIndex, // need for Pocket
        GrItems,
        Node,
        Bridges,   // need for Profile
        BridgeLen, // need for Profile
        NotTile,
        Trimming,
        CornerTrimming,
        IgnoreCopper, // need for Thermal
        HathStep      // need for Hatching
    };
    Q_ENUM(Param)

    GCodeParams() {
        if (!params.contains(PocketIndex))
            params[PocketIndex] = 0;
    }

    GCodeParams(const Tool& tool, double depth, GCodeType type)
        : GCodeParams {} {
        tools.emplace_back(tool);
        params[GCodeParams::Depth] = depth;
        gcType = type;
    }

    mvector<Tool> tools;
    std::map<int, Variant> params;
    GCodeType gcType = Null;
    mutable int fileId = -1;

    friend QDataStream& operator>>(QDataStream& stream, GCodeParams& type) {
        stream >> type.tools;
        stream >> type.params;
        stream >> type.gcType;
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const GCodeParams& type) {
        stream << type.tools;
        stream << type.params;
        stream << type.gcType;
        return stream;
    }

    const Tool& getTool() const { return tools[params.at(PocketIndex).toInt()]; }

    SideOfMilling side() const { return static_cast<SideOfMilling>(params.at(Side).toInt()); }
    bool convent() const { return params.at(Convent).toBool(); }
    double getToolDiameter() const { return tools.at(params.at(PocketIndex).toInt()).getDiameter(params.at(Depth).toInt()); }
    double getDepth() const { return params.at(Depth).toDouble(); }

    void setSide(SideOfMilling val) { params[Side] = val; }
    void setConvent(bool val) { params[Convent] = val; }
};

class Settings {
protected:
    static inline QString fileExtension_ {"tap"};
    static inline QString formatMilling_ {"G?X?Y?Z?F?S?"};
    static inline QString formatLaser_ {"G?X?Y?Z?F?S?"};
    static inline QString laserConstOn_ {"M3"};
    static inline QString laserDynamOn_ {"M4"};
    static inline QString spindleLaserOff_ {"M5"};
    static inline QString spindleOn_ {"M3"};

    static inline QString start_ {"G21 G17 G90\nM3 S?"};
    static inline QString end_ {"M5\nM30"};

    static inline QString laserStart_ {"G21 G17 G90"};
    static inline QString laserEnd_ {"M30"};

    static inline bool info_ {true};
    static inline bool sameFolder_ {true};

    static inline bool simplifyHldi_ {false};

    static inline int profileSort_ = 0;

public:
    static QString fileExtension() { return fileExtension_; }
    static QString formatMilling() { return formatMilling_; }
    static QString formatLaser() { return formatLaser_; }
    static QString laserConstOn() { return laserConstOn_; }
    static QString laserDynamOn() { return laserDynamOn_; }

    static QString spindleLaserOff() { return spindleLaserOff_; }
    static QString spindleOn() { return spindleOn_; }

    static QString laserStart() { return laserStart_; }
    static QString laserEnd() { return laserEnd_; }

    static QString start() { return start_; }
    static QString end() { return end_; }

    static bool info() { return info_; }
    static bool sameFolder() { return sameFolder_; }

    static bool simplifyHldi() { return simplifyHldi_; }

    static int profileSort() { return profileSort_; }
};

} // namespace GCode

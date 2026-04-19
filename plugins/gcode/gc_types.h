/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "curve.h"
#include "datastream.h"
#include "md5.h"
#include "myclipper.h"

#include "tool.h"

#include <QColor>
#include <QDebug>
#include <QVariant>
#include <variant>

constexpr auto G_CODE = "GCode"_hash32;
constexpr auto GC_DBG_FILE = "GCDbgFile"_hash32;

namespace GCode {

// enum GCodeType : int {
// Null = -1,

// Profile = 100, // FileType::GCode
// Pocket,
// Raster,
// Hatching,
// Voronoi,
// Thermal,
// Drill,
// LaserHLDI,

// GCodeProperties = 199,
//};

enum Code {
    GNull = -1,
    G00 = 0,
    G01 = 1,
    G02 = 2, // cw
    G03 = 3, // ccw
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

enum class Grouping {
    Copper,
    Cutoff,
};

using UsedItems = std::map<std::vector<int32_t>, std::vector<int32_t>>;

using V = std::variant<qsizetype, double, UsedItems>;

struct Variant : V {
    using V::V;

    friend QDataStream& operator>>(QDataStream& stream, V& v) {
        uint8_t index;
        stream >> index;
        switch(index) {
        case 0: stream >> v.emplace<0>(); break;
        case 1: stream >> v.emplace<1>(); break;
        case 2: stream >> v.emplace<2>(); break;
        }
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const V& v) {
        stream << uint8_t(v.index());
        std::visit([&stream](auto&& val) { stream << val; }, v);
        return stream;
    }

    qsizetype toInt() const {
        return std::visit([](auto&& val) -> int {
            using T = std::decay_t<decltype(val)>;
            if constexpr(std::is_same_v<T, UsedItems>)
                return int{};
            else
                return int(val);
        },
            (V&)*this);
    }

    size_t toUInt() const {
        return std::visit([](auto&& val) -> size_t {
            using T = std::decay_t<decltype(val)>;
            if constexpr(std::is_same_v<T, UsedItems>)
                return size_t{};
            else
                return size_t(val);
        },
            (V&)*this);
    }

    bool toBool() const {
        return std::visit([](auto&& val) -> bool {
            using T = std::decay_t<decltype(val)>;
            if constexpr(std::is_same_v<T, UsedItems>)
                return bool{};
            else
                return bool(val);
        },
            (V&)*this);
    }

    double toDouble() const {
        return std::visit([](auto&& val) -> double {
            using T = std::decay_t<decltype(val)>;
            if constexpr(std::is_same_v<T, UsedItems>)
                return {};
            else
                return double(val);
        },
            (V&)*this);
    }

    template <class T>
        requires std::is_enum_v<T>
    operator T() const { return static_cast<T>(toInt()); }

    template <std::unsigned_integral T>
    operator T() const { return static_cast<T>(toUInt()); }

    template <std::integral T>
    operator T() const { return static_cast<T>(toInt()); }

    template <std::floating_point T>
    operator T() const { return static_cast<T>(toDouble()); }

    template <class T>
    void setValue(const T& val) { *this = val; }

    template <class T>
    void setValue(T&& val) { *this = val; }

    template <class T>
    decltype(auto) value() const { return std::get<std::decay_t<T>>(*this); }
};

struct Params {
    Q_GADGET
public:
    enum Param : uint32_t {
        // Node,
        // AccDistance, // need for LaserHLDI
        // BridgeLen, // need for Profile
        // Bridges,   // need for Profile
        // CornerTrimming, // need for Profile
        // Fast,
        // FileId,
        // FrameOffset, // need for Voronoi
        // HathStep      // need for Hatching
        // IgnoreCopper, // need for Thermal
        // Pass, // need for Raster and LaserHLDI profile
        // Steps,     // need for Pocket
        // Tolerance, // need for Voronoi
        // Trimming,       // need for Profile
        // UseAngle, // need for Raster and LaserHLDI
        // UseRaster,
        // VorT,      // need for Voronoi
        // Width,     // need for Voronoi
        Convent,
        Depth,
        GrItems,
        MultiToolIndex, // need for Pocket
        NotTile,        // не раскладывать если даже раскладка включена
        Side,
        FileSide,

        UserParam = 100
    };

    Q_ENUM(Param)

    Params() {
        if(!params.contains(MultiToolIndex)) params[MultiToolIndex] = 0;
    }

    Params(const Tool& tool, double depth)
        : Params{} {
        tools.emplace_back(tool);
        params[Params::Depth] = depth;
    }

    Params(const Tool& tool, double depth, Paths&& toolPaths)
        : Params{tool, depth} {
        supportCurvess.emplace_back(toCurves(toolPaths));
    }

    Params(const Tool& tool, double depth, Curves&& toolPaths)
        : Params{tool, depth} {
        supportCurvess.emplace_back(std::move(toolPaths));
    }

    mvector<Tool> tools;
    std::map<std::underlying_type_t<Param>, Variant> params;

    // GCodeType gcType = Null;
    mutable int fileId = -1;
    // QColor color;

    friend QDataStream& operator>>(QDataStream& stream, Params& par) {
        return stream >> par.tools
            >> par.params
            >> par.closedCurves
            >> par.supportCurvess;
    }

    friend QDataStream& operator<<(QDataStream& stream, const Params& par) {
        return stream << par.tools
                      << par.params
                      << par.closedCurves
                      << par.supportCurvess;
    }

    explicit operator bool() const {
        return openCurves.size() || closedCurves.size();
    }

    const Tool& tool() const { return tools[params.at(MultiToolIndex).toInt()]; }

    SideOfMilling side() const { return static_cast<SideOfMilling>(params.at(Side).toInt()); }
    bool convent() const { return params.at(Convent).toBool(); }
    double getToolDiameter() const { return tools.at(params.at(MultiToolIndex).toInt()).getDiameter(params.at(Depth).toInt()); }
    double getDepth() const { return params.at(Depth).toDouble(); }

    void setSide(SideOfMilling val) { params[Side] = val; }
    void setConvent(bool val) { params[Convent] = val; }

    Curves closedCurves; // pocketAreaPaths
    Curves openCurves;
    Curvess supportCurvess; // toolCurvess
    Curvess toolPathss;     // toolCurvess

    const Curves& pocketAreaCurves() const { return closedCurves; }
    void setPocketAreaCurves(Curves&& arg) { closedCurves = std::move(arg); }

    auto feedRate() const -> double { return tool().feedRate(); }
    auto plungeRate() const -> double { return tool().plungeRate(); }
    auto spindleSpeed() const -> int { return tool().spindleSpeed(); }
    auto toolType() const -> int { return tool().type(); }
};

class Settings {
    // protected:
public:
    /*static inline*/ QString fileExtension_{u"tap"_s};
    /*static inline*/ QString formatMilling_{u"G?X?Y?I+J+Z?F?S?"_s};
    /*static inline*/ QString formatLaser_{u"G?X?Y?I+J+Z?F?S?"_s};
    /*static inline*/ QString laserConstOn_{u"M3"_s};
    /*static inline*/ QString laserDynamOn_{u"M4"_s};
    /*static inline*/ QString spindleLaserOff_{u"M5"_s};
    /*static inline*/ QString spindleOn_{u"M3"_s};

    /*static inline*/ QString start_{u"G21 G17 G90\nM3 S?"_s};
    /*static inline*/ QString end_{u"M5\nM30"_s};

    /*static inline*/ QString laserStart_{u"G21 G17 G90"_s};
    /*static inline*/ QString laserEnd_{u"M30"_s};

    /*static inline*/ bool info_{true};
    /*static inline*/ bool sameFolder_{true};

public:
    /*static*/ QString fileExtension() { return fileExtension_; }
    /*static*/ QString formatMilling() { return formatMilling_; }
    /*static*/ QString formatLaser() { return formatLaser_; }
    /*static*/ QString laserConstOn() { return laserConstOn_; }
    /*static*/ QString laserDynamOn() { return laserDynamOn_; }

    /*static*/ QString spindleLaserOff() { return spindleLaserOff_; }
    /*static*/ QString spindleOn() { return spindleOn_; }

    /*static*/ QString laserStart() { return laserStart_; }
    /*static*/ QString laserEnd() { return laserEnd_; }

    /*static*/ QString start() { return start_; }
    /*static*/ QString end() { return end_; }

    /*static*/ bool info() { return info_; }
    /*static*/ bool sameFolder() { return sameFolder_; }
};

} // namespace GCode

Q_DECLARE_METATYPE(GCode::Params*)

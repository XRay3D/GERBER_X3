/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "datastream.h"
#include "md5.h"
#include "mvector.h"
#include "tool.h"

#include <QColor>
#include <QDebug>
#include <QVariant>
#include <variant>

constexpr auto G_CODE = md5::hash32("GCode");

namespace GCode {

// enum GCodeType : int {
//     Null = -1,

//    Profile = 100, // FileType::GCode
//    Pocket,
//    Raster,
//    Hatching,
//    Voronoi,
//    Thermal,
//    Drill,
//    LaserHLDI,

//    GCodeProperties = 199,
//};

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

enum class Grouping {
    Copper,
    Cutoff,
};

using UsedItems = std::map<std::pair<int, int>, std::vector<int>>;
using V = std::variant<int, double, UsedItems>;

struct Variant : V {
    using V::V;

    friend QDataStream& operator>>(QDataStream& stream, V& v) {
        uint8_t index;
        stream >> index;
        using Init = V& (*)(V&);
        static std::unordered_map<uint8_t, Init> map {
            {0,       [](V& v) -> V& { return v = int {}; }},
            {1,    [](V& v) -> V& { return v = double {}; }},
            {2, [](V& v) -> V& { return v = UsedItems {}; }},
        };
        std::visit([&stream](auto&& val) { stream >> val; }, map[index](v));
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

struct Params {
    Q_GADGET
public:
    enum Param : uint32_t {
        //        Node,
        //        AccDistance, // need for LaserHLDI
        //        BridgeLen, // need for Profile
        //        Bridges,   // need for Profile
        //        CornerTrimming, // need for Profile
        //        Fast,
        //        FileId,
        //        FrameOffset, // need for Voronoi
        //        HathStep      // need for Hatching
        //        IgnoreCopper, // need for Thermal
        //        Pass, // need for Raster and LaserHLDI profile
        //        Steps,     // need for Pocket
        //        Tolerance, // need for Voronoi
        //        Trimming,       // need for Profile
        //        UseAngle, // need for Raster and LaserHLDI
        //        UseRaster,
        //        VorT,      // need for Voronoi
        //        Width,     // need for Voronoi
        Convent,
        Depth,
        GrItems,
        MultiToolIndex, // need for Pocket
        NotTile,        // не раскладывать если даже раскладка включена
        Side,

        UserParam = 100
    };

    Q_ENUM(Param)

    Params() {
        if (!params.contains(MultiToolIndex))
            params[MultiToolIndex] = 0;
    }

    Params(const Tool& tool, double depth /*, uint32_t type*/)
        : Params {} {
        tools.emplace_back(tool);
        params[Params::Depth] = depth;
        //        gcType = type;
    }

    mvector<Tool> tools;
    std::map<std::underlying_type_t<Param>, Variant> params;
    //    GCodeType gcType = Null;
    mutable int fileId = -1;
    //    QColor color;

    friend QDataStream& operator>>(QDataStream& stream, Params& type) {
        stream >> type.tools;
        stream >> type.params;
        //        stream >> type.gcType;
        return stream;
    }

    friend QDataStream& operator<<(QDataStream& stream, const Params& type) {
        stream << type.tools;
        stream << type.params;
        //        stream << type.gcType;
        return stream;
    }

    const Tool& getTool() const { return tools[params.at(MultiToolIndex).toInt()]; }

    SideOfMilling side() const { return static_cast<SideOfMilling>(params.at(Side).toInt()); }
    bool convent() const { return params.at(Convent).toBool(); }
    double getToolDiameter() const { return tools.at(params.at(MultiToolIndex).toInt()).getDiameter(params.at(Depth).toInt()); }
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

class cancelException : public std::exception {
public:
    cancelException(const char* description)
        : m_descr(description) {
    }
    ~cancelException() noexcept override = default;
    const char* what() const noexcept override { return m_descr.c_str(); }

private:
    std::string m_descr;
};

class ProgressCancel {
    static inline int max_;
    static inline int current_;
    static inline bool cancel_;

public:
    static void reset() {
        current_ = 0;
        max_ = 0;
        cancel_ = 0;
    }

    /////////////////
    /// \brief Progress max
    /// \return
    ///
    static int max() { return max_; }
    /////////////////
    /// \brief Progress setMax
    /// \param max
    ///
    static void setMax(int max) { max_ = max; }

    /////////////////
    /// \brief Progress current
    /// \return
    ///
    static int current() { return current_; }
    /////////////////
    /// \brief Progress setCurrent
    /// \param current
    ///
    static void setCurrent(int current = 0) { current_ = current; }
    /////////////////
    /// \brief Progress incCurrent
    ///
    static void incCurrent() { ++current_; }
    static bool isCancel() { return cancel_; }
    static void ifCancelThenThrow(/*const sl location = sl::current()*/) {
        ++current_;
        if (cancel_) [[unlikely]] {
            //            static std::stringstream ss;
            //            ss.clear();
            //            ss << "file: "
            //               << location.file_name() << "("
            //               << location.line() << ":"
            //               << location.column() << ") `"
            //               << location.function_name();
            //            throw cancelException(ss.str().data() /*__FUNCTION__*/);
            throw cancelException(__FUNCTION__);
        }
    }
    static void setCancel(bool cancel) { cancel_ = cancel; }
};

} // namespace GCode

inline void ifCancelThenThrow() { GCode::ProgressCancel::ifCancelThenThrow(); }

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

#include "datastream.h"
#include "geo/polygon.h"
#include "md5.h"

#include "tool.h"

#include <QColor>
#include <QDebug>
#include <QMap>
#include <QVariant>
#include <algorithm>
#include <cmath>
#include <limits>
#include <variant>

constexpr auto G_CODE      = "GCode"_hash32;
constexpr auto GC_DBG_FILE = "GCDbgFile"_hash32;

namespace GCode {

// Координата в УП -- ЦЕЛОЕ число единиц вывода, а не double и не текст.
//
// Единица -- Geo::exitWeldTolerance, тот самый допуск, на котором точная
// геометрия считает две точки одной. Мельче писать нечего (для домена этой
// разницы нет), грубее нельзя (разница есть, а в тексте её не видно -- и
// целый ход пропадает молча). Один и тот же порог с обеих сторон -- это и
// значит, что сетка вывода и сетка геометрии совпадают.
//
// Тип -- int: при шаге 0.1 мкм это +-214 метра, чего станку хватит с любым
// запасом, а габарит УП, в него не влезающий, ловится заранее (fitsOutput).
using Units = int;

inline constexpr Units unitsPerMm = static_cast<Units>(1.0 / Geo::exitWeldTolerance + 0.5);
static_assert(unitsPerMm >= 10 && 1.0 / unitsPerMm == Geo::exitWeldTolerance,
    "единица вывода должна быть ровно допуском сварки и степенью десяти");

// Число дробных знаков, которые несёт единица, -- ими и печатается число.
inline constexpr int unitDecimals = [] {
    int decimals{};
    for(Units units = unitsPerMm; units > 1; units /= 10) ++decimals;
    return decimals;
}();

inline Units toUnits(double mm) { return static_cast<Units>(std::llround(mm * unitsPerMm)); }
inline double fromUnits(Units units) { return static_cast<double>(units) / unitsPerMm; }

// Влезет ли габарит в вывод. Проверяется ДО счёта: координата, не влезшая в
// Units, переполнилась бы молча и уехала на другой конец стола.
inline bool fitsOutput(const QRectF& rect) {
    constexpr double limit = std::numeric_limits<Units>::max() / static_cast<double>(unitsPerMm);
    for(double value: {rect.left(), rect.right(), rect.top(), rect.bottom()})
        if(!(std::abs(value) < limit)) return false; // NaN сюда же
    return true;
}

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
    G00   = 0,
    G01   = 1,
    G02   = 2, // cw
    G03   = 3, // ccw
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
public:
    // Parameter keys are names, not an enum: each plugin can add its own
    // (see e.g. Profile::Creator::BridgeLen) without needing a shared numeric
    // range to avoid collisions, and a serialized/dumped Params reads as text.
    static inline const QString Convent        = u"Convent"_s;
    static inline const QString Depth          = u"Depth"_s;
    static inline const QString GrItems        = u"GrItems"_s;
    static inline const QString MultiToolIndex = u"MultiToolIndex"_s; // need for Pocket
    static inline const QString NotTile        = u"NotTile"_s;        // не раскладывать если даже раскладка включена
    static inline const QString Side           = u"Side"_s;
    static inline const QString FileSide       = u"FileSide"_s;
    static inline const QString LeftHand       = u"LeftHand"_s; // need for Threading
    static inline const QString Circle         = u"Circle"_s;   // need for Threading
    static inline const QString Chamfer        = u"Chamfer"_s;  // need for Threading
    static inline const QString Starts         = u"Starts"_s;   // need for Threading
    // Спиральное врезание. Ключ общий, а не плагинный: читает его
    // File::saveMillingProfile, который живёт здесь же, в общем слое.
    static inline const QString SpiralRamp = u"SpiralRamp"_s;

    Params() {
        if(!params.contains(MultiToolIndex)) params[MultiToolIndex] = 0;
    }

    Params(const Tool& tool, double depth)
        : Params{} {
        tools.emplace_back(tool);
        params[Params::Depth] = depth;
    }

    // Params(const Tool& tool, double depth, Paths&& toolPaths)
    //     : Params{tool, depth} {
    //     toolPathss /*supportCurvess*/.emplace_back(toCurves(toolPaths));
    // }

    Params(const Tool& tool, double depth, Geo::Polylines&& toolPaths)
        : Params{tool, depth} {
        toolPathss.emplace_back(std::move(toolPaths));
    }

    std::vector<Tool> tools;
    std::map<QString, Variant> params;

    // GCodeType gcType = Null;
    mutable int fileId = -1;
    // QColor color;

    // toolPathss и openCurves сохраняются наравне с исходной геометрией. Без них
    // у загруженной из проекта УП regenerate() вычищает текст до заголовка с
    // концовкой: генератор берёт траекторию именно из toolPathss. А regenerate()
    // зовётся не только из save(), но и при смене стороны платы в дереве -- то
    // есть УП молча превращалась в пустую от одного щелчка по «Сторона».
    friend QDataStream& operator>>(QDataStream& stream, Params& par) {
        return stream >> par.tools
            >> par.params
            >> par.closedCurves
            >> par.supportCurvess
            >> par.toolPathss
            >> par.openCurves;
    }

    friend QDataStream& operator<<(QDataStream& stream, const Params& par) {
        return stream << par.tools
                      << par.params
                      << par.closedCurves
                      << par.supportCurvess
                      << par.toolPathss
                      << par.openCurves;
    }

    explicit operator bool() const {
        return !openCurves.empty() || !closedCurves.empty();
    }

    const Tool& tool() const { return tools[params.at(MultiToolIndex).toInt()]; }

    SideOfMilling side() const { return static_cast<SideOfMilling>(params.at(Side).toInt()); }
    bool convent() const { return params.at(Convent).toBool(); }
    double getToolDiameter() const { return tools.at(params.at(MultiToolIndex).toInt()).getDiameter(params.at(Depth).toInt()); }
    double getDepth() const { return params.at(Depth).toDouble(); }

    bool leftHand() const { return params.contains(LeftHand) && params.at(LeftHand).toBool(); }
    bool circle() const { return params.contains(Circle) && params.at(Circle).toBool(); }
    bool chamfer() const { return params.contains(Chamfer) && params.at(Chamfer).toBool(); }
    int starts() const { return params.contains(Starts) ? std::max(1, static_cast<int>(params.at(Starts).toInt())) : 1; }
    // Отсутствие ключа == спираль включена: проекты, сохранённые до её
    // появления, ведут себя как раньше.
    bool spiralRamp() const { return !params.contains(SpiralRamp) || params.at(SpiralRamp).toBool(); }

    // Надо ли развернуть контур, пришедший из точного домена, чтобы обход стал
    // тем, что просит пользователь.
    //
    // Geo::Polygons::contours() отдаёт канон: внешняя граница против часовой,
    // дырки по часовой -- то есть регион ВСЕГДА слева по ходу. Фрезеровать же
    // надо попутно или встречно, а это зависит ещё и от стороны: снаружи
    // детали попутный ход -- один обход, внутри -- обратный.
    //
    // Правило это для контуров, построенных ОТ ДЕТАЛИ (Profile::Creator::
    // orderContours). Петлям кармана оно не годится: те приходят границами
    // области, которую фреза выбирает, и сторону материала выражает уже сама
    // их ориентация -- см. Creator::stacking.
    //
    // Живёт здесь, а не в плагине, потому что от направления обхода зависят
    // вещи, которые ломаются МОЛЧА: подрезка углов
    // ищет внутренние стыки по 90 или 270 градусам, и стоит развернуть обход
    // в одном месте, забыв про другое, -- она просто перестаёт что-либо
    // находить, без единой жалобы.
    bool reversedTravel() const { return (side() == Outer) ^ convent(); }

    void setSide(SideOfMilling val) { params[Side] = val; }
    void setConvent(bool val) { params[Convent] = val; }
    void setLeftHand(bool val) { params[LeftHand] = val; }
    void setCircle(bool val) { params[Circle] = val; }
    void setChamfer(bool val) { params[Chamfer] = val; }
    void setStarts(int val) { params[Starts] = val; }

    // Замкнутое -- регион в точном домене: над ним и идут булевы с офсетом.
    // Открытое -- просто набор линий, площади у них нет.
    Geo::Polygons closedCurves; // pocketAreaPaths
    Geo::Polylines openCurves;
    // Траектории -- наборы линий, сгруппированные по проходам, а не регионы.
    std::vector<Geo::Polylines> supportCurvess;
    std::vector<Geo::Polylines> toolPathss;

    const Geo::Polygons& pocketAreaCurves() const { return closedCurves; }
    void setPocketAreaCurves(Geo::Polygons&& arg) { closedCurves = std::move(arg); }

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

    QMap<QString, QString> scriptPaths_; // gcName -> JS script file path

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

    QString scriptPath(const QString& gcName) const { return scriptPaths_.value(gcName); }
    void setScriptPath(const QString& gcName, const QString& path) { scriptPaths_[gcName] = path; }
};

} // namespace GCode

Q_DECLARE_METATYPE(GCode::Params*)

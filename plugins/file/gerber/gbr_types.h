/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once

#include "datastream.h"
#include "md5.h"
#include "plugintypes.h"

#include <QDebug>
#include <QObject>
#include <myclipper.h>

#define DEPRECATED

namespace Gerber {

constexpr auto GERBER = md5::hash32("Gerber");

class GbrObj : public QObject {
    Q_OBJECT
public:
    GbrObj() { }
    virtual ~GbrObj() { }
};

class AbstractAperture;

using ApertureMap = std::map<int, std::shared_ptr<AbstractAperture>>;

enum ZeroOmissionMode {
    OmitLeadingZeros,
#ifdef DEPRECATED
    OmitTrailingZeros,
#endif
};

enum UnitMode {
    Inches,
    Millimeters,
};

enum ImagePolarity {
    Positive,
    Negative,
};

enum CoordinateValuesNotation {
    AbsoluteNotation,
#ifdef DEPRECATED
    IncrementalNotation,
#endif
};

enum InterpolationMode {
    Linear = 1,
    ClockwiseCircular = 2,
    CounterClockwiseCircular = 3
};

enum RegionMode {
    Off,
    On
};

enum QuadrantMode {
    Undef,
    Single,
    Multi
};

enum Operation {
    D01 = 1,
    D02 = 2,
    D03 = 3
};

enum Layer {
    Assy,
    Silk,
    Paste,
    Mask,
    Copper,
    Board,
};

enum Mirroring {
    NoMirroring,
    X_Mirroring,
    Y_Mirroring,
    XY_Mirroring,
};

enum GCode {
    // Graphics state operators defining the interpolateparameter defining the interpolate operation (D01).
    G01 = 1, // Sets the interpolation mode tolinear
    G02 = 2, // Sets the interpolation mode to ‘Clockwise circular interpolation’
    G03 = 3, // Sets the interpolation mode to ‘Counterclockwise circularinterpolation’
    // Comment
    G04 = 4,
    // Region mode
    G36 = 36, // Begin region
    G37 = 37, // End region
#ifdef DEPRECATED
    // Change aperture
    G54 = 54,
    // Units mode
    G70 = 70, // Inces
    G71 = 71, // Millimeteres
#endif
    // Graphics state operators defining the quadrant modeparameter, amodifier of the circular interpolation mode.
    G74 = 74, // Sets quadrant mode to ’Singlequadrant’
    G75 = 75, // Sets quadrant mode to ’Multiquadrant’
#ifdef DEPRECATED
    // Absolute / relative coordinates
    G90 = 90, // Absolute
    G91 = 91, // Relative (incremental)
#endif
};

enum PrimitiveType {
    Aperture,
    Line,
    Region,
};

class File;

struct Format {
    UnitMode unitMode = Millimeters;

    // Warning: Trailing zero omission is deprecated
    ZeroOmissionMode zeroOmisMode = OmitLeadingZeros;

    // Warning: Currently the only allowed notation is absolute notation. The incremental notation is deprecated.
    CoordinateValuesNotation coordValueNotation = AbsoluteNotation;

    // Warning: Using less than 4 decimal places is deprecated.
    enum {
        IntegerDefVal = 3,
        DecimalDefVal = 4
    };

    int xInteger = IntegerDefVal;
    int xDecimal = DecimalDefVal;
    int yInteger = IntegerDefVal;
    int yDecimal = DecimalDefVal;

    friend QDataStream& operator<<(QDataStream& stream, const Format& format) {
        return ::Block(stream).write(format);
    }
    friend QDataStream& operator>>(QDataStream& stream, Format& format) {
        return ::Block(stream).read(format);
    }
};

class State {
    friend class File;
    friend QDataStream& operator<<(QDataStream& stream, const State& state) {
        return ::Block(stream).write(
            state.dCode_,
            state.gCode_,
            state.imgPolarity_,
            state.interpolation_,
            state.type_,
            state.quadrant_,
            state.region_,
            state.aperture_,
            state.lineNum_,
            state.curPos_,
            state.mirroring_,
            state.scaling_,
            state.rotating_);
    }

    friend QDataStream& operator>>(QDataStream& stream, State& state) {
        return ::Block(stream).read(
            state.dCode_,
            state.gCode_,
            state.imgPolarity_,
            state.interpolation_,
            state.type_,
            state.quadrant_,
            state.region_,
            state.aperture_,
            state.lineNum_,
            state.curPos_,
            state.mirroring_,
            state.scaling_,
            state.rotating_);
    }

    File* file_ = nullptr;
    Operation dCode_ = D02;
    GCode gCode_ = G01;
    ImagePolarity imgPolarity_ = Positive;
    InterpolationMode interpolation_ = Linear;
    PrimitiveType type_ = Aperture;
    QuadrantMode quadrant_ = Undef;
    RegionMode region_ = Off;
    int aperture_ = 0;
    int lineNum_ = 0;
    Point curPos_;
    Mirroring mirroring_ = NoMirroring;
    double scaling_ = 1.0;
    double rotating_ = 0.0;

public:
    State(File* const file = nullptr)
        : file_{file} { }

    inline auto file() const { return file_; }

    inline auto dCode() const { return dCode_; }
    inline void setDCode(Operation dCode) { dCode_ = dCode; }

    inline auto gCode() const { return gCode_; }
    inline void setGCode(GCode gCode) { gCode_ = gCode; }

    inline auto imgPolarity() const { return imgPolarity_; }
    inline void setImgPolarity(ImagePolarity imgPolarity) { imgPolarity_ = imgPolarity; }

    inline auto interpolation() const { return interpolation_; }
    inline void setInterpolation(InterpolationMode interpolation) { interpolation_ = interpolation; }

    inline auto type() const { return type_; }
    inline void setType(PrimitiveType type) { type_ = type; }

    inline auto quadrant() const { return quadrant_; }
    inline void setQuadrant(QuadrantMode quadrant) { quadrant_ = quadrant; }

    inline auto region() const { return region_; }
    inline void setRegion(RegionMode region) { region_ = region; }

    inline auto aperture() const { return aperture_; }
    inline void setAperture(int aperture) { aperture_ = aperture; }

    inline auto& curPos() { return curPos_; }
    inline auto curPos() const { return curPos_; }
    inline void setCurPos(const Point& curPos) { curPos_ = curPos; }

    inline auto mirroring() const { return mirroring_; }
    inline void setMirroring(Mirroring mirroring) { mirroring_ = mirroring; }

    inline auto scaling() const { return scaling_; }
    inline void setScaling(double scaling) { scaling_ = scaling; }

    inline auto rotating() const { return rotating_; }
    inline void setRotating(double rotating) { rotating_ = rotating; }
};

struct GrObject : public GraphicObject {

    friend QDataStream& operator<<(QDataStream& stream, const GrObject& go) {
        return ::Block(stream).write(go.path, go.fill, go.state, go.type, go.name, go.pos);
    }

    friend QDataStream& operator>>(QDataStream& stream, GrObject& go) {
        return ::Block(stream).read(go.path, go.fill, go.state, go.type, go.name, go.pos);
    }

    File* gFile{nullptr};
    State state;

    // public:
    GrObject() = default;
    GrObject(int32_t id, const State& state, Paths&& paths, File* gFile, Type type, Path&& path = {})
        : gFile{gFile}
        , state{state} {
        GraphicObject::id = id;
        GraphicObject::fill = std::move(paths);
        GraphicObject::path = std::move(path);
        GraphicObject::type = type;
    }
};

struct StepRepeatStr {
    void reset() {
        x = 0;
        y = 0;
        i = 0.0;
        j = 0.0;
        storage.clear();
    }
    int x{};
    int y{};
    double i{};
    double j{};
    QList<GrObject> storage;
};

class Settings {
protected:
    static inline bool cleanPolygons_;
    static inline double cleanPolygonsDist_ = 0.0005;

    static inline bool simplifyRegions_;
    static inline bool skipDuplicates_;

    static inline bool wireMinkowskiSum_;

public:
    static bool cleanPolygons() { return cleanPolygons_; }
    static double cleanPolygonsDist() { return cleanPolygonsDist_; }

    static bool simplifyRegions() { return simplifyRegions_; }
    static bool skipDuplicates() { return skipDuplicates_; }

    static bool wireMinkowskiSum() { return wireMinkowskiSum_; }
};

} // namespace Gerber

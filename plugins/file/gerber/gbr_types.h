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
#include "plugintypes.h"
#include <QDebug>
#include <myclipper.h>

#define DEPRECATED

namespace Gerber {

class GbrObj : public QObject {
    Q_OBJECT
public:
    GbrObj() { }
    virtual ~GbrObj() { }
};

class AbstractAperture;

#if __cplusplus > 201703L
using ApertureMap = std::map<int, std::shared_ptr<AbstractAperture>>;

#else
struct ApertureMap : std::map<int, std::shared_ptr<AbstractAperture>> {
    using M = std::map<int, std::shared_ptr<AbstractAperture>>;
    bool contains(int key) const { return find(key) != end(); }
    M& map() { return *this; }
    const M& map() const { return *this; }
};
struct VarMap : std::map<QString, double> {
    using M = std::map<QString, double>;
    bool contains(const QString& key) const { return find(key) != end(); }
};
#endif

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
    CounterclockwiseCircular = 3
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
        stream.writeRawData(reinterpret_cast<const char*>(&format), sizeof(Format));
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Format& format) {
        stream.readRawData(reinterpret_cast<char*>(&format), sizeof(Format));
        return stream;
    }
};

class State {
    friend class File;
    friend QDataStream& operator<<(QDataStream& stream, const State& state) {
        stream << state.dCode_;
        stream << state.gCode_;
        stream << state.imgPolarity_;
        stream << state.interpolation_;
        stream << state.type_;
        stream << state.quadrant_;
        stream << state.region_;
        stream << state.aperture_;
        stream << state.lineNum_;
        stream << state.curPos_;
        stream << state.mirroring_;
        stream << state.scaling_;
        stream << state.rotating_;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, State& state) {
        stream >> state.dCode_;
        stream >> state.gCode_;
        stream >> state.imgPolarity_;
        stream >> state.interpolation_;
        stream >> state.type_;
        stream >> state.quadrant_;
        stream >> state.region_;
        stream >> state.aperture_;
        stream >> state.lineNum_;
        stream >> state.curPos_;
        stream >> state.mirroring_;
        stream >> state.scaling_;
        stream >> state.rotating_;
        return stream;
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
    IntPoint curPos_;
    Mirroring mirroring_ = NoMirroring;
    double scaling_ = 1.0;
    double rotating_ = 0.0;

public:
    State(File* const file = nullptr)
        : file_(file)
        , dCode_(D02)
        , gCode_(G01)
        , imgPolarity_(Positive)
        , interpolation_(Linear)
        , type_(Aperture)
        , quadrant_(Undef)
        , region_(Off)
        , aperture_(0)
        , lineNum_(0)
        , curPos_(IntPoint())
        , mirroring_(NoMirroring)
        , scaling_(1.0)
        , rotating_(0.0) {
    }

    inline File* file() const { return file_; }

    inline Operation dCode() const { return dCode_; }
    inline void setDCode(Operation dCode) { dCode_ = dCode; }

    inline GCode gCode() const { return gCode_; }
    inline void setGCode(GCode gCode) { gCode_ = gCode; }

    inline ImagePolarity imgPolarity() const { return imgPolarity_; }
    inline void setImgPolarity(ImagePolarity imgPolarity) { imgPolarity_ = imgPolarity; }

    inline InterpolationMode interpolation() const { return interpolation_; }
    inline void setInterpolation(InterpolationMode interpolation) { interpolation_ = interpolation; }

    inline PrimitiveType type() const { return type_; }
    inline void setType(PrimitiveType type) { type_ = type; }

    inline QuadrantMode quadrant() const { return quadrant_; }
    inline void setQuadrant(QuadrantMode quadrant) { quadrant_ = quadrant; }

    inline RegionMode region() const { return region_; }
    inline void setRegion(RegionMode region) { region_ = region; }

    inline int aperture() const { return aperture_; }
    inline void setAperture(int aperture) { aperture_ = aperture; }

    inline IntPoint& curPos() { return curPos_; }
    inline IntPoint curPos() const { return curPos_; }
    inline void setCurPos(const IntPoint& curPos) { curPos_ = curPos; }

    inline Mirroring mirroring() const { return mirroring_; }
    inline void setMirroring(Mirroring mirroring) { mirroring_ = mirroring; }

    inline double scaling() const { return scaling_; }
    inline void setScaling(double scaling) { scaling_ = scaling; }

    inline double rotating() const { return rotating_; }
    inline void setRotating(double rotating) { rotating_ = rotating; }
};

class GraphicObject final : public AbstrGraphicObject {
    friend class File;
    friend class Plugin;
    friend QDataStream& operator<<(QDataStream& stream, const GraphicObject& go) {
        stream << go.path_;
        stream << go.paths_;
        stream << go.state_;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, GraphicObject& go) {
        stream >> go.path_;
        stream >> go.paths_;
        stream >> go.state_;
        return stream;
    }

    File* gFile_;
    Path path_;
    State state_;

public:
    GraphicObject()
        : AbstrGraphicObject {{}}
        , gFile_(nullptr) {
    }
    GraphicObject(
        int /*id*/,
        const State& state,
        const Paths& paths,
        File* gFile,
        const Path& path = Path())
        : AbstrGraphicObject {paths}
        , gFile_(gFile)
        , path_(path)
        , state_(state) {
    }
    inline File* gFile() const { return gFile_; }
    inline State state() const { return state_; }

    inline Path& rPath() override { return path_; }
    inline Paths& rPaths() override { return paths_; }

    const Path& path() const override { return path_; }
    const Paths& paths() const override { return paths_; }

    Path line() const override { return path_.size() == 2 ? path_ : Path(); }
    Path lineW() const override { return path_.size() == 2 ? paths_.front() : Path(); } // polygon

    Path polyLine() const override { return closed() ? Path() : path_; }
    Paths polyLineW() const override { return closed() ? Paths() : paths_; } // closed

    Path elipse() const override;   // { return gFile_.; } // circle
    Paths elipseW() const override; // { return {}; }

    Path arc() const override { return {}; } // part of elipse
    Path arcW() const override { return {}; }

    Path polygon() const override { return state_.type() == Region ? path_ : Path(); }
    Paths polygonWholes() const override { return paths_; }

    Path hole() const override { return !positive() ? path_ : Path(); }
    Paths holes() const override { return !positive() ? Paths {paths_.front()} : paths_.mid(1); }

    bool positive() const override { return state_.imgPolarity() == Gerber::Positive; }                                                     // not hole
    bool closed() const override { return path_.size() ? path_.front() == path_.back() : paths_.front().front() == paths_.front().back(); } // front == back
};

struct StepRepeatStr {
    void reset() {
        x = 0;
        y = 0;
        i = 0.0;
        j = 0.0;
        storage.clear();
    }
    int x = 0;
    int y = 0;
    double i = 0.0;
    double j = 0.0;
    QList<GraphicObject> storage;
};

class Settings {
protected:
    static inline bool cleanPolygons_;
    static inline double cleanPolygonsDist_ = 0.0005;

    static inline bool simplifyRegions_;
    static inline bool skipDuplicates_;

public:
    static bool cleanPolygons() { return cleanPolygons_; }
    static double cleanPolygonsDist() { return cleanPolygonsDist_; }

    static bool simplifyRegions() { return simplifyRegions_; }
    static bool skipDuplicates() { return skipDuplicates_; }
};

} // namespace Gerber

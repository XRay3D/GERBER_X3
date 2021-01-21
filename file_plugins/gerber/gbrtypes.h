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
#include "interfaces/plugintypes.h"
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

#if _MSVC_LANG >= 201705L
using ApertureMap = std::map<int, std::shared_ptr<AbstractAperture>>;
#else
struct ApertureMap : std::map<int, std::shared_ptr<AbstractAperture>> {
    using M = std::map<int, std::shared_ptr<AbstractAperture>>;
    bool contains(int key) const { return find(key) != end(); }
    M& map() { return *this; }
    const M& map() const { return *this; }
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

    friend QDataStream& operator<<(QDataStream& stream, const Format& format)
    {
        stream.writeRawData(reinterpret_cast<const char*>(&format), sizeof(Format));
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, Format& format)
    {
        stream.readRawData(reinterpret_cast<char*>(&format), sizeof(Format));
        return stream;
    }
};

class State {
    friend class File;
    friend QDataStream& operator<<(QDataStream& stream, const State& state)
    {
        stream << state.m_dCode;
        stream << state.m_gCode;
        stream << state.m_imgPolarity;
        stream << state.m_interpolation;
        stream << state.m_type;
        stream << state.m_quadrant;
        stream << state.m_region;
        stream << state.m_aperture;
        stream << state.m_lineNum;
        stream << state.m_curPos;
        stream << state.m_mirroring;
        stream << state.m_scaling;
        stream << state.m_rotating;
        return stream;
    }

    friend QDataStream& operator>>(QDataStream& stream, State& state)
    {
        stream >> state.m_dCode;
        stream >> state.m_gCode;
        stream >> state.m_imgPolarity;
        stream >> state.m_interpolation;
        stream >> state.m_type;
        stream >> state.m_quadrant;
        stream >> state.m_region;
        stream >> state.m_aperture;
        stream >> state.m_lineNum;
        stream >> state.m_curPos;
        stream >> state.m_mirroring;
        stream >> state.m_scaling;
        stream >> state.m_rotating;
        return stream;
    }

    Format* m_format = nullptr;
    Operation m_dCode = D02;
    GCode m_gCode = G01;
    ImagePolarity m_imgPolarity = Positive;
    InterpolationMode m_interpolation = Linear;
    PrimitiveType m_type = Aperture;
    QuadrantMode m_quadrant = Undef;
    RegionMode m_region = Off;
    int m_aperture = 0;
    int m_lineNum = 0;
    IntPoint m_curPos;
    Mirroring m_mirroring = NoMirroring;
    double m_scaling = 1.0;
    double m_rotating = 0.0;

public:
    State(Format* const format = nullptr)
        : m_format(format)
        , m_dCode(D02)
        , m_gCode(G01)
        , m_imgPolarity(Positive)
        , m_interpolation(Linear)
        , m_type(Aperture)
        , m_quadrant(Undef)
        , m_region(Off)
        , m_aperture(0)
        , m_lineNum(0)
        , m_curPos(IntPoint())
        , m_mirroring(NoMirroring)
        , m_scaling(1.0)
        , m_rotating(0.0)
    {
    }

    inline Format* format() const { return m_format; }

    inline Operation dCode() const { return m_dCode; }
    inline void setDCode(Operation dCode) { m_dCode = dCode; }

    inline GCode gCode() const { return m_gCode; }
    inline void setGCode(GCode gCode) { m_gCode = gCode; }

    inline ImagePolarity imgPolarity() const { return m_imgPolarity; }
    inline void setImgPolarity(ImagePolarity imgPolarity) { m_imgPolarity = imgPolarity; }

    inline InterpolationMode interpolation() const { return m_interpolation; }
    inline void setInterpolation(InterpolationMode interpolation) { m_interpolation = interpolation; }

    inline PrimitiveType type() const { return m_type; }
    inline void setType(PrimitiveType type) { m_type = type; }

    inline QuadrantMode quadrant() const { return m_quadrant; }
    inline void setQuadrant(QuadrantMode quadrant) { m_quadrant = quadrant; }

    inline RegionMode region() const { return m_region; }
    inline void setRegion(RegionMode region) { m_region = region; }

    inline int aperture() const { return m_aperture; }
    inline void setAperture(int aperture) { m_aperture = aperture; }

    inline IntPoint& curPos() { return m_curPos; }
    inline IntPoint curPos() const { return m_curPos; }
    inline void setCurPos(const IntPoint& curPos) { m_curPos = curPos; }

    inline Mirroring mirroring() const { return m_mirroring; }
    inline void setMirroring(Mirroring mirroring) { m_mirroring = mirroring; }

    inline double scaling() const { return m_scaling; }
    inline void setScaling(double scaling) { m_scaling = scaling; }

    inline double rotating() const { return m_rotating; }
    inline void setRotating(double rotating) { m_rotating = rotating; }
};

class GraphicObject final : public AbstrGraphicObject {
    friend class File;
    friend class Plugin;
    friend QDataStream& operator<<(QDataStream& stream, const GraphicObject& go)
    {
        stream << go.m_path;
        stream << go.m_paths;
        stream << go.m_state;
        return stream;
    }
    friend QDataStream& operator>>(QDataStream& stream, GraphicObject& go)
    {
        stream >> go.m_path;
        stream >> go.m_paths;
        stream >> go.m_state;
        return stream;
    }

    File* m_gFile;
    Path m_path;
    Paths m_paths;
    State m_state;

public:
    GraphicObject()
        : m_gFile(nullptr)
    {
    }
    GraphicObject(
        int /*id*/,
        const State& state,
        const Paths& paths,
        File* gFile,
        const Path& path = Path())
        : m_gFile(gFile)
        , m_path(path)
        , m_paths(paths)
        , m_state(state)
    {
    }
    inline File* gFile() const { return m_gFile; }
    inline State state() const { return m_state; }

    inline Path& rPath() override { return m_path; }
    inline Paths& rPaths() override { return m_paths; }

    const Path& path() const override { return m_path; }
    const Paths& paths() const override { return m_paths; }

    Path line() const override { return m_path.size() == 2 ? m_path : Path(); }
    Path lineW() const override { return m_path.size() == 2 ? m_paths.front() : Path(); } // polygon

    Path polyLine() const override { return closed() ? Path() : m_path; }
    Paths polyLineW() const override { return closed() ? Paths() : m_paths; } // closed

    Path elipse() const override; // { return m_gFile.; } // circle
    Paths elipseW() const override; // { return {}; }

    Path arc() const override { return {}; } // part of elipse
    Path arcW() const override { return {}; }

    Path polygon() const override { return m_state.type() == Region ? m_path : Path(); }
    Paths polygonWholes() const override { return m_paths; }

    Path hole() const override { return !positive() ? m_path : Path(); }
    Paths holes() const override { return !positive() ? Paths({ m_paths.front() }) : m_paths.mid(1); }

    bool positive() const override { return m_state.imgPolarity() == Gerber::Positive; } // not hole
    bool closed() const override { return m_path.size()
            ? m_path.front() == m_path.back()
            : m_paths.front().front() == m_paths.front().back(); } // front == back
};

struct StepRepeatStr {
    void reset()
    {
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
    static inline bool m_cleanPolygons;
    static inline double m_cleanPolygonsDist = 0.0005;

    static inline bool m_simplifyRegions;
    static inline bool m_skipDuplicates;

public:
    static bool cleanPolygons() { return m_cleanPolygons; }
    static double cleanPolygonsDist() { return m_cleanPolygonsDist; }

    static bool simplifyRegions() { return m_simplifyRegions; }
    static bool skipDuplicates() { return m_skipDuplicates; }
};

} // namespace Gerber

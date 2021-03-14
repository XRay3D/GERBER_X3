// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
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
#include "gbraperture.h"
#include "mathparser.h"
#include <QDebug>
#include <QLineF>

namespace Gerber {

AbstractAperture::AbstractAperture(const Format* format)
    : m_format(format)
{
}

AbstractAperture::~AbstractAperture() {}

double AbstractAperture::drillDiameter() const
{
    return m_drillDiam;
}

Paths AbstractAperture::draw(const State& state, bool fl)
{
    if (state.dCode() == D03 && state.imgPolarity() == Positive && fl)
        m_isFlashed = true;
    if (m_paths.isEmpty())
        draw();

    Paths tmpPpaths(m_paths);

    for (Path& path : tmpPpaths) {
        if (state.imgPolarity() == Negative)
            ReversePath(path);
        if (m_format->unitMode == Inches && type() == Macro)
            for (IntPoint& pt : path) {
                pt.X *= 25.4;
                pt.Y *= 25.4;
            }

        transform(path, state);

        //if (state.curPos().X != 0 || state.curPos().Y != 0)
        TranslatePath(path, state.curPos());
    }
    //    Paths tmpPpaths;
    //    tmpPpaths.reserve(m_paths.size());

    //    for (Path path : m_paths) {
    //        if (state.imgPolarity() == Negative)
    //            ReversePath(path);
    //        if (m_format->unitMode == Inches && type() == Macro)
    //            for (IntPoint& pt : path) {
    //                pt.X *= 25.4;
    //                pt.Y *= 25.4;
    //            }

    //        transform(path, state);

    //        if (state.curPos().X != 0 || state.curPos().Y != 0)
    //            translate(path, state.curPos());
    //        tmpPpaths.push_back(path);
    //    }
    return tmpPpaths;
}

double AbstractAperture::minSize() const
{
    return m_size;
}

double AbstractAperture::apertureSize()
{
    if (m_paths.isEmpty())
        draw();
    return m_size;
}

Path AbstractAperture::drawDrill(const State& state)
{
    if (qFuzzyIsNull(m_drillDiam))
        return Path();

    Path drill = CirclePath(m_drillDiam * uScale);

    if (state.imgPolarity() == Positive)
        ReversePath(drill);

    TranslatePath(drill, state.curPos());
    return drill;
}

void AbstractAperture::transform(Path& poligon, const State& state)
{
    bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {

        if (state.mirroring() & X_Mirroring)
            pt.X = -pt.X;
        if (state.mirroring() & Y_Mirroring)
            pt.Y = -pt.Y;
        if (state.rotating() != 0.0 || state.scaling() != 1.0) {
            const double tmpAangle = qDegreesToRadians(state.rotating() - Angle(IntPoint(), pt));
            const double length = Length(IntPoint(), pt) * state.scaling();
            pt = IntPoint(static_cast<cInt>(qCos(tmpAangle) * length), static_cast<cInt>(qSin(tmpAangle) * length));
        }
    }

    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

/////////////////////////////////////////////////////
/// \brief ApCircle::ApCircle
/// \param diam
/// \param drillDiam
/// \param format
///
ApCircle::ApCircle(double diam, double drillDiam, const Format* format)
    : AbstractAperture(format)
{
    m_diam = diam;
    m_drillDiam = drillDiam;
    // GerberAperture interface
}

QString ApCircle::name() const { return QString("C(Ø%1)").arg(m_diam); } //CIRCLE

ApertureType ApCircle::type() const { return Circle; }

bool ApCircle::fit(double toolDiam) const { return m_diam > toolDiam; }

void ApCircle::draw()
{
    m_paths.push_back(CirclePath(m_diam * uScale));
    m_size = m_diam;
}
/////////////////////////////////////////////////////
/// \brief ApRectangle::ApRectangle
/// \param width
/// \param height
/// \param drillDiam
/// \param format
///
ApRectangle::ApRectangle(double width, double height, double drillDiam, const Format* format)
    : AbstractAperture(format)
{
    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString ApRectangle::name() const //RECTANGLE
{
    if (qFuzzyCompare(m_width, m_height))
        return QString("R(SQ %1)").arg(m_width);
    else
        return QString("R(%1 x %2)").arg(m_width).arg(m_height);
}

ApertureType ApRectangle::type() const { return Rectangle; }

bool ApRectangle::fit(double toolDiam) const { return qMin(m_height, m_width) > toolDiam; }

void ApRectangle::draw()
{
    m_paths.push_back(RectanglePath(m_width * uScale, m_height * uScale));
    m_size = qSqrt(m_width * m_width + m_height * m_height);
}
/////////////////////////////////////////////////////
/// \brief ApObround::ApObround
/// \param width
/// \param height
/// \param drillDiam
/// \param format
///
ApObround::ApObround(double width, double height, double drillDiam, const Format* format)
    : AbstractAperture(format)
{

    m_width = width;
    m_height = height;
    m_drillDiam = drillDiam;
}

QString ApObround::name() const { return QString("O(%1 x %2)").arg(m_width).arg(m_height); } //OBROUND

ApertureType ApObround::type() const { return Obround; }

bool ApObround::fit(double toolDiam) const { return qMin(m_height, m_width) > toolDiam; }

void ApObround::draw()
{
    Clipper clipper;
    const cInt h = static_cast<cInt>(m_height * uScale);
    const cInt w = static_cast<cInt>(m_width * uScale);
    if (qFuzzyCompare(w + 1.0, h + 1.0)) {
        m_paths.push_back(CirclePath(w));
    } else {
        if (w > h) {
            clipper.AddPath(CirclePath(h, IntPoint(-(w - h) / 2, 0)), ptClip, true);
            clipper.AddPath(CirclePath(h, IntPoint((w - h) / 2, 0)), ptClip, true);
            clipper.AddPath(RectanglePath(w - h, h), ptClip, true);
        } else if (w < h) {
            clipper.AddPath(CirclePath(w, IntPoint(0, -(h - w) / 2)), ptClip, true);
            clipper.AddPath(CirclePath(w, IntPoint(0, (h - w) / 2)), ptClip, true);
            clipper.AddPath(RectanglePath(w, h - w), ptClip, true);
        }
        clipper.Execute(ctUnion, m_paths, pftNonZero, pftNonZero);
    }
    m_size = qMax(w, h);
}
/////////////////////////////////////////////////////
/// \brief ApPolygon::ApPolygon
/// \param diam
/// \param nVertices
/// \param rotation
/// \param drillDiam
/// \param format
///
ApPolygon::ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const Format* format)
    : AbstractAperture(format)
{
    m_diam = diam;
    m_verticesCount = nVertices;
    m_rotation = rotation;
    m_drillDiam = drillDiam;
}

double ApPolygon::rotation() const { return m_rotation; }

int ApPolygon::verticesCount() const { return m_verticesCount; }

QString ApPolygon::name() const { return QString("P(Ø%1, N%2)").arg(m_diam).arg(m_verticesCount); } //POLYGON

ApertureType ApPolygon::type() const { return Polygon; }

bool ApPolygon::fit(double toolDiam) const { return m_diam * cos(M_PI / m_verticesCount) > toolDiam; }

void ApPolygon::draw()
{
    Path poligon;
    const double step = 360.0 / m_verticesCount;
    const double diam = this->m_diam * uScale;
    for (int i = 0; i < m_verticesCount; ++i) {
        poligon.push_back(IntPoint(
            static_cast<cInt>(qCos(qDegreesToRadians(step * i)) * diam * 0.5),
            static_cast<cInt>(qSin(qDegreesToRadians(step * i)) * diam * 0.5)));
    }
    if (m_rotation > 0.1) {
        RotatePath(poligon, m_rotation);
    }
    m_paths.push_back(poligon);
    m_size = diam;
}
/////////////////////////////////////////////////////
/// \brief ApMacro::ApMacro
/// \param macro
/// \param modifiers
/// \param coefficients
/// \param format
///
ApMacro::ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& coefficients, const Format* format)
    : AbstractAperture(format)
{
    m_macro = macro;
    m_modifiers = modifiers;
    while (m_modifiers.size() && m_modifiers.last().isEmpty()) {
        m_modifiers.removeLast();
    }
    m_coefficients = coefficients;
}

QString ApMacro::name() const { return QString("M(%1)").arg(m_macro); } //MACRO

ApertureType ApMacro::type() const { return Macro; }

bool ApMacro::fit(double) const { return true; }

void ApMacro::draw()
{
    enum {
        Comment = 0,
        Circle = 1,
        OutlineCustomPolygon = 4, // MAXIMUM 5000 POINTS
        OutlineRegularPolygon = 5, // 3-12 POINTS
        Moire = 6,
        Thermal = 7,
        VectorLine = 20,
        CenterLine = 21,
    };

    QMap<QString, double> macroCoefficients { m_coefficients };
    QVector<QPair<bool, Path>> items;
    try {
        for (int i = 0; i < m_modifiers.size(); ++i) {
            QString var(m_modifiers[i]);
            if (var.at(0) == '0') { // Skip Comment
                qDebug() << "Macro comment:" << var;
                continue;
            }

            QList<double> mod;

            if (var.contains('=')) {
                QList<QString> stringList = var.split('=');
                macroCoefficients[stringList.first()] = MathParser(macroCoefficients).parse(stringList.last().replace(QChar('x'), '*', Qt::CaseInsensitive));
                continue;
            } else {
                for (QString& var2 : var.split(',')) {
                    mod.push_back(var2.contains('$')
                            ? MathParser(macroCoefficients).parse(var2.replace(QChar('x'), '*', Qt::CaseInsensitive))
                            : var2.toDouble());
                }
            }

            if (mod.size() < 2)
                continue;

            const bool exposure = !qFuzzyIsNull(mod[1]);
            Path path;

            switch (static_cast<int>(mod[0])) {
            case Comment:
                continue;
            case Circle:
                path = drawCircle(mod);
                break;
            case OutlineCustomPolygon:
                path = drawOutlineCustomPolygon(mod);
                break;
            case OutlineRegularPolygon:
                path = drawOutlineRegularPolygon(mod);
                break;
            case Moire:
                drawMoire(mod);
                return;
            case Thermal:
                drawThermal(mod);
                return;
            case VectorLine:
                path = drawVectorLine(mod);
                break;
            case CenterLine:
                path = drawCenterLine(mod);
                break;
            }

            const double area = Area(path);
            if (area < 0 && exposure)
                ReversePath(path);
            else if (area > 0 && !exposure)
                ReversePath(path);

            items.append({ exposure, path });
        }
    } catch (...) {
        qWarning() << "Macro draw error";
        throw;
    }

    if (items.size() > 1) {
        Clipper clipper;
        for (int i = 0; i < items.size();) {
            clipper.Clear();
            clipper.AddPaths(m_paths, ptSubject, true);
            bool exp = items[i].first;
            while (i < items.size() && exp == items[i].first)
                clipper.AddPath(items[i++].second, ptClip, true);
            if (exp)
                clipper.Execute(ctUnion, m_paths, pftNonZero, pftNonZero);
            else
                clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);
        }
    } else
        m_paths.append(items.first().second);

    //    {
    //        Clipper clipper;
    //        clipper.AddPaths(m_paths, ptSubject, true);
    //        IntRect r(clipper.GetBounds());
    //        int k = uScale ;
    //        Path outer {
    //            IntPoint(r.left - k, r.bottom + k),
    //            IntPoint(r.right + k, r.bottom + k),
    //            IntPoint(r.right + k, r.top - k),
    //            IntPoint(r.left - k, r.top - k)
    //        };
    //        clipper.AddPath(outer, ptClip, true);
    //        clipper.Execute(ctXor, m_paths, pftEvenOdd);
    //        m_paths.takeFirst();
    //    }

    ClipperBase clipperBase;
    clipperBase.AddPaths(m_paths, ptSubject, true);
    IntRect rect = clipperBase.GetBounds();
    rect.right -= rect.left;
    rect.top -= rect.bottom;
    const double x = rect.right * dScale;
    const double y = rect.top * dScale;
    m_size = qSqrt(x * x + y * y);
}

Path ApMacro::drawCenterLine(const QList<double>& mod)
{
    enum {
        Width = 2,
        Height,
        CenterX,
        CenterY,
        RotationAngle
    };

    const IntPoint center(
        static_cast<cInt>(mod[CenterX] * uScale),
        static_cast<cInt>(mod[CenterY] * uScale));

    Path polygon = RectanglePath(mod[Width] * uScale, mod[Height] * uScale, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

    return polygon;
}

Path ApMacro::drawCircle(const QList<double>& mod)
{
    enum {
        Diameter = 2,
        CenterX,
        CenterY,
        RotationAngle
    };

    const IntPoint center(
        static_cast<cInt>(mod[CenterX] * uScale),
        static_cast<cInt>(mod[CenterY] * uScale));

    Path polygon = CirclePath(mod[Diameter] * uScale, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

    return polygon;
}

void ApMacro::drawMoire(const QList<double>& mod)
{
    enum {
        CenterX = 1,
        CenterY,
        Diameter,
        Thickness,
        Gap,
        NumberOfRings,
        CrossThickness,
        CrossLength,
        RotationAngle,
    };

    cInt diameter = static_cast<cInt>(mod[Diameter] * uScale);
    const cInt thickness = static_cast<cInt>(mod[Thickness] * uScale);
    const cInt gap = static_cast<cInt>(mod[Gap] * uScale);
    const cInt ct = static_cast<cInt>(mod[CrossThickness] * uScale);
    const cInt cl = static_cast<cInt>(mod[CrossLength] * uScale);

    const IntPoint center(
        static_cast<cInt>(mod[CenterX] * uScale),
        static_cast<cInt>(mod[CenterY] * uScale));

    {
        Clipper clipper;
        if (thickness && gap) {
            for (int num = 0; num < mod[NumberOfRings]; ++num) {
                clipper.AddPath(CirclePath(diameter), ptClip, true);
                diameter -= thickness * 2;
                Path polygon(CirclePath(diameter));
                ReversePath(polygon);
                clipper.AddPath(polygon, ptClip, true);
                diameter -= gap * 2;
            }
        }
        if (cl && ct) {
            clipper.AddPath(RectanglePath(cl, ct), ptClip, true);
            clipper.AddPath(RectanglePath(ct, cl), ptClip, true);
        }
        clipper.Execute(ctUnion, m_paths, pftPositive, pftPositive);
    }

    for (Path& path : m_paths)
        TranslatePath(path, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        for (Path& path : m_paths)
            RotatePath(path, mod[RotationAngle]);
    }
}

Path ApMacro::drawOutlineCustomPolygon(const QList<double>& mod)
{
    enum {
        NumberOfVertices = 2,
        X,
        Y,
    };

    const int num = static_cast<int>(mod[NumberOfVertices]);

    Path polygon;
    for (int j = 0; j < int(num); ++j)
        polygon.push_back(IntPoint(
            static_cast<cInt>(mod[X + j * 2] * uScale),
            static_cast<cInt>(mod[Y + j * 2] * uScale)));

    if (mod.size() > (num * 2 + 3) && mod.last() > 0)
        RotatePath(polygon, mod.last());

    return polygon;
}

Path ApMacro::drawOutlineRegularPolygon(const QList<double>& mod)
{
    enum {
        NumberOfVertices = 2,
        CenterX,
        CenterY,
        Diameter,
        RotationAngle
    };

    const int num = static_cast<int>(mod[NumberOfVertices]);
    if (3 > num || num > 12)
        throw QObject::tr("Bad outline (regular polygon) macro!");

    const cInt diameter = static_cast<cInt>(mod[Diameter] * uScale * 0.5);
    const IntPoint center(
        static_cast<cInt>(mod[CenterX] * uScale),
        static_cast<cInt>(mod[CenterY] * uScale));

    Path polygon;
    for (int j = 0; j < num; ++j)
        polygon.push_back(IntPoint(
            static_cast<cInt>(qCos(qDegreesToRadians(j * 360.0 / num)) * diameter),
            static_cast<cInt>(qSin(qDegreesToRadians(j * 360.0 / num)) * diameter)));

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

    TranslatePath(polygon, center);

    return polygon;
}

void ApMacro::drawThermal(const QList<double>& mod)
{
    enum {
        CenterX = 1,
        CenterY,
        OuterDiameter,
        InnerDiameter,
        GapThickness,
        RotationAngle
    };

    if (mod[OuterDiameter] <= mod[InnerDiameter] || mod[InnerDiameter] < 0.0 || mod[GapThickness] >= (mod[OuterDiameter] / qPow(2.0, 0.5)))
        throw QObject::tr("Bad thermal macro!");

    const cInt outer = static_cast<cInt>(mod[OuterDiameter] * uScale);
    const cInt inner = static_cast<cInt>(mod[InnerDiameter] * uScale);
    const cInt gap = static_cast<cInt>(mod[GapThickness] * uScale);

    const IntPoint center(
        static_cast<cInt>(mod[CenterX] * uScale),
        static_cast<cInt>(mod[CenterY] * uScale));

    {
        Clipper clipper;
        clipper.AddPath(CirclePath(outer), ptSubject, true);
        clipper.AddPath(CirclePath(inner), ptClip, true);
        clipper.AddPath(RectanglePath(gap, outer), ptClip, true);
        clipper.AddPath(RectanglePath(outer, gap), ptClip, true);
        clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);
    }

    for (Path& path : m_paths)
        TranslatePath(path, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        for (Path& path : m_paths)
            RotatePath(path, mod[RotationAngle]);
    }
}

Path ApMacro::drawVectorLine(const QList<double>& mod)
{
    enum {
        Width = 2,
        StartX,
        StartY,
        EndX,
        EndY,
        RotationAngle,
    };

    const IntPoint start(
        static_cast<cInt>(mod[StartX] * uScale),
        static_cast<cInt>(mod[StartY] * uScale));
    const IntPoint end(
        static_cast<cInt>(mod[EndX] * uScale),
        static_cast<cInt>(mod[EndY] * uScale));
    const IntPoint center(
        static_cast<cInt>(0.5 * start.X + 0.5 * end.X),
        static_cast<cInt>(0.5 * start.Y + 0.5 * end.Y));

    Path polygon = RectanglePath(Length(start, end), mod[Width] * uScale);
    double angle = Angle(start, end);
    RotatePath(polygon, angle);
    TranslatePath(polygon, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

    return polygon;
}
/////////////////////////////////////////////////////
/// \brief ApBlock::ApBlock
/// \param macro
/// \param modifiers
/// \param coefficients
/// \param format
///
ApBlock::ApBlock(const Format* format)
    : AbstractAperture(format)
{
}

QString ApBlock::name() const { return QString("BLOCK"); }

ApertureType ApBlock::type() const { return Block; }

bool ApBlock::fit(double) const { return true; }

void ApBlock::draw()
{
    m_paths.clear();
    int i = 0;
    while (i < size()) {
        Clipper clipper; //(ioStrictlySimple);
        clipper.AddPaths(m_paths, ptSubject, true);
        const int exp = at(i).state().imgPolarity();
        do {
            m_paths.append(at(i).paths());
            clipper.AddPaths(at(i++).paths(), ptClip, true);
        } while (i < size() && exp == at(i).state().imgPolarity());
        if (at(i - 1).state().imgPolarity() == Positive)
            clipper.Execute(ctUnion, m_paths, pftPositive);
        else
            clipper.Execute(ctDifference, m_paths, pftNonZero);
    }
    m_size = 1;
    qDebug() << m_paths.size();
    //CleanPolygons(m_paths, 0.0009 * uScale);
}

} // namespace Gerber

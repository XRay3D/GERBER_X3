// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com

#include "gbraperture.h"
#include <QDebug>
#include <QLineF>

#include "scene.h"

namespace Gerber {

AbstractAperture::AbstractAperture(const Format* format)
    : m_format(format)
{
}

AbstractAperture::~AbstractAperture() { }

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

    drawC(state, fl);

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

void ApCircle::read(QDataStream& stream)
{
    stream >> m_diam;
    stream >> m_drillDiam;
    stream >> m_isFlashed;
    stream >> m_size;
    draw();
}

void ApCircle::write(QDataStream& stream) const
{
    stream << m_diam;
    stream << m_drillDiam;
    stream << m_isFlashed;
    stream << m_size;
}

void ApCircle::draw()
{
    m_paths.push_back(CirclePath(m_diam * uScale));
    m_size = m_diam;
}

void ApCircle::drawC(const State& state, bool fl)
{
    Polygon_set_2 clipper;
    clipper.join(CirclePath2(m_diam, { state.curPos().X * dScale, state.curPos().Y * dScale }));
    //    if (items.size() > 1) {
    //        for (int i = 0; i < items.size();) {
    //            bool exp = items[i].first;
    //            while (i < items.size() && exp == items[i].first)
    //                if (exp)
    //                    clipper.join(items[i++].second);
    //                else
    //                    clipper.difference(items[i++].second);
    //        }
    //    } else {
    //        clipper.join(items.first().second);
    //    }
    QPainterPath pp;
    clipper.polygons_with_holes(boost::make_function_output_iterator([&pp](const Polygon_with_holes_2& pgn) {
        if (!pgn.is_unbounded()) {
            //            auto i = App::scene()->addPath(construct_path(pgn.outer_boundary()), QPen(Qt::green, 0.0), Qt::darkGreen);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(pgn.outer_boundary()));
        }

        Polygon_with_holes_2::Hole_const_iterator current = pgn.holes_begin();
        Polygon_with_holes_2::Hole_const_iterator end = pgn.holes_end();
        while (current != end) {
            //            auto i = App::scene()->addPath(construct_path(*current), QPen(Qt::red, 0.0), Qt::darkRed);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(*current));
            current++;
        }
    }));
    auto i = App::scene()->addPath(pp, QPen(Qt::green, 0.0), Qt::darkGreen);
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

void ApRectangle::read(QDataStream& stream)
{
    stream >> m_height;
    stream >> m_width;
    stream >> m_drillDiam;
    stream >> m_isFlashed;
    stream >> m_size;
    draw();
}

void ApRectangle::write(QDataStream& stream) const
{
    stream << m_height;
    stream << m_width;
    stream << m_drillDiam;
    stream << m_isFlashed;
    stream << m_size;
}

void ApRectangle::draw()
{
    m_paths.push_back(RectanglePath(m_width * uScale, m_height * uScale));
    m_size = qSqrt(m_width * m_width + m_height * m_height);
}

void ApRectangle::drawC(const State& state, bool fl)
{
    Polygon_set_2 clipper;
    clipper.join(RectanglePath2(m_width, m_height, { state.curPos().X * dScale, state.curPos().Y * dScale }));
    //    if (items.size() > 1) {
    //        for (int i = 0; i < items.size();) {
    //            bool exp = items[i].first;
    //            while (i < items.size() && exp == items[i].first)
    //                if (exp)
    //                    clipper.join(items[i++].second);
    //                else
    //                    clipper.difference(items[i++].second);
    //        }
    //    } else {
    //        clipper.join(items.first().second);
    //    }
    QPainterPath pp;
    clipper.polygons_with_holes(boost::make_function_output_iterator([&pp](const Polygon_with_holes_2& pgn) {
        if (!pgn.is_unbounded()) {
            //            auto i = App::scene()->addPath(construct_path(pgn.outer_boundary()), QPen(Qt::green, 0.0), Qt::darkGreen);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(pgn.outer_boundary()));
        }

        Polygon_with_holes_2::Hole_const_iterator current = pgn.holes_begin();
        Polygon_with_holes_2::Hole_const_iterator end = pgn.holes_end();
        while (current != end) {
            //            auto i = App::scene()->addPath(construct_path(*current), QPen(Qt::red, 0.0), Qt::darkRed);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(*current));
            current++;
        }
    }));
    auto i = App::scene()->addPath(pp, QPen(Qt::green, 0.0), Qt::darkGreen);
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

void ApObround::read(QDataStream& stream)
{
    stream >> m_height;
    stream >> m_width;
    stream >> m_drillDiam;
    stream >> m_isFlashed;
    stream >> m_size;
    draw();
}

void ApObround::write(QDataStream& stream) const
{
    stream << m_height;
    stream << m_width;
    stream << m_drillDiam;
    stream << m_isFlashed;
    stream << m_size;
}

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

void ApObround::drawC(const State& state, bool fl)
{
    Polygon_set_2 clipper;
    if (qFuzzyCompare(m_width + 1.0, m_height + 1.0)) {
        clipper.join(CirclePath2(m_width, { state.curPos().X * dScale, state.curPos().Y * dScale }));
    } else {
        Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));
        if (m_width > m_height) {
            clipper.join(CirclePath2(m_height, t(Point_2(-(m_width - m_height) / 2, 0))));
            clipper.join(CirclePath2(m_height, t(Point_2((m_width - m_height) / 2, 0))));
            clipper.join(RectanglePath2(m_width - m_height, m_height, Point_2(state.curPos().X * dScale, state.curPos().Y * dScale)));
        } else if //
            (m_width < m_height) {
            clipper.join(CirclePath2(m_width, t(Point_2(0, -(m_height - m_width) / 2))));
            clipper.join(CirclePath2(m_width, t(Point_2(0, (m_height - m_width) / 2))));
            clipper.join(RectanglePath2(m_width, m_height - m_width, Point_2(state.curPos().X * dScale, state.curPos().Y * dScale)));
        }
    }
    QPainterPath pp;
    clipper.polygons_with_holes(boost::make_function_output_iterator([&pp](const Polygon_with_holes_2& pgn) {
        if (!pgn.is_unbounded()) {
            //            auto i = App::scene()->addPath(construct_path(pgn.outer_boundary()), QPen(Qt::green, 0.0), Qt::darkGreen);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(pgn.outer_boundary()));
        }

        Polygon_with_holes_2::Hole_const_iterator current = pgn.holes_begin();
        Polygon_with_holes_2::Hole_const_iterator end = pgn.holes_end();
        while (current != end) {
            //            auto i = App::scene()->addPath(construct_path(*current), QPen(Qt::red, 0.0), Qt::darkRed);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(*current));
            current++;
        }
    }));
    auto i = App::scene()->addPath(pp, QPen(Qt::green, 0.0), Qt::darkGreen);
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

void ApPolygon::read(QDataStream& stream)
{
    stream >> m_diam;
    stream >> m_rotation;
    stream >> m_verticesCount;
    stream >> m_drillDiam;
    stream >> m_isFlashed;
    stream >> m_size;
    draw();
}

void ApPolygon::write(QDataStream& stream) const
{
    stream << m_diam;
    stream << m_rotation;
    stream << m_verticesCount;
    stream << m_drillDiam;
    stream << m_isFlashed;
    stream << m_size;
}

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

void ApPolygon::drawC(const State& state, bool fl)
{
    Polygon_set_2 clipper;

    Transformation r(CGAL::ROTATION, sin(qDegreesToRadians(m_rotation)), cos(qDegreesToRadians(m_rotation)));
    Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));
    t = t * r;
    const double step = 360.0 / m_verticesCount;
    QVector<Point_2> p;
    p.reserve(m_verticesCount);

    for (int i = 0; i < m_verticesCount; ++i) {
        p.push_back(
            t(Point_2(
                (qCos(qDegreesToRadians(step * i)) * m_diam * 0.5),
                (qSin(qDegreesToRadians(step * i)) * m_diam * 0.5))));
    }
    Polygon_2 poligon;
    for (int i = 0; i < m_verticesCount; ++i) {
        poligon.push_back(X_monotone_curve_2(p[i], p[(i + 1) % m_verticesCount]));
    }

    clipper.join(poligon);

    QPainterPath pp;
    clipper.polygons_with_holes(boost::make_function_output_iterator([&pp](const Polygon_with_holes_2& pgn) {
        if (!pgn.is_unbounded()) {
            //            auto i = App::scene()->addPath(construct_path(pgn.outer_boundary()), QPen(Qt::green, 0.0), Qt::darkGreen);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(pgn.outer_boundary()));
        }

        Polygon_with_holes_2::Hole_const_iterator current = pgn.holes_begin();
        Polygon_with_holes_2::Hole_const_iterator end = pgn.holes_end();
        while (current != end) {
            //            auto i = App::scene()->addPath(construct_path(*current), QPen(Qt::red, 0.0), Qt::darkRed);
            //i->setZValue(std::numeric_limits<double>::max());
            pp.addPath(construct_path(*current));
            current++;
        }
    }));
    auto i = App::scene()->addPath(pp, QPen(Qt::green, 0.0), Qt::darkGreen);
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

void ApBlock::read(QDataStream& stream)
{
    stream >> *this;
    stream >> m_isFlashed;
    stream >> m_size;
    draw();
}

void ApBlock::write(QDataStream& stream) const
{
    stream << *this;
    stream << m_isFlashed;
    stream << m_size;
}

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

void ApBlock::drawC(const State& state, bool fl)
{
    //    Polygon_set_2 clipper;
    //    if (items.size() > 1) {
    //        for (int i = 0; i < items.size();) {
    //            bool exp = items[i].first;
    //            while (i < items.size() && exp == items[i].first)
    //                if (exp)
    //                    clipper.join(items[i++].second);
    //                else
    //                    clipper.difference(items[i++].second);
    //        }
    //    } else {
    //        clipper.join(items.first().second);
    //    }
    //    QPainterPath pp;
    //    clipper.polygons_with_holes(boost::make_function_output_iterator([&pp](const Polygon_with_holes_2& pgn) {
    //        if (!pgn.is_unbounded()) {
    //            //            auto i = App::scene()->addPath(construct_path(pgn.outer_boundary()), QPen(Qt::green, 0.0), Qt::darkGreen);
    //            //i->setZValue(std::numeric_limits<double>::max());
    //            pp.addPath(construct_path(pgn.outer_boundary()));
    //        }
    //        Polygon_with_holes_2::Hole_const_iterator current = pgn.holes_begin();
    //        Polygon_with_holes_2::Hole_const_iterator end = pgn.holes_end();
    //        while (current != end) {
    //            //            auto i = App::scene()->addPath(construct_path(*current), QPen(Qt::red, 0.0), Qt::darkRed);
    //            //i->setZValue(std::numeric_limits<double>::max());
    //            pp.addPath(construct_path(*current));
    //            current++;
    //        }
    //    }));
    //    auto i = App::scene()->addPath(pp, QPen(Qt::green, 0.0), Qt::darkGreen);
}

} // namespace Gerber

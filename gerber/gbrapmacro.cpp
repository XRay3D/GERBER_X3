#include "gbrapmacro.h"

#include "mathparser.h"
#include "scene.h"

namespace Gerber {

ApMacro::ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& coefficients, const Format* format)
    : AbstractAperture(format)
    , m_macro(macro)
    , m_modifiers(modifiers)
    , m_coefficients(coefficients)
{
    while (m_modifiers.size() && m_modifiers.last().isEmpty()) {
        m_modifiers.removeLast();
    }
}

QString ApMacro::name() const { return QString("M(%1)").arg(m_macro); } //MACRO

ApertureType ApMacro::type() const { return Macro; }

bool ApMacro::fit(double) const { return true; }

void ApMacro::drawC(const State& state, bool /*fl*/)
{
    qDebug() << Q_FUNC_INFO << name() << state.aperture();
    //    QTime t;
    //    t = QTime::currentTime();
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

    QVector<QPair<bool, Polygon_2>> items;

    try {
        for (int i = 0; i < m_modifiers.size(); ++i) {
            QString var(m_modifiers[i]);
            if (var.at(0) == '0') { // Skip Comment
                //qDebug() << "Macro comment:" << var;
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
                            ? (MathParser(macroCoefficients).parse(var2.replace(QChar('x'), '*', Qt::CaseInsensitive)))
                            : var2.toDouble());
                }
            }

            if (mod.size() < 2)
                continue;

            Polygon_2 path;

            QMap<int, QString> name;
            name[Comment] = "Comment";
            name[Circle] = "Circle";
            name[OutlineCustomPolygon] = "OutlineCustomPolygon";
            name[OutlineRegularPolygon] = "OutlineRegularPolygon";
            name[Moire] = "Moire";
            name[Thermal] = "Thermal";
            name[VectorLine] = "VectorLine";
            name[CenterLine] = "CenterLine";

            //qDebug() << name[mod[0]];

            switch (static_cast<int>(mod[0])) {
            case Comment:
                continue;
            case Circle:
                path = drawCircle(state, mod);
                break;
            case OutlineCustomPolygon:
                path = drawOutlineCustomPolygon(state, mod);
                break;
            case OutlineRegularPolygon:
                path = drawOutlineRegularPolygon(state, mod);
                break;
            case Moire:
                drawMoire(state, items, mod);
                break;
            case Thermal:
                drawThermal(state, items, mod);
                break;
            case VectorLine:
                path = drawVectorLine(state, mod);
                break;
            case CenterLine:
                path = drawCenterLine(state, mod);
                break;
            }
            //            if (m_format->unitMode == Inches)
            //                for (IntPoint& pt : path) {
            //                    pt.X *= 25.4;
            //                    pt.Y *= 25.4;
            //                }
            //            const double area = Area(path);
            //            if (area < 0 && exposure)
            //                ReversePath(path);
            //            else if (area > 0 && !exposure)
            //                ReversePath(path);

            items.append({ !qFuzzyIsNull(mod[1]), path });
        }
    } catch (...) {
        qWarning() << "Macro draw error";
        throw;
    }
A:
    Polygon_set_2 clipper;
    if (items.size() > 1) {
        for (int i = 0; i < items.size();) {
            bool exp = items[i].first;
            while (i < items.size() && exp == items[i].first)
                if (exp)
                    clipper.join(items[i++].second);
                else
                    clipper.difference(items[i++].second);
        }
    } else {
        clipper.join(items.first().second);
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
    //    qDebug() << t.msecsTo(QTime::currentTime());
}

void ApMacro::read(QDataStream& stream)
{
    stream >> m_modifiers;
    stream >> m_coefficients;
    stream >> m_macro;
    stream >> m_isFlashed;
    stream >> m_size;
    draw();
}

void ApMacro::write(QDataStream& stream) const
{
    stream << m_modifiers;
    stream << m_coefficients;
    stream << m_macro;
    stream << m_isFlashed;
    stream << m_size;
}

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
                //qDebug() << "Macro comment:" << var;
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

            QMap<int, QString> name;
            name[Comment] = "Comment";
            name[Circle] = "Circle";
            name[OutlineCustomPolygon] = "OutlineCustomPolygon";
            name[OutlineRegularPolygon] = "OutlineRegularPolygon";
            name[Moire] = "Moire";
            name[Thermal] = "Thermal";
            name[VectorLine] = "VectorLine";
            name[CenterLine] = "CenterLine";

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

    Path polygon = ::RectanglePath(mod[Width] * uScale, mod[Height] * uScale, center);

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
            clipper.AddPath(::RectanglePath(cl, ct), ptClip, true);
            clipper.AddPath(::RectanglePath(ct, cl), ptClip, true);
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
    for (int j = 0; j < num; ++j) {
        auto angle = qDegreesToRadians(j * 360.0 / num);
        polygon.push_back(IntPoint(
            static_cast<cInt>(qCos(angle) * diameter),
            static_cast<cInt>(qSin(angle) * diameter)));
    }

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
        clipper.AddPath(::RectanglePath(gap, outer), ptClip, true);
        clipper.AddPath(::RectanglePath(outer, gap), ptClip, true);
        clipper.Execute(ctDifference, m_paths, pftNonZero, pftNonZero);
    }

    for (Path& path : m_paths)
        TranslatePath(path, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        for (Path& path : m_paths)
            RotatePath(path, mod[RotationAngle]);
    }
}

Polygon_2 ApMacro::RectanglePath(const Point_2& wh, Transformation& t, const Point_2& center)
{
    auto halfWidth = wh.x() /*width*/ * 0.5;
    auto halfHeight = wh.y() /*height*/ * 0.5;

    QVector<Point_2> p {
        Point_2(-halfWidth + center.x(), +halfHeight + center.y()),
        Point_2(-halfWidth + center.x(), -halfHeight + center.y()),
        Point_2(+halfWidth + center.x(), -halfHeight + center.y()),
        Point_2(+halfWidth + center.x(), +halfHeight + center.y()),
    };

    for (auto& pt : p)
        pt = t(pt);

    Polygon_2 path;
    path.push_back(X_monotone_curve_2(p[0], p[1]));
    path.push_back(X_monotone_curve_2(p[1], p[2]));
    path.push_back(X_monotone_curve_2(p[2], p[3]));
    path.push_back(X_monotone_curve_2(p[3], p[0]));
    return path;
}

Polygon_2 ApMacro::drawCenterLine(const State& state, const QList<double>& mod)
{
    enum {
        Width = 2,
        Height,
        CenterX,
        CenterY,
        RotationAngle
    };

    Point_2 center(mod[CenterX], mod[CenterY]);

    const double halfWidth = mod[Width] * 0.5;
    const double halfHeight = mod[Height] * 0.5;

    Transformation s(CGAL::SCALING, m_format->unitMode == Inches ? 25.4 : 1.0);
    Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));

    QVector<Point_2> p {
        s(Point_2(-halfWidth + center.x(), +halfHeight + center.y())),
        s(Point_2(-halfWidth + center.x(), -halfHeight + center.y())),
        s(Point_2(+halfWidth + center.x(), -halfHeight + center.y())),
        s(Point_2(+halfWidth + center.x(), +halfHeight + center.y())),
    };

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0) {
        Transformation r(CGAL::ROTATION, sin(qDegreesToRadians(mod[RotationAngle] - 360)), cos(qDegreesToRadians(mod[RotationAngle] - 360)));
        for (auto& pt : p)
            pt = r(pt);
    }

    for (auto& pt : p)
        pt = t(pt);
    Polygon_2 path;
    path.push_back(X_monotone_curve_2(p[0], p[1]));
    path.push_back(X_monotone_curve_2(p[1], p[2]));
    path.push_back(X_monotone_curve_2(p[2], p[3]));
    path.push_back(X_monotone_curve_2(p[3], p[0]));
    return path;
}

Polygon_2 ApMacro::drawCircle(const State& state, const QList<double>& mod)
{
    enum {
        Diameter = 2,
        CenterX,
        CenterY,
        RotationAngle
    };

    bool rb = mod.size() > RotationAngle && mod.last() > 0;

    Transformation r(CGAL::ROTATION,
        sin(qDegreesToRadians(rb ? mod[RotationAngle] : 0.0)),
        cos(qDegreesToRadians(rb ? mod[RotationAngle] : 0.0)));
    Transformation s(CGAL::SCALING, m_format->unitMode == Inches ? 25.4 : 1.0);
    Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));
    double rad = mod[Diameter] * (m_format->unitMode == Inches ? 12.7 : 0.5);
    Point_2 center(mod[CenterX], mod[CenterY]);
    return construct_polygon(Circle_2(t(r(s(center))), rad * rad));
}

Polygon_2 ApMacro::drawOutlineCustomPolygon(const State& state, const QList<double>& mod)
{
    enum {
        NumberOfVertices = 2,
        X,
        Y,
    };

    const int num = static_cast<int>(mod[NumberOfVertices]);
    bool rb = mod.size() > (num * 2 + 3) && mod.last() > 0;
    double rot = qDegreesToRadians(rb ? mod.last() : 0.0);
    Transformation r(CGAL::ROTATION, sin(rot), cos(rot));
    Transformation s(CGAL::SCALING, m_format->unitMode == Inches ? 25.4 : 1.0);
    Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));
    t = t * r * s;

    QVector<Point_2> p;
    p.reserve(num);

    for (int j = 0; j < num; ++j)
        p.append(t(Point_2(mod[X + j * 2], mod[Y + j * 2])));

    Polygon_2 path;
    for (int j = 0; j < num; ++j)
        path.push_back(X_monotone_curve_2(p[j], p[(j + 1) % num]));

    if (path.orientation() == CGAL::NEGATIVE)
        path.reverse_orientation();

    return path;
}

Polygon_2 ApMacro::drawOutlineRegularPolygon(const State& state, const QList<double>& mod)
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

    bool rb = mod.size() > RotationAngle && mod.last() > 0;
    double rot = qDegreesToRadians(rb ? mod.last() : 0.0);
    Transformation r(CGAL::ROTATION, sin(rot), cos(rot));
    Transformation s(CGAL::SCALING, m_format->unitMode == Inches ? 1 /*25.4*/ : 1.0);
    Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));
    Transformation t2(CGAL::TRANSLATION,
        Vector(mod[CenterX] * (m_format->unitMode == Inches ? 25.4 : 1.0),
            mod[CenterY] * (m_format->unitMode == Inches ? 25.4 : 1.0)));

    t = t * r * s;

    const Point_2 center(mod[CenterX], mod[CenterY]);
    double diameter = mod[Diameter] * (m_format->unitMode == Inches ? 12.7 : 0.5);

    Polygon_2 polygon;
    for (int j = 0; j < num; ++j) {
        auto angle1 = qDegreesToRadians(j * 360.0 / num);
        auto angle2 = qDegreesToRadians((j + 1 % num) * 360.0 / num);
        Point_2 p1(qCos(angle1) * diameter, qSin(angle1) * diameter);
        Point_2 p2(qCos(angle2) * diameter, qSin(angle2) * diameter);
        polygon.push_back(X_monotone_curve_2(t(p1), t(p2)));
    }
    return polygon;
}

Polygon_2 ApMacro::drawVectorLine(const State& state, const QList<double>& mod)
{
    enum {
        Width = 2,
        StartX,
        StartY,
        EndX,
        EndY,
        RotationAngle,
    };

    //bool rb = mod.size() > RotationAngle && mod[RotationAngle] != 0.0;
    //double rot = qDegreesToRadians(rb ? mod.last() : 0.0);
    //Transformation r(CGAL::ROTATION,
    //   sin(qDegreesToRadians(rb ? mod.last() - 180 : 0.0)),
    //   cos(qDegreesToRadians(rb ? mod.last() - 180 : 0.0)));
    Transformation s(CGAL::SCALING, m_format->unitMode == Inches ? 25.4 : 1.0);
    Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));

    const QPointF p1(mod[StartX], mod[StartY]);
    const QPointF p2(mod[EndX], mod[EndY]);
    const Point_2 center(
        0.5 * mod[StartX] + 0.5 * mod[EndX],
        0.5 * mod[StartY] + 0.5 * mod[EndY]);

    QLineF l(p1, p2);
    double rot = qDegreesToRadians(l.angle());
    qDebug() << "RotationAngle" << mod.last() << rot;
    Transformation r(CGAL::ROTATION, sin(rot), cos(rot));
    t = t * r * s;

    Polygon_2 polygon;
    polygon = RectanglePath(Point_2(l.length(), mod[Width]), t, center);

    //    double angle = Angle(start, end);
    //    RotatePath(polygon, angle);
    //    TranslatePath(polygon, center);
    //    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
    //        RotatePath(polygon, mod[RotationAngle]);

    return polygon;
}

void ApMacro::drawMoire(const State& state, QVector<QPair<bool, Polygon_2>>& items, const QList<double>& mod) { }

void ApMacro::drawThermal(const State& state, QVector<QPair<bool, Polygon_2>>& items, const QList<double>& mod)
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

    bool rb = mod.size() > RotationAngle && mod[RotationAngle] != 0.0;
    double rot = qDegreesToRadians(rb ? mod.last() : 0.0);
    Transformation r(CGAL::ROTATION, sin(rot), cos(rot));
    Transformation s(CGAL::SCALING, m_format->unitMode == Inches ? 25.4 : 1.0);
    Transformation t(CGAL::TRANSLATION, Vector(state.curPos().X * dScale, state.curPos().Y * dScale));
    t = t * r * s;

    const Point_2 center(mod[CenterX], mod[CenterY]);
    double radOut = mod[OuterDiameter] * (m_format->unitMode == Inches ? 12.7 : 0.5);
    double radIn = mod[InnerDiameter] * (m_format->unitMode == Inches ? 12.7 : 0.5);
    {
        items.append({ true, construct_polygon(Circle_2(t(center), radOut * radOut)) });
        items.append({ false, construct_polygon(Circle_2(t(center), radIn * radIn)) });
        items.append({ false, RectanglePath(Point_2 { mod[GapThickness], mod[OuterDiameter] + 1 }, t) });
        items.append({ false, RectanglePath(Point_2 { mod[OuterDiameter] + 1, mod[GapThickness] }, t) });
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

    Path polygon = ::RectanglePath(Length(start, end), mod[Width] * uScale);
    double angle = Angle(start, end);
    RotatePath(polygon, angle);
    TranslatePath(polygon, center);

    if (mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        RotatePath(polygon, mod[RotationAngle]);

    return polygon;
}
}

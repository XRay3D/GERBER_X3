#include "myclipper.h"
#include <QElapsedTimer>
#include <QLineF>
#include <qmath.h>
#include <settings.h>

Path toPath(const QPolygonF& p)
{
    Path path;
    for (const QPointF& pt : p)
        path.push_back(IntPoint(pt.x() * uScale, pt.y() * uScale));
    return path;
}

Paths toPaths(const QVector<QPolygonF>& p)
{
    Paths paths;
    for (const QPolygonF& pl : p)
        paths.push_back(toPath(pl));
    return paths;
}

QPolygonF toQPolygon(const Path& p)
{
    QPolygonF polygon;
    for (const IntPoint& pt : p)
        polygon.push_back(QPointF(pt.X * dScale, pt.Y * dScale));
    return polygon;
}

QVector<QPolygonF> toQPolygons(const Paths& p)
{
    QVector<QPolygonF> polygons;
    for (const Path& pl : p)
        polygons.push_back(toQPolygon(pl));
    return polygons;
}

QPointF toQPointF(const IntPoint& p) { return QPointF(p.X * dScale, p.Y * dScale); }

IntPoint toIntPoint(const QPointF& p) { return IntPoint(p.x() * uScale, p.y() * uScale); }

double Angle(const IntPoint& pt1, const IntPoint& pt2)
{
    const double dx = pt2.X - pt1.X;
    const double dy = pt2.Y - pt1.Y;

    const double theta = atan2(-dy, dx) * 360.0 / M_2PI;

    const double theta_normalized = theta < 0 ? theta + 360 : theta;

    if (qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

double Length(const IntPoint& pt1, const IntPoint& pt2)
{
    double x = pt2.X - pt1.X;
    double y = pt2.Y - pt1.Y;
    return sqrt(x * x + y * y);
}

static inline bool qt_is_finite(double d)
{
    uchar* ch = (uchar*)&d;
#ifdef QT_ARMFPA
    return (ch[3] & 0x7f) != 0x7f || (ch[2] & 0xf0) != 0xf0;
#else
    if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
        return (ch[0] & 0x7f) != 0x7f || (ch[1] & 0xf0) != 0xf0;
    } else {
        return (ch[7] & 0x7f) != 0x7f || (ch[6] & 0xf0) != 0xf0;
    }
#endif
}

Path CirclePath(double diametr, const IntPoint& center)
{
    if (diametr == 0.0)
        return Path();

    const double radius = diametr * 0.5;
    const int intSteps = Settings::circleSegments(radius * dScale); // MinStepsPerCircle;
    Path poligon(intSteps);
    for (int i = 0; i < intSteps; ++i) {
        poligon[i] = IntPoint(
            (cos(i * 2 * M_PI / intSteps) * radius) + center.X,
            (sin(i * 2 * M_PI / intSteps) * radius) + center.Y);
    }
    return poligon;
}

Path RectanglePath(double width, double height, const IntPoint& center)
{

    const double halfWidth = width * 0.5;
    const double halfHeight = height * 0.5;
    Path poligon{
        IntPoint(-halfWidth + center.X, +halfHeight + center.Y),
        IntPoint(-halfWidth + center.X, -halfHeight + center.Y),
        IntPoint(+halfWidth + center.X, -halfHeight + center.Y),
        IntPoint(+halfWidth + center.X, +halfHeight + center.Y),
    };
    if (Area(poligon) < 0.0)
        ReversePath(poligon);

    return poligon;
}

void RotatePath(Path& poligon, double angle, const IntPoint& center)
{
    bool fl = Area(poligon) < 0;
    for (IntPoint& pt : poligon) {
        const double tmpAangle = qDegreesToRadians(angle - Angle(center, pt));
        const double length = Length(center, pt);
        pt = IntPoint(cos(tmpAangle) * length, sin(tmpAangle) * length);
        pt.X += center.X;
        pt.Y += center.Y;
    }
    if (fl != (Area(poligon) < 0))
        ReversePath(poligon);
}

void TranslatePath(Path& path, const IntPoint& pos)
{
    if (pos.X == 0 && pos.Y == 0)
        return;
    for (Path::size_type i = 0, size = path.size(); i < size; ++i) {
        path[i].X += pos.X;
        path[i].Y += pos.Y;
    }
}

double Perimeter(const Path& path)
{
    double p = 0.0;
    for (int i = 0; i < path.size() - 1; ++i) {
        p += Length(path[i], path[i + 1]);
    }
    p += Length(path.first(), path.last());
    return p;
}

#ifndef MYCLIPPER_H
#define MYCLIPPER_H

#include "clipper.hpp"
#include <QDebug>
#include <QPolygonF>

#ifndef M_2PI
#define M_2PI (6.28318530717958647692528676655900576)
#endif

#ifndef M_PI
#define M_PI (3.1415926535897932384626433832795)
#endif

using namespace ClipperLib;

using Pathss = QVector /*std::vector*/<Paths>;

const /*long*/ long uScale = 10000;
const double dScale = 1.0 / uScale;

Path toPath(const QPolygonF& p);
Paths toPaths(const QVector<QPolygonF>& p);

QPolygonF toQPolygon(const Path& p);
QVector<QPolygonF> toQPolygons(const Paths& p);

QPointF toQPointF(const IntPoint& p);
IntPoint toIntPoint(const QPointF& p);

double Angle(const IntPoint& pt1, const IntPoint& pt2);
double Length(const IntPoint& pt1, const IntPoint& pt2);
double Perimeter(const Path& path);

//IntPoint Center(const IntPoint& pt1, const IntPoint& pt2)
//{
//    return IntPoint(int((qint64(pt1.X) + pt2.X) / 2), int((qint64(pt1.Y) + pt2.Y) / 2));
//}

Path CirclePath(double diametr, const IntPoint& center = IntPoint());
Path RectanglePath(double width, double height, const IntPoint& center = IntPoint());
void RotatePath(Path& poligon, double angle, const IntPoint& center = IntPoint());
void TranslatePath(Path& path, const IntPoint& pos);

#endif // MYCLIPPER_H

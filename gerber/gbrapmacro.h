#pragma once

#include "gbraperture.h"

#include <CGAL/Aff_transformation_2.h>
#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/General_polygon_set_2.h>
#include <CGAL/Gps_circle_segment_traits_2.h>
#include <CGAL/Lazy_exact_nt.h>
#include <CGAL/Polygon_2.h>

using Kernel = CGAL::Exact_predicates_exact_constructions_kernel;
using Point_2 = Kernel::Point_2;
using Circle_2 = Kernel::Circle_2;
using Traits_2 = CGAL::Gps_circle_segment_traits_2<Kernel>;
using Polygon_set_2 = CGAL::General_polygon_set_2<Traits_2>;
using Polygon_2 = Traits_2::General_polygon_2;
using Polygon_with_holes_2 = Traits_2::General_polygon_with_holes_2;
using Curve_2 = Traits_2::Curve_2;
using X_monotone_curve_2 = Traits_2::X_monotone_curve_2;

using Transformation = CGAL::Aff_transformation_2<Kernel>;
using Point = CGAL::Point_2<Kernel>;
using Vector = CGAL::Vector_2<Kernel>;
using Direction = CGAL::Direction_2<Kernel>;

namespace Gerber {

/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
class ApMacro final : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& coefficients, const Format* format);
    ApMacro(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    QString name() const;
    ApertureType type() const;
    bool fit(double) const;
    void draw(const State& state, bool fl);

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;

private:
    QString m_macro;
    QList<QString> m_modifiers;
    QMap<QString, double> m_coefficients;

    Path drawCenterLine(const QList<double>& mod);
    Path drawCircle(const QList<double>& mod);
    Path drawOutlineCustomPolygon(const QList<double>& mod);
    Path drawOutlineRegularPolygon(const QList<double>& mod);
    Path drawVectorLine(const QList<double>& mod);
    void drawMoire(const QList<double>& mod);
    void drawThermal(const QList<double>& mod);

    ////////////

    Polygon_2 RectanglePath(const Point_2& wh /*double width, double height*/, Transformation& t, const Point_2& center = {});

    Polygon_2 drawCenterLine(const State& state, const QList<double>& mod);
    Polygon_2 drawCircle(const State& state, const QList<double>& mod);
    Polygon_2 drawOutlineCustomPolygon(const State& state, const QList<double>& mod);
    Polygon_2 drawOutlineRegularPolygon(const State& state, const QList<double>& mod);
    Polygon_2 drawVectorLine(const State& state, const QList<double>& mod);
    void drawMoire(const State& state, QVector<QPair<bool, Polygon_2>>& items, const QList<double>& mod);
    void drawThermal(const State& state, QVector<QPair<bool, Polygon_2>>& items, const QList<double>& mod);
};
}

// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  March 25, 2023                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2023                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#include "cgal_solver.h"
#include "app.h"
#include "gi_ppath.h"
#include "graphicsview.h"
#include <CGAL/Kernel/global_functions_2.h>
#include <CGAL/Point_2.h>
#include <CGAL/Polygon_2.h>
#include <CGAL/Polygon_2_algorithms.h>
#include <CGAL/approximated_offset_2.h>
#include <CGAL/draw_polygon_2.h>
#include <CGAL/minkowski_sum_2.h>
#include <CGAL/offset_polygon_2.h>
#include <QDebug>
#include <QGraphicsScene>
#include <cassert>
#include <iterator>
#include <qmath.h>

#include <CGAL/Polygon_repair/repair.h>

#include <qnamespace.h>
#include <qpolygon.h>
#include <qtransform.h>
#include <span>
#include <utility>
#include <vector>

// typedef double Approximate_number_type;
// typedef CGAL::Cartesian<Approximate_number_type> Approximate_kernel;
// typedef Approximate_kernel::Point_2 Approximate_point_2;

// class Approximate_2 {
// protected:
//     using Traits = Arr_circle_segment_Traits<Kernel, Filter>;

//     /*! The traits (in case it has state) */
//     const Traits& m_traits;

//     /*! Constructor
//      * \param traits the traits.
//      */
//     Approximate_2(const Traits& traits)
//         : m_traits(traits) { }

//     friend class Arr_circle_segment_Traits<Kernel, Filter>;

// public:
//     /*! Obtain an approximation of a point coordinate.
//      * \param p the exact point.
//      * \param i the coordinate index (either 0 or 1).
//      * \pre i is either 0 or 1.
//      * \return An approximation of p's x-coordinate (if i == 0), or an
//      *         approximation of p's y-coordinate (if i == 1).
//      */
//     Approximate_number_type operator()(const Point_2& p, int i) const {
//         CGAL_precondition((i == 0) || (i == 1));
//         return (i == 0) ? (CGAL::to_double(p.x())) : (CGAL::to_double(p.y()));
//     }

//     /*! Obtain an approximation of a point.
//      */
//     Approximate_point_2 operator()(const Point_2& p) const { return Approximate_point_2(operator()(p, 0), operator()(p, 1)); }

//     /*! Obtain an approximation of an \f$x\f$-monotone curve.
//      */
//     template <typename OutputIterator>
//     OutputIterator operator()(const X_monotone_curve_2& xcv, double error,
//         OutputIterator oi, bool l2r = true) const {
//         if(xcv.is_linear()) return approximate_segment(xcv, oi, l2r);
//         return approximate_arc(xcv, error, oi, l2r);
//         ;
//     }

// private:
//     /*! Handle segments.
//      */
//     template <typename OutputIterator>
//     OutputIterator approximate_segment(const X_monotone_curve_2& xcv,
//         OutputIterator oi,
//         bool l2r = true) const {
//         // std::cout << "SEGMENT\n";
//         auto min_vertex = m_traits.construct_min_vertex_2_object();
//         auto max_vertex = m_traits.construct_max_vertex_2_object();
//         const auto& src = (l2r) ? min_vertex(xcv) : max_vertex(xcv);
//         const auto& trg = (l2r) ? max_vertex(xcv) : min_vertex(xcv);
//         auto xs = CGAL::to_double(src.x());
//         auto ys = CGAL::to_double(src.y());
//         auto xt = CGAL::to_double(trg.x());
//         auto yt = CGAL::to_double(trg.y());
//         *oi++ = Approximate_point_2(xs, ys);
//         *oi++ = Approximate_point_2(xt, yt);
//         return oi;
//     }

//     template <typename OutputIterator, typename Op, typename Transform>
//     OutputIterator add_points(double x1, double y1, double t1,
//         double x2, double y2, double t2,
//         double error, OutputIterator oi,
//         Op op, Transform transform) const {
//         auto tm = (t1 + t2) * 0.5;

//         // Compute the canocal point where the error is maximal.
//         double xm, ym;
//         op(tm, xm, ym);

//         auto dx = x2 - x1;
//         auto dy = y2 - y1;

//         // Compute the error; abort if it is below the threshold
//         auto l = std::sqrt(dx * dx + dy * dy);
//         auto e = std::abs((xm * dy - ym * dx + x2 * y1 - x1 * y2) / l);
//         if(e < error) return oi;

//         double x, y;
//         transform(xm, ym, x, y);
//         add_points(x1, y1, t1, xm, ym, tm, error, oi, op, transform);
//         *oi++ = Approximate_point_2(x, y);
//         add_points(xm, ym, tm, x2, y2, t2, error, oi, op, transform);
//         return oi;
//     }

//     /*! Compute the circular point given the parameter t and the transform
//      * data, that is, the center (translation) and the sin and cos of the
//      * rotation angle.
//      */
//     void circular_point(double r, double t, double& x, double& y) const {
//         x = r * std::cos(t);
//         y = r * std::sin(t);
//     }

//     /*! Transform a point. In particular, rotate the canonical point
//      * (`xc`,`yc`) by an angle, the sine and cosine of which are `sint` and
//      * `cost`, respectively, and translate by (`cx`,`cy`).
//      */
//     void transform_point(double xc, double yc, double cx, double cy,
//         double& x, double& y) const {
//         x = xc + cx;
//         y = yc + cy;
//     }

//     /*! Handle circular arcs.
//      */
//     template <typename OutputIterator>
//     OutputIterator approximate_arc(const X_monotone_curve_2& xcv,
//         double error, OutputIterator oi,
//         bool l2r = true) const {
//         auto min_vertex = m_traits.construct_min_vertex_2_object();
//         auto max_vertex = m_traits.construct_max_vertex_2_object();
//         const auto& src = (l2r) ? min_vertex(xcv) : max_vertex(xcv);
//         const auto& trg = (l2r) ? max_vertex(xcv) : min_vertex(xcv);
//         auto xs = CGAL::to_double(src.x());
//         auto ys = CGAL::to_double(src.y());
//         auto xt = CGAL::to_double(trg.x());
//         auto yt = CGAL::to_double(trg.y());

//         const typename Kernel::Circle_2& circ = xcv.supporting_circle();
//         auto r_sqr = circ.squared_radius();
//         auto r = std::sqrt(CGAL::to_double(r_sqr));

//         // Obtain the center:
//         auto cx = CGAL::to_double(circ.center().x());
//         auto cy = CGAL::to_double(circ.center().y());

//         // Inverse transform the source and target
//         auto xs_t = xs - cx;
//         auto ys_t = ys - cy;
//         auto xt_t = xt - cx;
//         auto yt_t = yt - cy;

//         // Compute the parameters ts and tt such that
//         // source == (x(ts),y(ts)), and
//         // target == (x(tt),y(tt))
//         auto ts = std::atan2(r * ys_t, r * xs_t);
//         if(ts < 0) ts += 2 * M_PI;
//         auto tt = std::atan2(r * yt_t, r * xt_t);
//         if(tt < 0) tt += 2 * M_PI;
//         auto orient(xcv.orientation());
//         if(xcv.source() != src) orient = CGAL::opposite(orient);
//         if(orient == CGAL::COUNTERCLOCKWISE) {
//             if(tt < ts) tt += 2 * M_PI;
//         } else {
//             if(ts < tt) ts += 2 * M_PI;
//         }

//         *oi++ = Approximate_point_2(xs, ys);
//         add_points(xs_t, ys_t, ts, xt_t, yt_t, tt, error, oi, [&](double tm, double& xm, double& ym) { circular_point(r, tm, xm, ym); }, [&](double xc, double& yc, double& x, double& y) { transform_point(xc, yc, cx, cy, x, y); });
//         *oi++ = Approximate_point_2(xt, yt);
//         return oi;
//     }
// };

/*! Obtain an Approximate_2 functor object. */
// Approximate_2 approximate_2_object() const { return Approximate_2(*this); }

QPainterPath ConstructPath(const Polygon_2& pgn_) {
    // if(pgn.size() < 4) return {}; // FIXME
    try {
        if(pgn_.is_empty()) return {};

        Polygon_2 pgn = pgn_;

        auto current = pgn.curves_begin();
        auto end = pgn.curves_end();

        // if(pgn_.size() == 1 && current->is_circular())

        if(auto back = --pgn.curves_end(); current->source() != back->target()) {
            const auto& source = current->source();
            const auto& target = back->target();
            const Point_2 ps{CGAL::to_double(target.x()), CGAL::to_double(target.y())};
            const Point_2 pt{CGAL::to_double(source.x()), CGAL::to_double(source.y())};
            pgn.push_back(X_monotone_curve_2{ps, pt});
            end = --pgn.curves_end();
        }

        auto orientation = area(pgn) < 0 ? CGAL::CLOCKWISE : CGAL::COUNTERCLOCKWISE; //    pgn.orientation();

        Q_ASSERT(orientation == CGAL::CLOCKWISE || orientation == CGAL::COUNTERCLOCKWISE);

        // Degenerate polygon, ring.size() < 3
        if(orientation == CGAL::ZERO) {
            qWarning() << "construct_path: Ignoring degenerated polygon";
            return {};
        }

        const bool isClockwise = orientation == CGAL::CLOCKWISE;

        QPainterPath result;
        result.moveTo(CGAL::to_double(current->source().x()), CGAL::to_double(current->source().y()));
        /*
                for(auto it = current; it != end; ++it) {
                    qWarning() << "S" << CGAL::to_double(it->source().x()) << CGAL::to_double(it->source().y());
                    qWarning() << "T" << CGAL::to_double(it->target().x()) << CGAL::to_double(it->target().y());
                }
        */
        // for(auto&& curve: std::span{current, end}) {
        // }
        do {
            const Polygon_2::X_monotone_curve_2& curve = *current;
            const auto& source = curve.source();
            const auto& target = curve.target();

            if(curve.is_linear()) {
                result.lineTo(QPointF{
                    CGAL::to_double(target.x()),
                    CGAL::to_double(target.y()),
                });
            } else if(curve.is_circular()) {
                const auto bbox = curve.supporting_circle().bbox();
                const QRectF rect{
                    QPointF{bbox.xmin(), bbox.ymin()},
                    QPointF{bbox.xmax(), bbox.ymax()}
                };
                const auto center = curve.supporting_circle().center();
                // angles
                const double asource = atan2(CGAL::to_double(source.y() - center.y()), CGAL::to_double(source.x() - center.x()));
                const double atarget = atan2(CGAL::to_double(target.y() - center.y()), CGAL::to_double(target.x() - center.x()));
                double aspan = atarget - asource;

                /**/ if(aspan < -CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && !isClockwise))
                    aspan += 2.0 * CGAL_PI;
                else if(aspan > +CGAL_PI || (qFuzzyCompare(aspan, -CGAL_PI) && isClockwise))
                    aspan -= 2.0 * CGAL_PI;

                result.arcTo(rect, qRadiansToDegrees(-asource), qRadiansToDegrees(-aspan));
            } else { // ?!?
                Q_UNREACHABLE();
            }
        } while(++current != end);

        return result;
    } catch(std::exception& e) {
        qCritical() << e.what();
#include <CGAL/Polygon_repair/repair.h>
    }
    return {};
}

void OffsetPoly(const Polygon_2& poly, double val) {
    QPolygonF pf;
    try {

        using Segment_2 = CGAL::Segment_2<Kernel>;
        using Polygon = CGAL::Polygon_2<Kernel>;
        using Point = CGAL::Point_2<Kernel>;
        using AP = Traits::Approximate_point_2;

        // auto poly_ = poly;
        // if(area(poly_) < 0) poly_.reverse_orientation();
#if 0
        auto approximate = [](const X_monotone_curve_2& curve, auto& oi, unsigned int n) {
            const double xSrc = CGAL::to_double(curve.source().x());
            const double ySrc = CGAL::to_double(curve.source().y());

            const double xTgt = CGAL::to_double(curve.target().x());
            const double yTgt = CGAL::to_double(curve.target().y());
            if(curve.is_linear()) {
                if(oi.empty() || oi.back() != std::make_pair(xSrc, ySrc))
                    oi.emplace_back(xSrc, ySrc);
                oi.emplace_back(xTgt, yTgt);
                return;
            }
            auto circle = curve.supporting_circle();
            // Otherwise, sample (n - 1) equally-spaced points in between.
            const double xCenter = CGAL::to_double(circle.center().x());
            const double yCenter = CGAL::to_double(circle.center().y());
            const double sqrRad = CGAL::to_double(circle.squared_radius());

            const bool isUp = [&] {
                auto orient = curve.orientation();
                bool dir_right = ((curve.is_directed_right()) != 0);
                CGAL_precondition(orient != CGAL::COLLINEAR);
                return ((orient == CGAL::COUNTERCLOCKWISE && !dir_right)
                    || (orient == CGAL::CLOCKWISE && dir_right));
            }();
            const double xJump = (xTgt - xSrc) / n;
            double x, y;
            double disc;
            if(oi.empty() || oi.back() != std::make_pair(xSrc, ySrc))
                oi.emplace_back(xSrc, ySrc); // The left point.
            for(unsigned int i = 1; i < n; ++i) {
                x = xSrc + xJump * i;
                disc = sqrRad - CGAL::square(x - xCenter);
                if(disc < 0) disc = 0;
                if(isUp) y = yCenter + std::sqrt(disc);
                else y = yCenter - std::sqrt(disc);

                oi.emplace_back(x, y);
            }
            oi.emplace_back(xTgt, yTgt); // The right point.
        };

        std::vector<std::pair<double, double>> poly4;

        for(auto it = poly.curves_begin(); it != poly.curves_end(); ++it)
            approximate(*it, poly4, 100);
#elif 0
        poly.approximate(std::back_inserter(poly4), 100);
#elif 0

        using CGAL::to_double;

        auto approximate = Traits{}.approximate_2_object();
        std::vector<AP> poly2;
        for(auto it = poly.curves_begin(); it != poly.curves_end(); ++it) {
            // if(it->is_linear()) {
            //     const auto& src = it->source();
            //     const auto& tgt = it->target();
            //     if(poly2.empty())
            //         poly2.emplace_back(to_double(src.x()), to_double(src.y()));
            //     poly2.emplace_back(to_double(tgt.x()), to_double(tgt.y()));
            // } else {
            qInfo() << (it->is_circular() ? "is_circular" : "")
                    << (it->is_linear() ? "is_linear" : "")
                    << (it->is_vertical() ? "is_vertical" : "")
                    << (it->is_directed_right() ? "is_directed_right" : "");
            approximate(*it, 0.0001, std::back_inserter(poly2));
            // }
        }

#else
        auto ppath = ConstructPath(poly);
        pf = ppath.toSubpathPolygons(QTransform::fromScale(100, 100)).front();
        pf = QTransform::fromScale(1. / 100, 1. / 100).map(pf);
        Polygon poly3;
        for(auto&& pt: pf)
            poly3.push_back({pt.x(), pt.y()});

            // for(auto it = ++pf.rbegin(), rend = --pf.rend(); it != rend; ++it)
            //     poly3.push_back({it->x(), it->y()});
#endif

        // Polygon poly3;
        // Point pt{};
        // for(auto it = poly2.begin(); it != poly2.end(); ++it) {
        //     auto x = it->x(), y = it->y();
        //     // auto [x, y] = *it;
        //     if(Point p{x, y}; pt != p) {
        //         poly3.push_back(pt = p);
        //         pf.push_back({x, y});
        //     }
        // }

        // Polygon_set_2 c;
        // auto /*Polygon_with_holes_2*/ offset = CGAL::approximated_offset_2(poly3, val / 2, 0.0001);
        // TestPolygon(offset.outer_boundary(), {Qt::green, 1.0});

        for(auto mp = CGAL::Polygon_repair::repair(poly3); auto&& pwh: mp.polygons_with_holes()) {
            auto /*Polygon_with_holes_2*/ offset = CGAL::approximated_offset_2(pwh, val / 2, 0.0001);
            TestPolygon(offset.outer_boundary(), {Qt::green, 1.0});
        }

        {
            // CGAL::draw(poly3);
            // CGAL::Basic_viewer_qt basicViewer{{}};

            // // Step 2: Instantiate a Qt_widget_layer to draw on top of the widget
            // CGAL::Qt_widget_layer<Qt_widget_Polygon_with_holes_2> layer;
            // layer.set_line_color(Qt::blue);

            // // Step 3: Add the layer to the widget
            // basicViewer.add_line(&layer);

            // // Step 4: Create a vector to store polygons
            // PolygonVector polygons;

            // // Step 5: Add polygons to the vector
            // // You can either construct the polygons manually or read them from a file

            // polygons.push_back(offset);

            // // Step 6: Draw the polygons on the widget
            // for(const auto& polygon: polygons)
            //     layer.add_polygon_with_holes(polygon);

            // // Show the widget
            // basicViewer.show();
        }

    } catch(std::exception& e) {
        auto scene = App::grView().scene();
        App::grView().scene()->addPolygon(pf, {Qt::yellow, 0.1})->setZValue(std::numeric_limits<double>::max());
        qCritical() << e.what();
    }
}

Polygon_2 ConstructPolygon(const Circle_2& circle) {
    Traits traits;
    Curve_2 curve{circle};
    Polygon_2 pgn;
#if CGAL_VER > 5
    // Subdivide the circle into two x-monotone arcs and construct the polygon.
    traits.make_x_monotone_2_object()(curve,
        CGAL::dispatch_or_drop_output<X_monotone_curve_2>(std::back_inserter(pgn)));
#else
    // Subdivide the circle into two x-monotone arcs.
    std::list<CGAL::Object> objects;
    traits.make_x_monotone_2_object()(curve, std::back_inserter(objects));
    CGAL_assertion(objects.size() == 2);
    // Construct the polygon.
    X_monotone_curve_2 arc;
    std::list<CGAL::Object>::iterator iter;
    for(iter = objects.begin(); iter != objects.end(); ++iter) {
        CGAL::assign(arc, *iter);
        pgn.push_back(arc);
    }
#endif
    assert(pgn.size() == 2);
    return pgn;
}
Polygon_2 ConstructPolygon(
    const Point_2& p1, const Point_2& p2,
    const Point_2& p3, const Point_2& p4) {
    Polygon_2 pgn;
    pgn.push_back(X_monotone_curve_2{p1, p2});
    pgn.push_back(X_monotone_curve_2{p2, p3});
    pgn.push_back(X_monotone_curve_2{p3, p4});
    pgn.push_back(X_monotone_curve_2{p4, p1});
    return pgn;
}

void construct_arc(const Point_2& ps, const Point_2& pt, const Point_2& center,
    double bulge, double rad, bool ccw, Polygon_2& pgn) {

#if 0
    using FT = Kernel::FT;
    using Arc_point_2 = Traits::Point_2;
    Traits traits;
    Traits::Make_x_monotone_2 make_x_monotone = traits.make_x_monotone_2_object();

#if 0

    Circle_2 supp_circ;

    const FT x_coord = ((ps.x() + pt.x()) / 2);
    const FT y_coord = ((ps.y() + pt.y()) / 2);
    const FT sqr_rad = rad * rad; // CGAL::squared_distance(ps, pt) / 4;

    supp_circ = Circle_2{center, sqr_rad, !ccw ? CGAL::COUNTERCLOCKWISE : CGAL::CLOCKWISE};

    Curve_2 circ_arc(supp_circ,
        Arc_point_2(ps.x(), ps.y()),
        Arc_point_2(pt.x(), pt.y()));

#else
    Circle_2 supp_circ{ps, pt, bulge};
    if(bulge < 0) supp_circ = supp_circ.opposite();
    Curve_2 circ_arc{
        supp_circ,
        Arc_point_2{ps.x(), ps.y()},
        Arc_point_2{pt.x(), pt.y()}
    };
#endif

    // Разбиение дуги на x-монотонные поддуги (может быть не более трех поддуг)
    // и добавление их к многоугольнику.
    X_monotone_curve_2 obj_vec[3];
    aut* obj_begin = obj_vec;
    aut* obj_end = make_x_monotone(circ_arc, obj_begin);

    X_monotone_curve_2 cv1;

    Polygon_2 dbgPgn;

    for(auto&& obj: std::span{obj_begin, obj_end})
        if(CGAL::assign(cv1, obj) && cv1.is_circular()) {
            pgn.push_back(cv1);
            dbgPgn.push_back(cv1);
        }

    auto gi = new Gi::PPath{ConstructPath(dbgPgn)};
    gi->setZValue(std::numeric_limits<double>::max());
    gi->setPen({Qt::white, 1.0});
    gi->setVisible(true);
    App::grView().scene()->addItem(gi);

#endif
}

void test___() {
#if 0
        try {
            // clang-format off
            S.join(ConstructPolygon(Circle_2{Point_2{0.5, 0.5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{5.5, 0.5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{5, 5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{1, 5}, 1}));
            S.join(ConstructPolygon(Point_2{1, 0}, Point_2{5, 0}, Point_2{5, 2}, Point_2{1, 2}));
            S.join(ConstructPolygon(Point_2{1, 4}, Point_2{5, 4}, Point_2{5, 6}, Point_2{1, 6}));
            S.join(ConstructPolygon(Point_2{0, 1}, Point_2{2, 1}, Point_2{2, 5}, Point_2{0, 5}));
            S.join(ConstructPolygon(Point_2{4, 1}, Point_2{6, 1}, Point_2{6, 5}, Point_2{4, 5}));
            S.join(ConstructPolygon(Circle_2{Point_2{8, 2.5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{9, 2.5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{10, 2.5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{9, 5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{3, 1.5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{3, 4.5}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{1.5, 3}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{4.5, 3}, 1}));

            // Invert the above and "capture" them into a box
            // This allow to test for CW polygons with various arc configuration
            S.complement();
            S.intersection(ConstructPolygon(Point_2{-1, -1}, Point_2{13, -1}, Point_2{13, 7}, Point_2{-1, 7}));

            // Insert a copy of the first polygons, placed above the previous box
            // This allow to test for CCW polygons with various arc configuration
            S.join(ConstructPolygon(Circle_2{Point_2{0.5, 0.5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{5.5, 0.5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{5, 5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{1, 5 + 10}, 1}));
            S.join(ConstructPolygon(Point_2{1, 0 + 10}, Point_2{5, 0 + 10}, Point_2{5, 2 + 10}, Point_2{1, 2 + 10}));
            S.join(ConstructPolygon(Point_2{1, 4 + 10}, Point_2{5, 4 + 10}, Point_2{5, 6 + 10}, Point_2{1, 6 + 10}));
            S.join(ConstructPolygon(Point_2{0, 1 + 10}, Point_2{2, 1 + 10}, Point_2{2, 5 + 10}, Point_2{0, 5 + 10}));
            S.join(ConstructPolygon(Point_2{4, 1 + 10}, Point_2{6, 1 + 10}, Point_2{6, 5 + 10}, Point_2{4, 5 + 10}));
            S.join(ConstructPolygon(Circle_2{Point_2{8, 2.5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{9, 2.5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{10, 2.5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{9, 5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{3, 1.5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{3, 4.5 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{1.5, 3 + 10}, 1}));
            S.join(ConstructPolygon(Circle_2{Point_2{4.5, 3 + 10}, 1}));

            // Insert the resulting polygons into the graphics scene
            S.polygons_with_holes(boost::make_function_output_iterator([&scene](const Polygon_with_holes_2& pgn) {
                if(!pgn.is_unbounded())
                    scene.addPath(construct_path(pgn.outer_boundary()), QPen(Qt::green, 0.0), Qt::darkGreen);

                Polygon_with_holes_2::Hole_const_iterator current = pgn.holes_begin();
                Polygon_with_holes_2::Hole_const_iterator end = pgn.holes_end();
                while(current != end) {
                    scene.addPath(construct_path(*current), QPen(Qt::red, 0.0), Qt::darkRed);
                    current++;
                }
            }));

            // clang-format on


            // And show the result
            QGraphicsView view;
            view.setBackgroundBrush(QColor("#073642"));
            view.setInteractive(true);
            view.setDragMode(QGraphicsView::ScrollHandDrag);
            scene.setSceneRect(scene.itemsBoundingRect());
            view.setScene(&scene);
            view.fitInView(scene.sceneRect(), Qt::KeepAspectRatio);
            view.scale(7, -7); // Revert Y-axis

            view.show();
            return app.exec();
            Polygon_set_2 clipper;
            //            if (path_2.is_simple()) {
            //            } else {
            //            }
            //            for (int i = 0; i < path_.size() - 1; ++i) {
            //                Linear_polygon p;
            //                p.push_back(toPoint(path_[i]));
            //                p.push_back(toPoint(path_[i + 1]));
            //                clipper.join(CGAL::approximated_offset_2(p, size * dScale, 0.00001));
            //            }
            for(auto& var: PATH) {
                if(!var.isArc) {
                    Linear_polygon p;
                    p.push_back(toPoint2(var.p1));
                    p.push_back(toPoint2(var.p2));
                    Polygon_with_holes_2 pp = CGAL::approximated_offset_2(p, size, 0.00001);
                    //                    clipper.join(pp);
                } else {

                    QLineF l1(toQPointF(var.c), toQPointF(var.p1));
                    QLineF l2(toQPointF(var.c), toQPointF(var.p2));
                    Polygon_set_2 c;

                    if(!qFuzzyCompare(var.angle1, var.angle2)) {
                        {
                            if(var.multi) {
                                App::scene()->addLine(l1, QPen(Qt::red, 0.0));
                                App::scene()->addLine(l2, QPen(Qt::red, 0.0));
                            } else {
                                App::scene()->addLine(l1, QPen(Qt::green, 0.0));
                                App::scene()->addLine(l2, QPen(Qt::green, 0.0));
                            }
                        }
                        // continue;

                        Point_2 p1(toPoint2(var.p1));
                        Point_2 p2(toPoint2(var.p2));
                        Point_2 c0(toPoint2(var.c));

                        // l1.setLength(l1.length() * 2);
                        l2.setLength(l1.length());
                        auto sqr_rad = CGAL::squared_distance(p1, Point_2(p1.x() - c0.x() + p1.x(), p1.y() - c0.y() + p1.y()));
                        // qDebug() << "squared_distance" << CGAL::to_double(sqr_rad) << (l1.length() * l1.length());
                        if(!var.isCcw) {
                            std::swap(p1, p2);
                            std::swap(l1, l2);
                        }
                        Curve_2 circ_arc[2];
                        Point_2 pt[4];
                        Arc_point_2 pa[4];
                        if(1) {
                            double rr = l1.length() + size;
                            auto supp_circ = Circle_2{c0, rr * rr, CGAL::COUNTERCLOCKWISE};
                            circ_arc[0] = Curve_2(supp_circ, Arc_point_2(p1.x(), p1.y()), Arc_point_2(p2.x(), p2.y()));
                            // circ_arc[0] = Curve_2(supp_circ, Arc_point_2(l1.p2().x(), l1.p2().y()), Arc_point_2(l2.p2().x(), l2.p2().y()));
                            std::list<CGAL::Object> objects;
                            Traits().make_x_monotone_2_object()(circ_arc[0], std::back_inserter(objects));
                            // CGAL_assertion(objects.size() == 2);
                            X_monotone_curve_2 arc;
                            std::list<CGAL::Object>::iterator iter;
                            Polygon_2 pgn;
                            for(iter = objects.begin(); iter != objects.end(); ++iter) {
                                CGAL::assign(arc, *iter);
                                pgn.push_back(arc);
                            }

                            // Polygon_with_holes_2 pp = CGAL::minkowski_sum_by_full_convolution_2(pgn, CirclePath2(size * 2));

                            pa[0] = circ_arc[0].source();
                            pa[1] = circ_arc[0].target();
                            Point_2 sp1(CGAL::to_double(circ_arc[0].source().x()), CGAL::to_double(circ_arc[0].source().y()));
                            Point_2 tp1(CGAL::to_double(circ_arc[0].target().x()), CGAL::to_double(circ_arc[0].target().y()));
                            pgn.push_back(X_monotone_curve_2(tp1, c0));
                            pgn.push_back(X_monotone_curve_2(c0, sp1));
                            c.join(pgn);
                            pt[0] = sp1;
                            pt[1] = tp1;
                        }
                        if(1) {
                            double rr = l1.length() - size;
                            auto supp_circ = Circle_2{c0, rr * rr, CGAL::COUNTERCLOCKWISE};
                            circ_arc[1] = Curve_2{supp_circ, pa[0], pa[1]};
                            // Curve_2 circ_arc(supp_circ, Arc_point_2(l1.p2().x(), l1.p2().y()), Arc_point_2(l2.p2().x(), l2.p2().y()));
                            std::list<CGAL::Object> objects;
                            Traits().make_x_monotone_2_object()(circ_arc[1], std::back_inserter(objects));
                            // CGAL_assertion(objects.size() == 2);
                            X_monotone_curve_2 arc;
                            std::list<CGAL::Object>::iterator iter;
                            Polygon_2 pgn;
                            for(iter = objects.begin(); iter != objects.end(); ++iter) {
                                CGAL::assign(arc, *iter);
                                pgn.push_back(arc);
                            }

                            Point_2 sp2(CGAL::to_double(circ_arc[0].source().x()), CGAL::to_double(circ_arc[0].source().y()));
                            Point_2 tp2(CGAL::to_double(circ_arc[0].target().x()), CGAL::to_double(circ_arc[0].target().y()));
                            pgn.push_back(X_monotone_curve_2(tp2, c0));
                            pgn.push_back(X_monotone_curve_2(c0, sp2));
                            c.difference(pgn);
                            pt[2] = sp2;
                            pt[3] = tp2;
                        }
                        auto arc = [&c](Point_2 ps, Point_2 pt) {
                            Traits traits;
                            const auto x_coord = ((ps.x() + pt.x()) / 2);
                            const auto y_coord = ((ps.y() + pt.y()) / 2);
                            const auto sqr_rad = CGAL::squared_distance(ps, pt) / 4;
                            Circle_2 supp_circ(Point_2{x_coord, y_coord}, sqr_rad, CGAL::COUNTERCLOCKWISE);
                            Curve_2 circ_arc(supp_circ,
                                Arc_point_2(ps.x(), ps.y()),
                                Arc_point_2(pt.x(), pt.y()));
                            std::list<CGAL::Object> objects;
                            Traits().make_x_monotone_2_object()(circ_arc, std::back_inserter(objects));
                            X_monotone_curve_2 arc;
                            std::list<CGAL::Object>::iterator iter;
                            Polygon_2 pgn;
                            for(iter = objects.begin(); iter != objects.end(); ++iter) {
                                CGAL::assign(arc, *iter);
                                pgn.push_back(arc);
                            }

                            c.join(pgn);
                        };
                        // arc(pt[0], pt[1]);
                        // arc(pt[2], pt[3]);
                    } else {
                        c.join(/* */ CirclePath2(l1.length() * 2 + size * 2, toPoint2(var.c)));
                        c.difference(CirclePath2(l1.length() * 2 - size * 2, toPoint2(var.c)));
                    }
                    //                    c.polygons_with_holes(boost::make_function_output_iterator([&clipper](const Polygon_with_holes_2& pgn) {
                    //                        clipper.join(pgn);
                    //                    }));
                    clipper.join(c);
                }
            }

            QPainterPath pp;
            clipper.polygons_with_holes(boost::make_function_output_iterator([&pp](const Polygon_with_holes_2& pgn) {
                if(!pgn.is_unbounded()) {
                    //            auto i = App::scene()->addPath(construct_path(pgn.outer_boundary()), QPen(Qt::green, 0.0), Qt::darkGreen);
                    // i->setZValue(std::numeric_limits<double>::max());
                    pp.addPath(ConstructPath(pgn.outer_boundary()));
                }

                Polygon_with_holes_2::Hole_const_iterator current = pgn.holes_begin();
                Polygon_with_holes_2::Hole_const_iterator end = pgn.holes_end();
                while(current != end) {
                    //            auto i = App::scene()->addPath(construct_path(*current), QPen(Qt::red, 0.0), Qt::darkRed);
                    // i->setZValue(std::numeric_limits<double>::max());
                    pp.addPath(ConstructPath(*current));
                    current++;
                }
            }));
            // auto i = App::scene()->addPath(pp, QPen(Qt::red, 0.0), Qt::darkRed);
            // auto i = App::grView().scene()->addPath(pp, QPen(Qt::yellow, 0.0), QColor(255, 255, 0, 150));
            qDebug() << "ok" << path_.size();
        } catch(CGAL::Error_exception& e) {
            qDebug() << "wtf" << QString::fromStdString(e.message());
        } catch(...) {
            qDebug() << "wtf" << path_.size();
        }
#endif
}
std::tuple<Point_2, Point_2, double> arcToBulge(const QPointF& center, double startAngle, double endAngle, double radius) { // FIXME

    const auto startPoint = QPointF{cos(startAngle), sin(startAngle)} * radius + center; //   QLineF::fromPolar(radius, startAngle).p2() + ~center;
    const auto endPoint = QPointF{cos(endAngle), sin(endAngle)} * radius + center;       // QLineF::fromPolar(radius, endAngle).p2() + ~center;
    constexpr double pi2 = std::numbers::pi * 2;
    // const double a = std::fmod((pi2 + (endAngle - startAngle)), pi2) / 4.0;
    // const double bulge = std::sin(a) / std::cos(a);

    double a = std::fmod((pi2 + (endAngle - startAngle)), pi2) / 4.0;
    double bulge = std::sin(a) / std::cos(a);

    if(std::abs(bulge) > 1.0) {
        a = std::fmod((pi2 + (startAngle - endAngle)), pi2) / 4.0;
        bulge = -std::sin(a) / std::cos(a);
    }

    return std::tuple{
        Point_2{startPoint.x(), startPoint.y()},
        Point_2{endPoint.x(),   endPoint.y()  },
        bulge
    };
}

// ------ return signed area under the linear segment (P1, P2)
auto area(Traits::Point_2 const& P1, Traits::Point_2 const& P2) {
    auto const dx = CGAL::to_double(P1.x()) - CGAL::to_double(P2.x());
    auto const sy = CGAL::to_double(P1.y()) + CGAL::to_double(P2.y());
    return dx * sy / 2;
}

// ------ return signed area under the circular segment (P1, P2, C)
auto area(Traits::Point_2 const& P1, Traits::Point_2 const& P2, Circle_2 const& C) {
    auto const dx = CGAL::to_double(P1.x()) - CGAL::to_double(P2.x());
    auto const dy = CGAL::to_double(P1.y()) - CGAL::to_double(P2.y());
    auto const squaredChord = dx * dx + dy * dy;
    auto const chord = std::sqrt(squaredChord);
    auto const squaredRadius = CGAL::to_double(C.squared_radius());
    auto const areaSector = squaredRadius * std::asin(std::min(1.0, chord / (std::sqrt(squaredRadius) * 2)));
    auto const areaTriangle = chord * std::sqrt(std::max(0.0, squaredRadius * 4 - squaredChord)) / 4;
    auto const areaCircularSegment = areaSector - areaTriangle;
    return area(P1, P2) + C.orientation() * areaCircularSegment;
}

// ------ return signed area under the X-monotone curve
auto area(X_monotone_curve_2 const& XCV) {
    if(XCV.is_linear())
        return area(XCV.source(), XCV.target());
    else if(XCV.is_circular())
        return area(XCV.source(), XCV.target(), XCV.supporting_circle());
    else
        return 0.0;
}

// ------ return area of the simple polygon
Kernel::FT area(Polygon_2 const& P) {
    auto res = 0.0;
    for(auto it = P.curves_begin(); it != P.curves_end(); ++it) res += area(*it);
    return res;
}

// ------ return area of the polygon with (optional) holes
Kernel::FT area(Polygon_with_holes_2 const& P) {
    auto res = area(P.outer_boundary());
    for(auto it = P.holes_begin(); it != P.holes_end(); ++it) res += area(*it);
    return res;
}

void construct_arc(const Point_2& ps, const Point_2& pt, double bulge, Polygon_2& pgn) {
    using FT = Kernel::FT;
    using Arc_point_2 = Traits::Point_2;
    auto make_x_monotone = Traits{}.make_x_monotone_2_object();

    Circle_2 supp_circ{ps, pt, bulge};
    if(bulge < 0) supp_circ = supp_circ.opposite();
    Curve_2 circ_arc{
        supp_circ,
        Arc_point_2{ps.x(), ps.y()},
        Arc_point_2{pt.x(), pt.y()}
    };
#if CGAL_VER > 5
    // Разделим окружность на x-монотонные дуги и добавление их к многоугольнику.
    make_x_monotone(circ_arc, CGAL::dispatch_or_drop_output<X_monotone_curve_2>(std::back_inserter(pgn)));
#else
    // Разбиение дуги на x-монотонные поддуги (может быть не более трех поддуг)
    // и добавление их к многоугольнику.
    X_monotone_curve_2 obj_vec[3];
    auto* obj_begin = obj_vec;
    auto* obj_end = make_x_monotone(circ_arc, obj_begin);

    for(X_monotone_curve_2 cv1; auto&& obj: std::span{obj_begin, obj_end})
        // if(CGAL::assign(cv1, obj) && cv1.is_circular())
        // pgn.push_back(cv1);
        pgn.push_back(obj);
#endif
}

void TestPolygon(const Polygon_2& pgn, const QPen& pen) {
    auto gi = new Gi::PPath{ConstructPath(pgn)};
    gi->setZValue(std::numeric_limits<double>::max());
    gi->setPen(pen);
    gi->setVisible(true);
    App::grView().scene()->addItem(gi);
}

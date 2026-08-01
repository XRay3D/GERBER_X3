/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 ********************************************************************************/
#pragma once

#include "geometry.h"

namespace geo {

struct Span {
    //     PointF NearestPointNotOnSpan(const PointF& p) const;
    //     double Parameter(const PointF& p) const;
    //     PointF NearestPointToSpan(const Span& p, double& d) const;

    //     static const PointF null_point;
    //     static const Vertex null_vertex;

    // public:
    geo::PointF pt;
    geo::Vertex vx;
    bool m_start_span{};
    //     Span();
    //     Span(const PointF& p, const Vertex& v, bool start_span = false)
    //         : m_start_span(start_span)
    //         , pt(p)
    //         , vx(v)
    //     {}
    //     PointF NearestPoint(const PointF& p) const;
    //     PointF NearestPoint(const Span& p, double* d = NULL) const;
    //     void GetBox(CBox2D& box);
    //     double IncludedAngle() const;
    //     double GetArea() const;
    //     bool On(const PointF& p, double* t = NULL) const;
    //     PointF MidPerim(double d) const;
    //     PointF MidParam(double param) const;
    //     double Length() const;
    //     PointF GetVector(double fraction) const;
    //     void Intersect(
    //         const Span& s,
    //         std::list<PointF>& pts
    //         ) const;  // finds all the intersection points between two spans

    geo::PointF NearestPointNotOnSpan(const geo::PointF& p) const;

    geo::PointF NearestPoint(const geo::PointF& p) const;

    geo::PointF MidPerim(double d) const;
    geo::PointF MidParam(double param) const;

    geo::PointF NearestPointToSpan(const Span& p, double& d) const;
    geo::PointF NearestPoint(const Span& p, double* d) const;
    QRectF boundingRect();

    double IncludedAngle() const;
    double GetArea() const;
    double Parameter(const geo::PointF& p) const;
    bool On(const geo::PointF& p, double* t) const;
    double Length() const;
    geo::PointF GetVector(double fraction) const;
    // void Intersect(const Span& s, std::list<geo::PointF>& pts) const {
    //     // finds all the intersection points between two spans and puts them in the given list
    //     geoff_geometry::geo::PointF pInt1, pInt2;
    //     double t[4];
    //     int num_int = MakeSpan(*this).Intof(MakeSpan(s), pInt1, pInt2, t);
    //     if(num_int > 0) {
    //         pts.emplace_back(pInt1.x(), pInt1.y());
    //     }
    //     if(num_int > 1) {
    //         pts.emplace_back(pInt2.x(), pInt2.y());
    //     }
    // }
};

double IncludedAngle(const geo::PointF& v0, const geo::PointF& v1, int dir);

} // namespace geo
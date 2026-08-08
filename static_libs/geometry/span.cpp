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
#include "span.h"

using namespace geo;
// using namespace circle;

double IncludedAngle(const PointF& v0, const PointF& v1, int dir) {
    // returns the absolute included angle between 2 vectors in the direction of dir ( 1=acw  -1=cw)
    double inc_ang = PointF::dotProduct(v0, v1);
    if(inc_ang > 1. - 1.0e-10) return 0;

    if(inc_ang < -1. + 1.0e-10) {
        inc_ang = pi;
    } else { // dot product,   v1 . v2  =  cos ang
        if(inc_ang > 1.0)
            inc_ang = 1.0;

        inc_ang = acos(inc_ang); // 0 to pi radians

        if(dir * PointF::crossProduct(v0, v1) < 0)
            inc_ang = 2 * pi - inc_ang; // cp
    }
    return dir * inc_ang;
}

static int GetQuadrant(const PointF& v) {
    // 0 = [+,+], 1 = [-,+], 2 = [-,-], 3 = [+,-]
    if(v.x() > 0) {
        if(v.y() > 0) {
            return 0;
        }
        return 3;
    }
    if(v.y() > 0) {
        return 1;
    }
    return 2;
}

static PointF QuadrantEndPoint(int i) {
    if(i > 3) i -= 4;
    switch(i) {
    case 0 : return {0.0, +1.0};
    case 1 : return {-1.0, 0.0};
    case 2 : return {0.0, -1.0};
    default: return {+1.0, 0.0};
    }
}

PointF Span::NearestPointNotOnSpan(const PointF& p) const {
    auto arc = this->arc();
    if(!arc) {
        PointF Vs{pe - vs.pt};
        Vs.normalize();
        double dp = PointF::dotProduct((p - vs.pt), Vs);
        return (Vs * dp) + vs.pt;
    }
    double r = p.dist(arc->center);
    if(r < PointF::tolerance) return vs.pt;
    PointF vc{arc->center - p};
    return p + vc * ((r - arc->radius) / r);
}

PointF Span::NearestPoint(const PointF& p) const {
    PointF np = NearestPointNotOnSpan(p);
    double t = Parameter(np);
    if(0.0 <= t && t <= 1.0) return np;
    double d1 = p.dist(vs.pt);
    double d2 = p.dist(pe);
    return (d1 < d2) ? vs.pt : pe;
}

PointF Span::MidPerim(double d) const {
    /// returns a point which is 0-d along span
    auto arc = this->arc();
    if(!arc) {
        PointF v{pe - vs.pt};
        v.normalize();
        return v * d + vs.pt;
    }
    // длина дуги = радиус * угол, отсюда и угол, на который нужно повернуть
    // радиус-вектор начала; знак -- направление обхода
    PointF v{vs.pt - arc->center};
    v.Rotate(d * (arc->theta < 0.0 ? -1.0 : 1.0) / arc->radius);
    return v + arc->center;
}

PointF Span::MidParam(double param) const {
    /// returns a point which is 0-1 along span
    if(qFuzzyIsNull(param)) return vs.pt;

    if(qFuzzyIsNull(param - 1.0)) return pe;

    auto arc = this->arc();
    if(!arc) return PointF{pe - vs.pt} * param + vs.pt;
    return arc->pointAt(param);
}

PointF Span::NearestPointToSpan(const Span& p, double& d) const {
    PointF midpoint = MidParam(0.5);
    PointF np = p.NearestPoint(vs.pt);
    PointF best_point = vs.pt;
    double dist = np.dist(vs.pt);
    if(p.m_start_span) {
        dist -= (PointF::tolerance * 2); // give start of curve most priority
    }
    PointF npm = p.NearestPoint(midpoint);
    double dm = npm.dist(midpoint)
        - PointF::tolerance; // lie about midpoint distance to give midpoints priority
    if(dm < dist) {
        dist = dm;
        best_point = midpoint;
    }
    PointF np2 = p.NearestPoint(pe);
    double dp2 = np2.dist(pe);
    if(dp2 < dist) {
        dist = dp2;
        best_point = pe;
    }
    d = dist;
    return best_point;
}

PointF Span::NearestPoint(const Span& p, double* d) const {
    double best_dist;
    PointF best_point = this->NearestPointToSpan(p, best_dist);

    // try the other way round too
    double best_dist2;
    PointF best_point2 = p.NearestPointToSpan(*this, best_dist2);
    if(best_dist2 < best_dist) {
        best_point = NearestPoint(best_point2);
        best_dist = best_dist2;
    }

    if(d) {
        *d = best_dist;
    }
    return best_point;
}

QRectF Span::boundingRect() {
    QPolygonF box{vs.pt, pe};

    if(auto arc = this->arc()) {
        // дуга выпирает за хорду: добавляем те крайние по осям точки
        // окружности, что попали в её размах
        PointF vStart = vs.pt - arc->center;
        PointF vEnd = pe - arc->center;
        int qs = GetQuadrant(vStart);
        int qe = GetQuadrant(vEnd);
        if(arc->theta > 0.0) std::swap(qs, qe); // против часовой -- квадранты в обратном порядке

        if(qe < qs) qe = qe + 4;

        for(int i = qs; i < qe; i++)
            box.emplace_back(arc->center + QuadrantEndPoint(i) * arc->radius);
    }
    return box.boundingRect();
}

double Span::IncludedAngle() const {
    auto arc = this->arc();
    return arc ? arc->theta : 0.0;
}

double Span::GetArea() const {
    if(auto arc = this->arc()) {
        const auto& c = arc->center;
        return 0.5
            * ((c.x() - vs.x()) * (c.y() + vs.y())
                - (c.x() - pe.x()) * (c.y() + pe.y())
                - arc->theta * arc->radius * arc->radius);
    }

    return 0.5 * (pe.x() - vs.x()) * (vs.y() + pe.y());
}

double Span::Parameter(const PointF& p) const {
    auto arc = this->arc();
    if(!arc) {
        PointF v0{p - vs.pt};
        PointF v{pe - vs.pt};
        double length = v.length();
        v.normalize();
        return PointF::dotProduct(v, v0) / length;
    }
    // доля пройденного угла: точка должна лежать на окружности дуги
    const double a = std::atan2(p.y() - arc->center.y(), p.x() - arc->center.x());
    double delta = std::remainder(a - arc->startAngle, 2.0 * pi);
    if(arc->theta > 0.0) {
        if(delta < 0.0) delta += 2.0 * pi;
    } else {
        if(delta > 0.0) delta -= 2.0 * pi;
    }
    return delta / arc->theta;
}

bool Span::On(const PointF& p, double* t) const {
    if(p != NearestPoint(p)) {
        return false;
    }
    if(t) {
        *t = Parameter(p);
    }
    return true;
}

double Span::Length() const {
    auto arc = this->arc();
    return arc ? arc->length() : vs.pt % pe;
}

PointF Span::GetVector(double fraction) const {
    /// returns the direction vector at point which is 0-1 along span
    auto arc = this->arc();
    if(!arc) {
        PointF v{pe - vs.pt};
        v.normalize();
        return v;
    }

    PointF p = MidParam(fraction);
    PointF v{p - arc->center};
    v.normalize();
    // касательная -- радиус, повёрнутый на 90 градусов в сторону обхода
    if(arc->theta > 0.0) return PointF(-v.y(), v.x());
    return PointF(v.y(), -v.x());
}

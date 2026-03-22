#include "span.h"
#include "curve.h"

double IncludedAngle(const PointF& v0, const PointF& v1, int dir) {
    // returns the absolute included angle between 2 vectors in the direction of dir ( 1=acw  -1=cw)
    double inc_ang = PointF::dotProduct(v0, v1);
    if(inc_ang > 1. - 1.0e-10) {
        return 0;
    }
    if(inc_ang < -1. + 1.0e-10) {
        inc_ang = pi;
    } else { // dot product,   v1 . v2  =  cos ang
        if(inc_ang > 1.0) {
            inc_ang = 1.0;
        }
        inc_ang = acos(inc_ang); // 0 to pi radians

        if(dir * PointF::crossProduct(v0, v1) < 0) {
            inc_ang = 2 * pi - inc_ang; // cp
        }
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
    if(!vx.type) {
        PointF Vs{vx.pt - pt};
        Vs.normalize();
        double dp = PointF::dotProduct((p - pt), Vs);
        return (Vs * dp) + pt;
    } else {
        double radius = pt.dist(vx.center);
        double r = p.dist(vx.center);
        if(r < PointF::tolerance) return pt;
        PointF vc{vx.center - p};
        return p + vc * ((r - radius) / r);
    }
}

PointF Span::NearestPoint(const PointF& p) const {
    PointF np = NearestPointNotOnSpan(p);
    double t = Parameter(np);
    if(0.0 <= t && t <= 1.0) return np;
    double d1 = p.dist(pt);
    double d2 = p.dist(vx.pt);
    return (d1 < d2) ? pt : vx.pt;
}

PointF Span::MidPerim(double d) const {
    /// returns a point which is 0-d along span
    PointF p;
    if(vx.type == 0) {
        PointF vs{vx.pt - pt};
        vs.normalize();
        p = vs * d + pt;
    } else {
        PointF v{pt - vx.center};
        double radius = v.length();
        v.Rotate(d * int(vx.type) / radius);
        p = v + vx.center;
    }
    return p;
}

PointF Span::MidParam(double param) const {
    /// returns a point which is 0-1 along span
    if(qFuzzyIsNull(param)) return pt;

    if(qFuzzyIsNull(param - 1.0)) return vx.pt;

    PointF p;
    if(vx.type == 0) {
        PointF vs{vx.pt - pt};
        p = vs * param + pt;
    } else {
        PointF v{pt - vx.center};
        v.Rotate(param * IncludedAngle());
        p = v + vx.center;
    }
    return p;
}

PointF Span::NearestPointToSpan(const Span& p, double& d) const {
    PointF midpoint = MidParam(0.5);
    PointF np = p.NearestPoint(pt);
    PointF best_point = pt;
    double dist = np.dist(pt);
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
    PointF np2 = p.NearestPoint(vx.pt);
    double dp2 = np2.dist(vx.pt);
    if(dp2 < dist) {
        dist = dp2;
        best_point = vx.pt;
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
    QPolygonF box{pt, vx.pt};

    if(this->vx.type) {
        // arc, add quadrant points
        PointF vs = pt - vx.center;
        PointF ve = vx.pt - vx.center;
        int qs = GetQuadrant(vs);
        int qe = GetQuadrant(ve);
        if(vx.type == -1) {
            // swap qs and qe
            int t = qs;
            qs = qe;
            qe = t;
        }

        if(qe < qs) qe = qe + 4;

        double rad = vx.pt.dist(vx.center);

        for(int i = qs; i < qe; i++)
            box.emplace_back(vx.center + QuadrantEndPoint(i) * rad);
    }
    return box.boundingRect();
}

double Span::IncludedAngle() const {
    if(vx.type) {
        PointF vs{pt - vx.center};
        PointF ve{vx.pt - vx.center};
        vs.ry() *= -1.;
        ve.ry() *= -1.;

        if(vx.type == -1) {
            vs = -vs;
            ve = -ve;
        }
        vs.normalize();
        ve.normalize();

        return ::IncludedAngle(vs, ve, vx.type);
    }

    return 0.0;
}

double Span::GetArea() const {
    if(vx.type) {
        double angle = IncludedAngle();
        double radius = pt.dist(vx.center);
        return (
            0.5
            * ((vx.center.x() - pt.x()) * (vx.center.y() + pt.y())
                - (vx.center.x() - vx.pt.x()) * (vx.center.y() + vx.pt.y()) - angle * radius * radius));
    }

    return 0.5 * (vx.pt.x() - pt.x()) * (pt.y() + vx.pt.y());
}

double Span::Parameter(const PointF& p) const {
    double t;
    if(vx.type == 0) {
        PointF v0{p - pt};
        PointF vs{vx.pt - pt};
        double length = vs.length();
        vs.normalize();
        t = PointF::dotProduct(vs, v0);
        t = t / length;
    } else {
        // true if p lies on arc span sp (p must be on circle of span)
        PointF vs{pt - vx.center};
        PointF v{p - vx.center};
        vs.ry() *= -1.;
        v.ry() *= -1.;
        vs.normalize();
        v.normalize();
        if(vx.type == -1) {
            vs = -vs;
            v = -v;
        }
        double ang = ::IncludedAngle(vs, v, vx.type);
        double angle = IncludedAngle();
        t = ang / angle;
    }
    return t;
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
    if(vx.type)
        return std::abs(IncludedAngle()) * vx.radius();
    return pt % vx.pt;
}

PointF Span::GetVector(double fraction) const {
    /// returns the direction vector at point which is 0-1 along span
    if(vx.type == 0) {
        PointF v{vx.pt - pt};
        v.normalize();
        return v;
    }

    PointF p = MidParam(fraction);
    PointF v{p - vx.center};
    v.normalize();
    if(vx.type == 1) {
        return PointF(-v.y(), v.x());
    } else {
        return PointF(v.y(), -v.x());
    }
}

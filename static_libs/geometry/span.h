#pragma once

#include <QDebug>
#include <QLineF>

using namespace std::literals;

constexpr double length(const QPointF p1, const QPointF p2) {
    return QLineF{p1, p2}.length();
}

struct PointF : QPointF {
    static inline constexpr double tolerance = 0.001;
    using QPointF::QPointF;
    PointF(QPointF pt)
        : QPointF{pt} { }

    PointF& operator=(QPointF pt) {
        return static_cast<QPointF&>(*this) = pt, *this;
    }
    void Rotate(double cosa, double sina) { // rotate vector by angle
        double temp = -y() * sina + x() * cosa;
        ry() = x() * sina + cosa * y();
        rx() = temp;
    }
    void Rotate(double angle) {
        if(qFuzzyIsNull(angle)) return;
        Rotate(cos(angle), sin(angle));
    }

    double length() const { return sqrt(x() * x() + y() * y()); }

    double normalize() {
        double len = length();
        if(!qFuzzyIsNull(len)) *this = (*this) / len;
        return len;
    }

    double dist(const QPointF& p) const {
        QPointF d = p - *this;
        return sqrt(d.x() * d.x() + d.y() * d.y());
    }

    constexpr double operator%(const QPointF& p) const {
        return dist(p);
    }

    constexpr static inline qreal crossProduct(const QPointF& l, const QPointF& r) {
        return l.x() * r.x() - l.y() * r.y();
    }

    friend constexpr double operator*(const QPointF& l, const QPointF& r) { // dot product
        return dotProduct(l, r);
    }

    friend constexpr double operator^(const QPointF& l, const QPointF& r) { // cross product m0.m1.sin a = v0 ^ v1
        return crossProduct(l, r);
    }

    constexpr PointF crossV(const QPointF& v) const {
        return {
            y() * v.x() - x() * v.y(),
            x() * v.y() - y() * v.x(),
        };
    }
};

struct Vertex {
    PointF pt{}, center{};
    enum Type : int {
        Line = 0,
        Ccw = 1,
        Cw = -1,
    } type{};

    constexpr double radius() const {
        if(type) return length(pt, center);
        return std::nan("");
    }

    constexpr operator bool() const { return type != Line; }

    constexpr inline qreal x() const noexcept { return pt.x(); }

    constexpr inline qreal y() const noexcept { return pt.y(); }

    friend QDebug operator<<(QDebug dbg, const Vertex& v) { // cross product m0.m1.sin a = v0 ^ v1
        constexpr std::array types{
            "CW"sv,
            "LINE"sv,
            "CCW"sv,
        };
        return dbg.noquote() << "\nVertex(" << v.pt << v.center << types[v.type + 1] << ')';
    }
};

struct Span {
    //     PointF NearestPointNotOnSpan(const PointF& p) const;
    //     double Parameter(const PointF& p) const;
    //     PointF NearestPointToSpan(const Span& p, double& d) const;

    //     static const PointF null_point;
    //     static const Vertex null_vertex;

    // public:
    PointF pt;
    Vertex vx;
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

    PointF NearestPointNotOnSpan(const PointF& p) const;

    PointF NearestPoint(const PointF& p) const;

    PointF MidPerim(double d) const;
    PointF MidParam(double param) const;

    PointF NearestPointToSpan(const Span& p, double& d) const;
    PointF NearestPoint(const Span& p, double* d) const;
    QRectF boundingRect();

    double IncludedAngle() const;
    double GetArea() const;
    double Parameter(const PointF& p) const;
    bool On(const PointF& p, double* t) const;
    double Length() const;
    PointF GetVector(double fraction) const;
    // void Intersect(const Span& s, std::list<PointF>& pts) const {
    //     // finds all the intersection points between two spans and puts them in the given list
    //     geoff_geometry::PointF pInt1, pInt2;
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

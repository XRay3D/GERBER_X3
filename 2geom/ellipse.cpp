/** @file
 * @brief Ellipse shape
 *//*
 * Authors:
 *   Marco Cecchetti <mrcekets at gmail.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2008-2014 Authors
 *
 * This library is free software; you can redistribute it and/or
 * modify it either under the terms of the GNU Lesser General Public
 * License version 2.1 as published by the Free Software Foundation
 * (the "LGPL") or, at your option, under the terms of the Mozilla
 * Public License Version 1.1 (the "MPL"). If you do not alter this
 * notice, a recipient may use your version of this file under either
 * the MPL or the LGPL.
 *
 * You should have received a copy of the LGPL along with this library
 * in the file COPYING-LGPL-2.1; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 * You should have received a copy of the MPL along with this library
 * in the file COPYING-MPL-1.1
 *
 * The contents of this file are subject to the Mozilla Public License
 * Version 1.1 (the "License"); you may not use this file except in
 * compliance with the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * This software is distributed on an "AS IS" basis, WITHOUT WARRANTY
 * OF ANY KIND, either express or implied. See the LGPL or the MPL for
 * the specific language governing rights and limitations.
 */

#include <2geom/conicsec.h>
#include <2geom/ellipse.h>
#include <2geom/elliptical-arc.h>
#include <2geom/numeric/fitting-tool.h>
#include <2geom/numeric/fitting-model.h>

namespace Geom {

Ellipse::Ellipse(Geom::Circle const &c)
    : _center(c.center())
    , _rays(c.radius(), c.radius())
    , _angle(0)
{}

void Ellipse::setCoefficients(double A, double B, double C, double D, double E, double F)
{
    double den = 4*A*C - B*B;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing ellipse centre");
    }
    _center[X] = (B*E - 2*C*D) / den;
    _center[Y] = (B*D - 2*A*E) / den;

    // evaluate the a coefficient of the ellipse equation in normal form
    // E(x,y) = a*(x-cx)^2 + b*(x-cx)*(y-cy) + c*(y-cy)^2 = 1
    // where b = a*B , c = a*C, (cx,cy) == centre
    double num =   A * sqr(_center[X])
                 + B * _center[X] * _center[Y]
                 + C * sqr(_center[Y])
                 - F;


    //evaluate ellipse rotation angle
    _angle = std::atan2( -B, -(A - C) )/2;

    // evaluate the length of the ellipse rays
    double sinrot, cosrot;
    sincos(_angle, sinrot, cosrot);
    double cos2 = cosrot * cosrot;
    double sin2 = sinrot * sinrot;
    double cossin = cosrot * sinrot;

    den = A * cos2 + B * cossin + C * sin2;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing 'rx' coefficient");
    }
    double rx2 = num / den;
    if (rx2 < 0) {
        THROW_RANGEERROR("rx2 < 0, while computing 'rx' coefficient");
    }
    _rays[X] = std::sqrt(rx2);

    den = C * cos2 - B * cossin + A * sin2;
    if (den == 0) {
        THROW_RANGEERROR("den == 0, while computing 'ry' coefficient");
    }
    double ry2 = num / den;
    if (ry2 < 0) {
        THROW_RANGEERROR("ry2 < 0, while computing 'rx' coefficient");
    }
    _rays[Y] = std::sqrt(ry2);

    // the solution is not unique so we choose always the ellipse
    // with a rotation angle between 0 and PI/2
    makeCanonical();
}

Point Ellipse::initialPoint() const
{
    Coord sinrot, cosrot;
    sincos(_angle, sinrot, cosrot);
    Point p(ray(X) * cosrot + center(X), ray(X) * sinrot + center(Y));
    return p;
}


Affine Ellipse::unitCircleTransform() const
{
    Affine ret = Scale(ray(X), ray(Y)) * Rotate(_angle);
    ret.setTranslation(center());
    return ret;
}

Affine Ellipse::inverseUnitCircleTransform() const
{
    if (ray(X) == 0 || ray(Y) == 0) {
        THROW_RANGEERROR("a degenerate ellipse doesn't have an inverse unit circle transform");
    }
    Affine ret = Translate(-center()) * Rotate(-_angle) * Scale(1/ray(X), 1/ray(Y));
    return ret;
}


LineSegment Ellipse::axis(Dim2 d) const
{
    Point a(0, 0), b(0, 0);
    a[d] = -1;
    b[d] = 1;
    LineSegment ls(a, b);
    ls.transform(unitCircleTransform());
    return ls;
}

LineSegment Ellipse::semiaxis(Dim2 d, int sign) const
{
    Point a(0, 0), b(0, 0);
    b[d] = sgn(sign);
    LineSegment ls(a, b);
    ls.transform(unitCircleTransform());
    return ls;
}

Rect Ellipse::boundsExact() const
{
    auto const trans = unitCircleTransform();

    auto proj_bounds = [&] (Dim2 d) {
        // The dth coordinate function pulls back to trans[d] * x + trans[d + 2] * y + trans[d + 4]
        // in the coordinate system where the ellipse is a unit circle. We compute its range of
        // values on the unit circle.
        auto const r = std::hypot(trans[d], trans[d + 2]);
        auto const mid = trans[d + 4];
        return Interval(mid - r, mid + r);
    };

    return { proj_bounds(X), proj_bounds(Y) };
}

Rect Ellipse::boundsFast() const
{
    // Every ellipse is contained in the circle with the same center and radius
    // equal to the larger of the two rays. We return the bounding square
    // of this circle (this is really fast but only exact for circles).
    auto const larger_ray = (ray(X) > ray(Y) ? ray(X) : ray(Y));
    assert(larger_ray >= 0.0);
    auto const rr = Point(larger_ray, larger_ray);
    return Rect(_center - rr, _center + rr);
}

std::vector<double> Ellipse::coefficients() const
{
    std::vector<double> c(6);
    coefficients(c[0], c[1], c[2], c[3], c[4], c[5]);
    return c;
}

void Ellipse::coefficients(Coord &A, Coord &B, Coord &C, Coord &D, Coord &E, Coord &F) const
{
    if (ray(X) == 0 || ray(Y) == 0) {
        THROW_RANGEERROR("a degenerate ellipse doesn't have an implicit form");
    }

    double cosrot, sinrot;
    sincos(_angle, sinrot, cosrot);
    double cos2 = cosrot * cosrot;
    double sin2 = sinrot * sinrot;
    double cossin = cosrot * sinrot;
    double invrx2 = 1 / (ray(X) * ray(X));
    double invry2 = 1 / (ray(Y) * ray(Y));

    A = invrx2 * cos2 + invry2 * sin2;
    B = 2 * (invrx2 - invry2) * cossin;
    C = invrx2 * sin2 + invry2 * cos2;
    D = -2 * A * center(X) - B * center(Y);
    E = -2 * C * center(Y) - B * center(X);
    F =   A * center(X) * center(X)
        + B * center(X) * center(Y)
        + C * center(Y) * center(Y)
        - 1;
}


void Ellipse::fit(std::vector<Point> const &points)
{
    size_t sz = points.size();
    if (sz < 5) {
        THROW_RANGEERROR("fitting error: too few points passed");
    }
    NL::LFMEllipse model;
    NL::least_squeares_fitter<NL::LFMEllipse> fitter(model, sz);

    for (size_t i = 0; i < sz; ++i) {
        fitter.append(points[i]);
    }
    fitter.update();

    NL::Vector z(sz, 0.0);
    model.instance(*this, fitter.result(z));
}


EllipticalArc *
Ellipse::arc(Point const &ip, Point const &inner, Point const &fp)
{
    // This is resistant to degenerate ellipses:
    // both flags evaluate to false in that case.

    bool large_arc_flag = false;
    bool sweep_flag = false;

    // Determination of large arc flag:
    // large_arc is false when the inner point is on the same side
    // of the center---initial point line as the final point, AND
    // is on the same side of the center---final point line as the
    // initial point.
    // Additionally, large_arc is always false when we have exactly
    // 1/2 of an arc, i.e. the cross product of the center -> initial point
    // and center -> final point vectors is zero.
    // Negating the above leads to the condition for large_arc being true.
    Point fv = fp - _center;
    Point iv = ip - _center;
    Point innerv = inner - _center;
    double ifcp = cross(fv, iv);

    if (ifcp != 0 && (sgn(cross(fv, innerv)) != sgn(ifcp) ||
                      sgn(cross(iv, innerv)) != sgn(-ifcp)))
    {
        large_arc_flag = true;
    }

    // Determination of sweep flag:
    // For clarity, let's assume that Y grows up. Then the cross product
    // is positive for points on the left side of a vector and negative
    // on the right side of a vector.
    //
    //     cross(?, v) > 0
    //  o------------------->
    //     cross(?, v) < 0
    //
    // If the arc is small (large_arc_flag is false) and the final point
    // is on the right side of the vector initial point -> center,
    // we have to go in the direction of increasing angles
    // (counter-clockwise) and the sweep flag is true.
    // If the arc is large, the opposite is true, since we have to reach
    // the final point going the long way - in the other direction.
    // We can express this observation as:
    // cross(_center - ip, fp - _center) < 0 xor large_arc flag
    // This is equal to:
    // cross(-iv, fv) < 0 xor large_arc flag
    // But cross(-iv, fv) is equal to cross(fv, iv) due to antisymmetry
    // of the cross product, so we end up with the condition below.
    if ((ifcp < 0) ^ large_arc_flag) {
        sweep_flag = true;
    }

    EllipticalArc *ret_arc = new EllipticalArc(ip, ray(X), ray(Y), rotationAngle(),
                                               large_arc_flag, sweep_flag, fp);
    return ret_arc;
}

Ellipse &Ellipse::operator*=(Rotate const &r)
{
    _angle += r.angle();
    _center *= r;
    return *this;
}

Ellipse &Ellipse::operator*=(Affine const& m)
{
    Affine a = Scale(ray(X), ray(Y)) * Rotate(_angle);
    Affine mwot = m.withoutTranslation();
    Affine am = a * mwot;
    Point new_center = _center * m;

    if (are_near(am.descrim(), 0)) {
        double angle;
        if (am[0] != 0) {
            angle = std::atan2(am[2], am[0]);
        } else if (am[1] != 0) {
            angle = std::atan2(am[3], am[1]);
        } else {
            angle = M_PI/2;
        }
        Point v = Point::polar(angle) * am;
        _center = new_center;
        _rays[X] = L2(v);
        _rays[Y] = 0;
        _angle = atan2(v);
        return *this;
    } else if (mwot.isScale(0) && _angle.radians() == 0) {
        _rays[X] *= std::abs(mwot[0]);
        _rays[Y] *= std::abs(mwot[3]);
        _center = new_center;
        return *this;
    }

    std::vector<double> coeff = coefficients();
    Affine q( coeff[0],   coeff[1]/2,
              coeff[1]/2, coeff[2],
              0,          0   );

    Affine invm = mwot.inverse();
    q = invm * q ;
    std::swap(invm[1], invm[2]);
    q *= invm;
    setCoefficients(q[0], 2*q[1], q[3], 0, 0, -1);
    _center = new_center;

    return *this;
}

Ellipse Ellipse::canonicalForm() const
{
    Ellipse result(*this);
    result.makeCanonical();
    return result;
}

void Ellipse::makeCanonical()
{
    if (_rays[X] == _rays[Y]) {
        _angle = 0;
        return;
    }

    if (_angle < 0) {
        _angle += M_PI;
    }
    if (_angle >= M_PI/2) {
        std::swap(_rays[X], _rays[Y]);
        _angle -= M_PI/2;
    }
}

Point Ellipse::pointAt(Coord t) const
{
    Point p = Point::polar(t);
    p *= unitCircleTransform();
    return p;
}

Coord Ellipse::valueAt(Coord t, Dim2 d) const
{
    Coord sinrot, cosrot, cost, sint;
    sincos(rotationAngle(), sinrot, cosrot);
    sincos(t, sint, cost);

    if ( d == X ) {
        return    ray(X) * cosrot * cost
                - ray(Y) * sinrot * sint
                + center(X);
    } else {
        return    ray(X) * sinrot * cost
                + ray(Y) * cosrot * sint
                + center(Y);
    }
}

Coord Ellipse::timeAt(Point const &p) const
{
    // degenerate ellipse is basically a reparametrized line segment
    if (ray(X) == 0 || ray(Y) == 0) {
        if (ray(X) != 0) {
            return asin(Line(axis(X)).timeAt(p));
        } else if (ray(Y) != 0) {
            return acos(Line(axis(Y)).timeAt(p));
        } else {
            return 0;
        }
    }
    Affine iuct = inverseUnitCircleTransform();
    return Angle(atan2(p * iuct)).radians0(); // return a value in [0, 2pi)
}

Point Ellipse::unitTangentAt(Coord t) const
{
    Point p = Point::polar(t + M_PI/2);
    p *= unitCircleTransform().withoutTranslation();
    p.normalize();
    return p;
}

bool Ellipse::contains(Point const &p) const
{
    Point tp = p * inverseUnitCircleTransform();
    return tp.length() <= 1;
}

/** @brief Convert curve time on the major axis to the corresponding angle
 *         parameters on a degenerate ellipse collapsed onto that axis.
 * @param t The curve time on the major axis of an ellipse.
 * @param vertical If true, the major axis goes from angle -π/2 to +π/2;
 *                 otherwise, the major axis connects angles π and 0.
 * @return The two angles at which the collapsed ellipse passes through the
 *         major axis point corresponding to the given time \f$t \in [0, 1]\f$.
 */
static std::array<Coord, 2> axis_time_to_angles(Coord t, bool vertical)
{
    Coord const to_unit = std::clamp(2.0 * t - 1.0, -1.0, 1.0);
    if (vertical) {
        double const arcsin = std::asin(to_unit);
        return {arcsin, M_PI - arcsin};
    } else {
        double const arccos = std::acos(to_unit);
        return {arccos, -arccos};
    }
}

/** @brief For each intersection of some shape with the major axis of an ellipse, produce one or two
 *         intersections of a degenerate ellipse (collapsed onto that axis) with the same shape.
 *
 * @param axis_intersections The intersections of some shape with the major axis.
 * @param vertical Whether this is the vertical major axis (in the ellipse's natural coordinates).
 * @return A vector with doubled intersections (corresponding to the two passages of the squashed
 *         ellipse through that point) and swapped order of the intersected shapes.
*/
static std::vector<ShapeIntersection> double_axis_intersections(std::vector<ShapeIntersection> &&axis_intersections,
                                                                bool vertical)
{
    if (axis_intersections.empty()) {
        return {};
    }
    std::vector<ShapeIntersection> result;
    result.reserve(2 * axis_intersections.size());

    for (auto const &x : axis_intersections) {
        for (auto a : axis_time_to_angles(x.second, vertical)) {
            result.emplace_back(a, x.first, x.point()); // Swap first <-> converted second.
            if (x.second == 0.0 || x.second == 1.0) {
                break; // Do not double up endpoint intersections.
            }
        }
    }
    return result;
}

std::vector<ShapeIntersection> Ellipse::intersect(Line const &line) const
{
    std::vector<ShapeIntersection> result;

    if (line.isDegenerate()) {
        return result;
    }
    if (ray(X) == 0 || ray(Y) == 0) {
        return double_axis_intersections(line.intersect(majorAxis()), ray(X) == 0);
    }

    // Ax^2 + Bxy + Cy^2 + Dx + Ey + F
    std::array<Coord, 6> coeffs;
    coefficients(coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4], coeffs[5]);
    rescale_homogenous(coeffs);
    auto [A, B, C, D, E, F] = coeffs;
    Affine iuct = inverseUnitCircleTransform();

    // generic case
    std::array<Coord, 3> line_coeffs;
    line.coefficients(line_coeffs[0], line_coeffs[1], line_coeffs[2]);
    rescale_homogenous(line_coeffs);
    auto [a, b, c] = line_coeffs;
    Point lv = line.vector();

    if (fabs(lv[X]) > fabs(lv[Y])) {
        // y = -a/b x - c/b
        Coord q = -a/b;
        Coord r = -c/b;

        // substitute that into the ellipse equation, making it quadratic in x
        Coord I = A + B*q + C*q*q;          // x^2 terms
        Coord J = B*r + C*2*q*r + D + E*q;  // x^1 terms
        Coord K = C*r*r + E*r + F;          // x^0 terms
        std::vector<Coord> xs = solve_quadratic(I, J, K);

        for (double x : xs) {
            Point p(x, q*x + r);
            result.emplace_back(atan2(p * iuct), line.timeAt(p), p);
        }
    } else {
        Coord q = -b/a;
        Coord r = -c/a;

        Coord I = A*q*q + B*q + C;
        Coord J = A*2*q*r + B*r + D*q + E;
        Coord K = A*r*r + D*r + F;
        std::vector<Coord> xs = solve_quadratic(I, J, K);

        for (double x : xs) {
            Point p(q*x + r, x);
            result.emplace_back(atan2(p * iuct), line.timeAt(p), p);
        }
    }
    return result;
}

std::vector<ShapeIntersection> Ellipse::intersect(LineSegment const &seg) const
{
    if (!boundsFast().intersects(seg.boundsFast())) {
        return {};
    }

    // We simply reuse the procedure for lines and filter out
    // results where the line time value is outside of the unit interval,
    // but we apply a small tolerance to account for numerical errors.
    double const param_prec = EPSILON / seg.length(0.0);
    // TODO: accept a precision setting instead of always using EPSILON
    // (requires an ABI break).

    auto xings = intersect(Line(seg));
    if (xings.empty()) {
        return xings;
    }
    decltype(xings) result;
    result.reserve(xings.size());

    for (auto const &x : xings) {
        if (x.second < -param_prec || x.second > 1.0 + param_prec) {
            continue;
        }
        result.emplace_back(x.first, std::clamp(x.second, 0.0, 1.0), x.point());
    }
    return result;
}

std::vector<ShapeIntersection> Ellipse::intersect(Ellipse const &other) const
{
    // Handle degenerate cases first.
    if (ray(X) == 0 || ray(Y) == 0) { // Degenerate ellipse, collapsed to the major axis.
        return double_axis_intersections(other.intersect(majorAxis()), ray(X) == 0);
    }
    if (*this == other) { // Two identical ellipses.
        THROW_INFINITELY_MANY_SOLUTIONS("The two ellipses are identical.");
    }
    if (!boundsFast().intersects(other.boundsFast())) {
        return {};
    }

    // Find coefficients of the implicit equations of the two ellipses and rescale
    // them (losslessly) for better numerical conditioning.
    std::array<double, 6> coeffs;
    coefficients(coeffs[0], coeffs[1], coeffs[2], coeffs[3], coeffs[4], coeffs[5]);
    rescale_homogenous(coeffs);
    auto [A, B, C, D, E, F] = coeffs;

    std::array<double, 6> otheffs;
    other.coefficients(otheffs[0], otheffs[1], otheffs[2], otheffs[3], otheffs[4], otheffs[5]);
    rescale_homogenous(otheffs);
    auto [a, b, c, d, e, f] = otheffs;

    // Assume that Q(x, y) = 0 is the ellipse equation given by uppercase letters
    // and R(x, y) = 0 is the equation given by lowercase ones.
    // In other words, Q is the quadratic function describing this ellipse and
    // R is the quadratic function for the other ellipse.
    //
    // A point (x, y) is common to both ellipses if and only if it solves the system
    // { Q(x, y) = 0,
    // { R(x, y) = 0.
    //
    // If µ is any real number, we can multiply the first equation by µ and add that
    // to the first equation, obtaining the new system of equations:
    // {            Q(x, y) = 0,
    // { µQ(x, y) + R(x, y) = 0.
    //
    // The first equation still says that (x, y) is a point on this ellipse, but the
    // second equation uses the new expression (µQ + R) instead of the original R.
    //
    // Why do we do this? The reason is that the set of functions {µQ + R : µ real}
    // is a "real system of conics" and there's a theorem which guarantees that such a system
    // always contains a "degenerate conic" [proof below].
    // In practice, the degenerate conic will describe a line or a pair of lines, and intersecting
    // a line with an ellipse is much easier than intersecting two ellipses directly.
    //
    // But in order to be able to do this, we must find a value of µ for which µQ + R is degenerate.
    // We can write the expression (µQ + R)(x, y) in the following way:
    //
    //                          |  aa  bb/2 dd/2 | |x|
    // (µQ + R)(x, y) = [x y 1] | bb/2  cc  ee/2 | |y|
    //                          | dd/2 ee/2  ff  | |1|
    //
    // where aa = µA + a and so on. The determinant can be explicitly written out,
    // giving an equation which is cubic in µ and can be solved analytically.
    // The conic µQ + R is degenerate if and only if this determinant is 0.
    //
    // Proof that there's always a degenerate conic: a cubic real polynomial always has a root,
    // and if the polynomial in µ isn't cubic (coefficient of µ^3 is zero), then the starting
    // conic is already degenerate.

    Coord I, J, K, L; // Coefficients of µ in the expression for the determinant.
    I = (-B*B*F + 4*A*C*F + D*E*B - A*E*E - C*D*D) / 4;
    J = -((B*B - 4*A*C) * f + (2*B*F - D*E) * b + (2*A*E - D*B) * e +
          (2*C*D - E*B) * d + (D*D - 4*A*F) * c + (E*E - 4*C*F) * a) / 4;
    K = -((b*b - 4*a*c) * F + (2*b*f - d*e) * B + (2*a*e - d*b) * E +
          (2*c*d - e*b) * D + (d*d - 4*a*f) * C + (e*e - 4*c*f) * A) / 4;
    L = (-b*b*f + 4*a*c*f + d*e*b - a*e*e - c*d*d) / 4;

    std::vector<Coord> mus = solve_cubic(I, J, K, L);
    Coord mu = infinity();

    // Now that we have solved for µ, we need to check whether the conic
    // determined by µQ + R is reducible to a product of two lines. If it's not,
    // it means that there are no intersections. If it is, the intersections of these
    // lines with the original ellipses (if there are any) give the coordinates
    // of intersections.

    // Prefer middle root if there are three.
    // Out of three possible pairs of lines that go through four points of intersection
    // of two ellipses, this corresponds to cross-lines. These intersect the ellipses
    // at less shallow angles than the other two options.
    if (mus.size() == 3) {
        std::swap(mus[1], mus[0]);
    }
    /// Discriminant within this radius of 0 will be considered zero.
    static Coord const discriminant_precision = 1e-10;

    for (Coord candidate_mu : mus) {
        Coord const aa = std::fma(candidate_mu, A, a);
        Coord const bb = std::fma(candidate_mu, B, b);
        Coord const cc = std::fma(candidate_mu, C, c);
        Coord const delta = sqr(bb) - 4*aa*cc;
        if (delta < -discriminant_precision) {
            continue;
        }
        mu = candidate_mu;
        break;
    }

    // if no suitable mu was found, there are no intersections
    if (mu == infinity()) {
        return {};
    }

    // Create the degenerate conic and decompose it into lines.
    std::array<double, 6> degen = {std::fma(mu, A, a), std::fma(mu, B, b), std::fma(mu, C, c),
                                   std::fma(mu, D, d), std::fma(mu, E, e), std::fma(mu, F, f)};
    rescale_homogenous(degen);
    auto const lines = xAx(degen[0], degen[1], degen[2],
                           degen[3], degen[4], degen[5]).decompose_df(discriminant_precision);

    // intersect with the obtained lines and report intersections
    std::vector<ShapeIntersection> result;
    for (auto const &line : lines) {
        if (line.isDegenerate()) {
            continue;
        }
        auto as = intersect(line);
        // NOTE: If we only cared about the intersection points, we could simply
        // intersect this ellipse with the lines and ignore the other ellipse.
        // But we need the time coordinates on the other ellipse as well.
        auto bs = other.intersect(line);
        if (as.empty() || bs.empty()) {
            continue;
        }
        // Due to numerical errors, a tangency may sometimes be found as 1 intersection
        // on one ellipse and 2 intersections on the other. If this happens, we average
        // the points of the two intersections.
        auto const intersection_average = [](ShapeIntersection const &i,
                                             ShapeIntersection const &j) -> ShapeIntersection
        {
            return ShapeIntersection(i.first, j.first, middle_point(i.point(), j.point()));
        };
        auto const synthesize_intersection = [&](ShapeIntersection const &i,
                                                 ShapeIntersection const &j) -> void
        {
            result.emplace_back(i.first, j.first, middle_point(i.point(), j.point()));
        };
        if (as.size() == 2) {
            if (bs.size() == 2) {
                synthesize_intersection(as[0], bs[0]);
                synthesize_intersection(as[1], bs[1]);
            } else if (bs.size() == 1) {
                synthesize_intersection(intersection_average(as[0], as[1]), bs[0]);
            }
        } else if (as.size() == 1) {
            if (bs.size() == 2) {
                synthesize_intersection(as[0], intersection_average(bs[0], bs[1]));
            } else if (bs.size() == 1) {
                synthesize_intersection(as[0], bs[0]);
            }
        }
    }
    return result;
}

std::vector<ShapeIntersection> Ellipse::intersect(D2<Bezier> const &b) const
{
    Coord A, B, C, D, E, F;
    coefficients(A, B, C, D, E, F);

    // We plug the X and Y curves into the implicit equation and solve for t.
    Bezier x = A*b[X]*b[X] + B*b[X]*b[Y] + C*b[Y]*b[Y] + D*b[X] + E*b[Y] + F;
    std::vector<Coord> r = x.roots();

    std::vector<ShapeIntersection> result;
    for (double & i : r) {
        Point p = b.valueAt(i);
        result.emplace_back(timeAt(p), i, p);
    }
    return result;
}

bool Ellipse::operator==(Ellipse const &other) const
{
    if (_center != other._center) return false;

    Ellipse a = this->canonicalForm();
    Ellipse b = other.canonicalForm();

    if (a._rays != b._rays) return false;
    if (a._angle != b._angle) return false;

    return true;
}


bool are_near(Ellipse const &a, Ellipse const &b, Coord precision)
{
    // We want to know whether no point on ellipse a is further than precision
    // from the corresponding point on ellipse b. To check this, we compute
    // the four extreme points at the end of each ray for each ellipse
    // and check whether they are sufficiently close.

    // First, we need to correct the angles on the ellipses, so that they are
    // no further than M_PI/4 apart. This can always be done by rotating
    // and exchanging axes.
    Ellipse ac = a, bc = b;
    if (distance(ac.rotationAngle(), bc.rotationAngle()).radians0() >= M_PI/2) {
        ac.setRotationAngle(ac.rotationAngle() + M_PI);
    }
    if (distance(ac.rotationAngle(), bc.rotationAngle()) >= M_PI/4) {
        Angle d1 = distance(ac.rotationAngle() + M_PI/2, bc.rotationAngle());
        Angle d2 = distance(ac.rotationAngle() - M_PI/2, bc.rotationAngle());
        Coord adj = d1.radians0() < d2.radians0() ? M_PI/2 : -M_PI/2;
        ac.setRotationAngle(ac.rotationAngle() + adj);
        ac.setRays(ac.ray(Y), ac.ray(X));
    }

    // Do the actual comparison by computing four points on each ellipse.
    Point tps[] = {Point(1,0), Point(0,1), Point(-1,0), Point(0,-1)};
    for (auto & tp : tps) {
        if (!are_near(tp * ac.unitCircleTransform(),
                      tp * bc.unitCircleTransform(),
                      precision))
            return false;
    }
    return true;
}

std::ostream &operator<<(std::ostream &out, Ellipse const &e)
{
    out << "Ellipse(" << e.center() << ", " << e.rays()
        << ", " << format_coord_nice(e.rotationAngle()) << ")";
    return out;
}

}  // end namespace Geom


/*
  Local Variables:
  mode:c++
  c-file-style:"stroustrup"
  c-file-offsets:((innamespace . 0)(inline-open . 0)(case-label . +))
  indent-tabs-mode:nil
  fill-column:99
  End:
*/
// vim: filetype=cpp:expandtab:shiftwidth=4:tabstop=8:softtabstop=4:fileencoding=utf-8:textwidth=99 :



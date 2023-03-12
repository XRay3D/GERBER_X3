/**
 * @file
 * @brief Bernstein-Bezier polynomial
 *//*
 * Authors:
 *   MenTaLguY <mental@rydia.net>
 *   Michael Sloan <mgsloan@gmail.com>
 *   Nathan Hurst <njh@njhurst.com>
 *   Krzysztof Kosiński <tweenk.pl@gmail.com>
 *
 * Copyright 2007-2015 Authors
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
 *
 */

#include <2geom/bezier.h>
#include <2geom/solver.h>
#include <2geom/concepts.h>
#include <2geom/choose.h>

namespace Geom {

std::vector<Coord> Bezier::valueAndDerivatives(Coord t, unsigned n_derivs) const {
    /* This is inelegant, as it uses several extra stores.  I think there might be a way to
     * evaluate roughly in situ. */

     // initialize return vector with zeroes, such that we only need to replace the non-zero derivs
    std::vector<Coord> val_n_der(n_derivs + 1, Coord(0.0));

    // initialize temp storage variables
    std::valarray<Coord> d_(order()+1);
    for(unsigned i = 0; i < size(); i++) {
        d_[i] = c_[i];
    }

    unsigned nn = n_derivs + 1;
    if(n_derivs > order()) {
        nn = order()+1; // only calculate the non zero derivs
    }
    for(unsigned di = 0; di < nn; di++) {
        //val_n_der[di] = (casteljau_subdivision(t, &d_[0], NULL, NULL, order() - di));
        val_n_der[di] = bernstein_value_at(t, &d_[0], order() - di);
        for(unsigned i = 0; i < order() - di; i++) {
            d_[i] = (order()-di)*(d_[i+1] - d_[i]);
        }
    }

    return val_n_der;
}

void Bezier::subdivide(Coord t, Bezier *left, Bezier *right) const
{
    if (left) {
        left->c_.resize(size());
        if (right) {
            right->c_.resize(size());
            casteljau_subdivision<double>(t, &const_cast<std::valarray<Coord>&>(c_)[0],
                &left->c_[0], &right->c_[0], order());
        } else {
            casteljau_subdivision<double>(t, &const_cast<std::valarray<Coord>&>(c_)[0],
                &left->c_[0], NULL, order());
        }
    } else if (right) {
        right->c_.resize(size());
        casteljau_subdivision<double>(t, &const_cast<std::valarray<Coord>&>(c_)[0],
            NULL, &right->c_[0], order());
    }
}

std::pair<Bezier, Bezier> Bezier::subdivide(Coord t) const
{
    std::pair<Bezier, Bezier> ret;
    subdivide(t, &ret.first, &ret.second);
    return ret;
}

std::vector<Coord> Bezier::roots() const
{
    std::vector<Coord> solutions;
    find_bezier_roots(solutions, 0, 1);
    std::sort(solutions.begin(), solutions.end());
    return solutions;
}

std::vector<Coord> Bezier::roots(Interval const &ivl) const
{
    std::vector<Coord> solutions;
    find_bernstein_roots(&const_cast<std::valarray<Coord>&>(c_)[0], order(), solutions, 0, ivl.min(), ivl.max());
    std::sort(solutions.begin(), solutions.end());
    return solutions;
}

Bezier Bezier::forward_difference(unsigned k) const
{
    Bezier fd(Order(order() - k));
    int n = fd.size();
    
    for (int i = 0; i < n; i++) {
        fd[i] = 0;
        int b = (i & 1) ? -1 : 1; // b = (-1)^j binomial(n, j - i)
        for (int j = i; j < n; j++) {
            fd[i] += c_[j] * b;
            binomial_increment_k(b, n, j - i);
            b = -b;
        }
    }
    return fd;
}

Bezier Bezier::elevate_degree() const
{
    Bezier ed(Order(order()+1));
    unsigned n = size();
    ed[0] = c_[0];
    ed[n] = c_[n-1];
    for(unsigned i = 1; i < n; i++) {
        ed[i] = (i*c_[i-1] + (n - i)*c_[i])/(n);
    }
    return ed;
}

Bezier Bezier::reduce_degree() const
{
    if(order() == 0) return *this;
    Bezier ed(Order(order()-1));
    unsigned n = size();
    ed[0] = c_[0];
    ed[n-1] = c_[n]; // ensure exact endpoints
    unsigned middle = n/2;
    for(unsigned i = 1; i < middle; i++) {
        ed[i] = (n*c_[i] - i*ed[i-1])/(n-i);
    }
    for(unsigned i = n-1; i >= middle; i--) {
        ed[i] = (n*c_[i] - i*ed[n-i])/(i);
    }
    return ed;
}

Bezier Bezier::elevate_to_degree(unsigned newDegree) const
{
    Bezier ed = *this;
    for(unsigned i = degree(); i < newDegree; i++) {
        ed = ed.elevate_degree();
    }
    return ed;
}

Bezier Bezier::deflate() const
{
    if(order() == 0) return *this;
    unsigned n = order();
    Bezier b(Order(n-1));
    for(unsigned i = 0; i < n; i++) {
        b[i] = (n*c_[i+1])/(i+1);
    }
    return b;
}

SBasis Bezier::toSBasis() const
{
    SBasis sb;
    bezier_to_sbasis(sb, (*this));
    return sb;
    //return bezier_to_sbasis(&c_[0], order());
}

Bezier &Bezier::operator+=(Bezier const &other)
{
    if (c_.size() > other.size()) {
        c_ += other.elevate_to_degree(degree()).c_;
    } else if (c_.size() < other.size()) {
        *this = elevate_to_degree(other.degree());
        c_ += other.c_;
    } else {
        c_ += other.c_;
    }
    return *this;
}

Bezier &Bezier::operator-=(Bezier const &other)
{
    if (c_.size() > other.size()) {
        c_ -= other.elevate_to_degree(degree()).c_;
    } else if (c_.size() < other.size()) {
        *this = elevate_to_degree(other.degree());
        c_ -= other.c_;
    } else {
        c_ -= other.c_;
    }
    return *this;
}

Bezier operator*(Bezier const &f, Bezier const &g)
{
    int m = f.order();
    int n = g.order();
    Bezier h(Bezier::Order(m+n));
    // h_k = sum_(i+j=k) (m i)f_i (n j)g_j / (m+n k)
    
    int mci = 1;
    for (int i = 0; i <= m; i++) {
        double const fi = mci * f[i];

        int ncj = 1;
        for (int j = 0; j <= n; j++) {
            h[i + j] += fi * ncj * g[j];
            binomial_increment_k(ncj, n, j);
        }

        binomial_increment_k(mci, m, i);
    }

    int mnck = 1;
    for (int k = 0; k <= m + n; k++) {
        h[k] /= mnck;
        binomial_increment_k(mnck, m + n, k);
    }

    return h;
}

Bezier portion(Bezier const &a, double from, double to)
{
    Bezier ret(a);

    bool reverse_result = false;
    if (from > to) {
        std::swap(from, to);
        reverse_result = true;
    }

    do {
        if (from == 0) {
            if (to == 1) {
                break;
            }
            casteljau_subdivision<double>(to, &ret.c_[0], &ret.c_[0], NULL, ret.order());
            break; 
        }
        casteljau_subdivision<double>(from, &ret.c_[0], NULL, &ret.c_[0], ret.order());
        if (to == 1) break;
        casteljau_subdivision<double>((to - from) / (1 - from), &ret.c_[0], &ret.c_[0], NULL, ret.order());
        // to protect against numerical inaccuracy in the above expression, we manually set
        // the last coefficient to a value evaluated directly from the original polynomial
        ret.c_[ret.order()] = a.valueAt(to);
    } while(0);

    if (reverse_result) {
        std::reverse(&ret.c_[0], &ret.c_[0] + ret.c_.size());
    }
    return ret;
}

Bezier derivative(Bezier const &a)
{
    //if(a.order() == 1) return Bezier(0.0);
    if(a.order() == 1) return Bezier(a.c_[1]-a.c_[0]);
    Bezier der(Bezier::Order(a.order()-1));

    for(unsigned i = 0; i < a.order(); i++) {
        der.c_[i] = a.order()*(a.c_[i+1] - a.c_[i]);
    }
    return der;
}

Bezier integral(Bezier const &a)
{
    Bezier inte(Bezier::Order(a.order()+1));

    inte[0] = 0;
    for(unsigned i = 0; i < inte.order(); i++) {
        inte[i+1] = inte[i] + a[i]/(inte.order());
    }
    return inte;
}

OptInterval bounds_fast(Bezier const &b)
{
    OptInterval ret = Interval::from_array(&const_cast<Bezier&>(b).c_[0], b.size());
    return ret;
}

OptInterval bounds_exact(Bezier const &b)
{
    OptInterval ret(b.at0(), b.at1());
    std::vector<Coord> r = derivative(b).roots();
    for (double i : r) {
        ret->expandTo(b.valueAt(i));
    }
    return ret;
}

OptInterval bounds_local(Bezier const &b, OptInterval const &i)
{
    //return bounds_local(b.toSBasis(), i);
    if (i) {
        return bounds_fast(portion(b, i->min(), i->max()));
    } else {
        return OptInterval();
    }
}

/*
 * The general bézier of degree n is
 *
 *     p(t) = sum_{i = 0...n} binomial(n, i) t^i (1 - t)^(n - i) x[i]
 *
 * It can be written explicitly as a polynomial in t as
 *
 *     p(t) = sum_{i = 0...n} binomial(n, i) t^i [ sum_{j = 0...i} binomial(i, j) (-1)^(i - j) x[j] ]
 *
 * Its derivative is
 *
 *     p'(t) = n sum_{i = 1...n} binomial(n - 1, i - 1) t^(i - 1) [ sum_{j = 0...i} binomial(i, j) (-1)^(i - j) x[j] ]
 *
 * This is used by the various specialisations below as an optimisation for low degree n <= 3.
 * In the remaining cases, the generic implementation is used which resorts to iteration.
 */

void bezier_expand_to_image(Interval &range, Coord x0, Coord x1, Coord x2)
{
    range.expandTo(x2);

    if (range.contains(x1)) {
        // The interval contains all control points, and therefore the entire curve.
        return;
    }

    // p'(t) / 2 = at + b
    auto a = (x2 - x1) - (x1 - x0);
    auto b = x1 - x0;

    // t = -b / a
    if (std::abs(a) > EPSILON) {
        auto t = -b / a;
        if (t > 0.0 && t < 1.0) {
            auto s = 1.0 - t;
            auto x = s * s * x0 + 2 * s * t * x1 + t * t * x2;
            range.expandTo(x);
        }
    }
}

void bezier_expand_to_image(Interval &range, Coord x0, Coord x1, Coord x2, Coord x3)
{
    range.expandTo(x3);

    if (range.contains(x1) && range.contains(x2)) {
        // The interval contains all control points, and therefore the entire curve.
        return;
    }

    // p'(t) / 3 = at^2 + 2bt + c
    auto a = (x3 - x0) - 3 * (x2 - x1);
    auto b = (x2 - x1) - (x1 - x0);
    auto c = x1 - x0;

    auto expand = [&] (Coord t) {
        if (t > 0.0 && t < 1.0) {
            auto s = 1.0 - t;
            auto x = s * s * s * x0 + 3 * s * s * t * x1 + 3 * t * t * s * x2 + t * t * t * x3;
            range.expandTo(x);
        }
    };

    // t = (-b ± sqrt(b^2 - ac)) / a
    if (std::abs(a) < EPSILON) {
        if (std::abs(b) > EPSILON) {
            expand(-c / (2 * b));
        }
    } else {
        auto d2 = b * b - a * c;
        if (d2 >= 0.0) {
            auto bsign = b >= 0.0 ? 1 : -1;
            auto tmp = -(b + bsign * std::sqrt(d2));
            expand(tmp / a);
            expand(c / tmp); // Using Vieta's formula: product of roots == c/a
        }
    }
}

} // namespace Geom

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

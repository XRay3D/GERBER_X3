/*
 * Authors:
 *   Thomas Holder
 *   Sergei Izmailov
 *
 * Copyright 2020 Authors
 *
 * SPDX-License-Identifier: LGPL-2.1 or MPL-1.1
 */

#include <2geom/basic-intersection.h>
#include <2geom/parallelogram.h>

#include <cassert>

namespace Geom {

namespace {
/// Return true if `p` is inside a unit rectangle
inline bool unit_rect_contains(Point const &p)
{
    return 0 <= p.x() && p.x() <= 1 && //
           0 <= p.y() && p.y() <= 1;
}

/// N'th corner of a unit rectangle
inline Point unit_rect_corner(unsigned i)
{
    assert(i < 4);
    unsigned const y = i >> 1;
    unsigned const x = (i & 1) ^ y;
    return Point(x, y);
}
} // namespace

Point Parallelogram::corner(unsigned i) const
{
    assert(i < 4);
    return unit_rect_corner(i) * m_affine;
}

Rect Parallelogram::bounds() const
{
    Rect rect(corner(0), corner(2));
    rect.expandTo(corner(1));
    rect.expandTo(corner(3));
    return rect;
}

bool Parallelogram::intersects(Parallelogram const &other) const
{
    if (m_affine.isSingular() || other.m_affine.isSingular()) {
        return false;
    }

    auto const affine1 = other.m_affine * m_affine.inverse();
    auto const affine2 = affine1.inverse();

    // case 1: any corner inside the other rectangle
    for (unsigned i = 0; i != 4; ++i) {
        auto const p = unit_rect_corner(i);
        if (unit_rect_contains(p * affine1) || //
            unit_rect_contains(p * affine2)) {
            return true;
        }
    }

    // case 2: any sides intersect (check diagonals)
    for (unsigned i = 0; i != 2; ++i) {
        auto const A = corner(i);
        auto const B = corner((i + 2) % 4);
        for (unsigned j = 0; j != 2; ++j) {
            auto const C = other.corner(j);
            auto const D = other.corner((j + 2) % 4);
            if (non_collinear_segments_intersect(A, B, C, D)) {
                return true;
            }
        }
    }

    return false;
}

bool Parallelogram::contains(Point const &p) const
{
    return !m_affine.isSingular() && //
           unit_rect_contains(p * m_affine.inverse());
}

bool Parallelogram::contains(Parallelogram const &other) const
{
    if (m_affine.isSingular()) {
        return false;
    }

    auto const inv = m_affine.inverse();

    for (unsigned i = 0; i != 4; ++i) {
        if (!unit_rect_contains(other.corner(i) * inv)) {
            return false;
        }
    }

    return true;
}

Coord Parallelogram::minExtent() const
{
    return std::min(m_affine.expansionX(), //
                    m_affine.expansionY());
}

Coord Parallelogram::maxExtent() const
{
    return std::max(m_affine.expansionX(), //
                    m_affine.expansionY());
}

bool Parallelogram::isSheared(Coord eps) const
{
    return !are_near(dot(m_affine.xAxis(), m_affine.yAxis()), //
                     0.0, eps);
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

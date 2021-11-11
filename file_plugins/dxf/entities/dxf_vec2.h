/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  11 November 2021                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once

#include <QtMath>

struct Vec2 {
    double x = 0.0;
    double y = 0.0;

    /*
    :class:`Vec2` represents a special 2D vector ``(x, y)``. The :class:`Vec2`
    class is optimized for speed and not immutable, :meth:`iadd`, :meth:`isub`,
    :meth:`imul` and :meth:`idiv` modifies the vector itself, the :class:`Vec3`
    class returns a new object.

    :class:`Vec2` initialization accepts double-tuples ``(x, y[, z])``, two
    doubles or any object providing :attr:`x` and :attr:`y` attributes like
    :class:`Vec2` and :class:`Vec3` objects.

    Args:
        v: vector object with :attr:`x` and :attr:`y` attributes/properties or a sequence of double ``[x, y, ...]`` or
           x-axis as double if argument `y` is not ``None``
        y: second double for :code:`Vec2(x, y)`

    :class:`Vec2` implements a subset of :class:`Vec3`.

    */

    //@classmethod
    Vec2 from_angle(double angle, double length = 1.);

    //@classmethod
    Vec2 from_deg_angle(double angle, double length = 1.);

    double abs();

    //@property
    //Returns length of vector.
    double magnitude() noexcept;

    //@property
    bool is_null() const noexcept;

    //@property
    //Angle of vector in radians.
    double angle() const noexcept;

    //@property
    //Angle of vector in degrees.
    double angle_deg() noexcept;

    Vec2 orthogonal(bool ccw = true);

    Vec2 lerp(Vec2 other, double factor = .5);
    Vec2 project(Vec2 other);

    Vec2 normalize(double length = 1.);

    Vec2 reversed();

    operator bool();
    /*
    //__neg__ = reversed
    bool isclose(self, other: 'VecXY', double abs_tol = 1e-12){
        return math.isclose(self.x, other.x, abs_tol=abs_tol) and math.isclose(
            self.y, other.y, abs_tol=abs_tol)
    */
    bool operator==(Vec2 other);

    bool operator<(Vec2 other);

    Vec2 operator+(Vec2 other);

    Vec2& operator+=(Vec2 other);

    Vec2 operator-(Vec2 other);
    /*
    Vec2 __rsub__(self, other: 'VecXY'){
        try:
            return {other.x - self.x, other.y - self.y};
        except AttributeError:
            raise TypeError('invalid argument')
    */
    Vec2& operator-=(Vec2 other);

    Vec2 operator*(double other);
    /*
    Vec2 __rmul__(self, double other){
        return {self.x * other, self.y * other};
    */
    Vec2& operator*=(double other);

    Vec2 operator/(double other);

    Vec2 operator/=(double other);

    double dot(Vec2 other);

    double det(Vec2 other);

    double distance(Vec2 other);

    double angle_between(Vec2 other);

    Vec2 rotate(double angle);
    Vec2 rotate_deg(double angle);

    //@staticmethod
    template <class T>
    Vec2 sum(const T items)
    {
        /* Add all vectors in `items`. */
        Vec2 s;
        for (auto v : items)
            s += v;
        return s;
    }
};

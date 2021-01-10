// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_vec2.h"


Vec2 Vec2::from_angle(double angle, double length) { return { cos(angle) * length, sin(angle) * length }; }

Vec2 Vec2::from_deg_angle(double angle, double length) { return from_angle(qDegreesToRadians(angle), length); }

double Vec2::abs() { return magnitude(); }

double Vec2::magnitude() noexcept { return hypot(x, y); /*sqrt(x * x + y * y);*/ }

bool Vec2::is_null() const noexcept { return qFuzzyIsNull(x) && qFuzzyIsNull(y); }

double Vec2::angle() const noexcept { return atan2(y, x); }

double Vec2::angle_deg() noexcept { return qRadiansToDegrees(angle()); }

Vec2 Vec2::orthogonal(bool ccw)
{
    /*
        Orthogonal vector
        Args:
            ccw: counter clockwise if ``True`` else clockwise
        */
    if (ccw)
        return { -y, x };
    else
        return { y, -x };
}

Vec2 Vec2::lerp(Vec2 other, double factor)
{
    /*
        Linear interpolation between `self` and `other`.
        Args:
            other: target vector/point
            factor: interpolation factor (0=self, 1=other, 0.5=mid point)
        Returns: interpolated vector
        */
    x = x + (other.x - x) * factor;
    y = y + (other.y - y) * factor;
    return { x, y };
}

Vec2 Vec2::project(Vec2 other)
{
    //Project vector `other` onto `self`.
    Vec2 uv = normalize();
    return uv * uv.dot(other);
}

Vec2 Vec2::normalize(double length) { return *this * (length / magnitude()); }

Vec2 Vec2::reversed() { return { -x, -y }; }

Vec2::operator bool() { return !is_null(); }

bool Vec2::operator==(Vec2 other) { return qFuzzyCompare(x, other.x) && qFuzzyCompare(y, other.y); }

bool Vec2::operator<(Vec2 other)
{
    //# accepts also tuples, for more convenience at testing
    if (qFuzzyCompare(x, other.x))
        return y < other.y;
    else
        return x < other.x;
}

Vec2 Vec2::operator+(Vec2 other) { return { x + other.x, y + other.y }; }

Vec2 &Vec2::operator+=(Vec2 other)
{
    x += other.x;
    y += other.y;
    return *this;
}

Vec2 Vec2::operator-(Vec2 other) { return { x - other.x, y - other.y }; }

Vec2 &Vec2::operator-=(Vec2 other)
{
    x -= other.x;
    y -= other.y;
    return *this;
}

Vec2 Vec2::operator*(double other) { return { x * other, y * other }; }

Vec2 &Vec2::operator*=(double other)
{
    x *= other;
    y *= other;
    return *this;
}

Vec2 Vec2::operator/(double other) { return { x / other, y / other }; }

Vec2 Vec2::operator/=(double other)
{
    x /= other;
    y /= other;
    return *this;
}

double Vec2::dot(Vec2 other) { return x * other.x + y * other.y; }

double Vec2::det(Vec2 other) { return x * other.y - y * other.x; }

double Vec2::distance(Vec2 other) { return hypot(x - other.x, y - other.y); }

double Vec2::angle_between(Vec2 other)
{
    /*
        Calculate angle between `self` and `other` in radians. +angle is
        counter clockwise orientation.
        */
    double cos_theta = normalize().dot(other.normalize());
    //# avoid domain errors caused by doubleing point imprecision:
    if (cos_theta < -1.0)
        cos_theta = -1.0;
    else if (cos_theta > 1.0)
        cos_theta = 1.0;
    return acos(cos_theta);
}

Vec2 Vec2::rotate(double angle)
{
    /*
        Rotate vector around origin.
        Args:
            angle: angle in radians
        */
    return from_angle(this->angle() + angle, magnitude());
}

Vec2 Vec2::rotate_deg(double angle)
{
    /*
        Rotate vector around origin.
        Args:
            angle: angle in degrees
        Returns: rotated vector
        */
    return from_angle(this->angle() + qDegreesToRadians(angle), magnitude());
}

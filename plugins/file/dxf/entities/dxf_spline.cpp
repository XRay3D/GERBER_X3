// This is an open source non-commercial project. Dear PVS-Studio, please check it.
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
 *******************************************************************************/
#include "dxf_spline.h"

#include <QMetaEnum>

#include <QGraphicsEllipseItem>

#include <QPainter>
#include <QPainterPath>
#include <QPolygonF>
#include <QtMath>

namespace Dxf {
QPolygonF interpolate(const QPolygonF& points, int numValues) {
    QwtSpline spline;
    spline.setSplineType(QwtSpline::Periodic);
    if(!spline.setPoints(points))
        return points;

    QPolygonF interpolatedPoints(numValues);
    const double delta = (points[/*numPoints*/ points.size() - 1].x() - points[0].x()) / (points.size() - 1);
    for(int i = 0; i < points.size(); i++) // interpolate
    {
        const double x = points[0].x() + i * delta;
        interpolatedPoints[i].setX(x);
        interpolatedPoints[i].setY(spline.value(x));
    }
    return interpolatedPoints;
}

void BSplineCurve(const QPointF& point1,
    const QPointF& point2,
    const QPointF& point3,
    const QPointF& point4,
    const double t,
    QPointF& result) {
    const double t2 = t * t;
    const double t3 = t2 * t;
    const double mt = 1.0 - t;
    const double mt3 = mt * mt * mt;

    const double bi3 = mt3;
    const double bi2 = 3 * t3 - 6 * t2 + 4;
    const double bi1 = -3 * t3 + 3 * t2 + 3 * t + 1;
    const double bi = t3;

    result.rx() = point1.x() * bi3 + point2.x() * bi2 + point3.x() * bi1 + point4.x() * bi;
    result.rx() /= 6.0;

    result.ry() = point1.y() * bi3 + point2.y() * bi2 + point3.y() * bi1 + point4.y() * bi;
    result.ry() /= 6.0;
}

/*  So after investing way more time than I should have,
 *  I finally found the answer in a line in my textbook that I missed.
 *  Apparently, the curve is defined only for values of u between uVec[d-1] and uVec[n].
 *
 * Поэтому, потратив гораздо больше времени, чем следовало бы,
 * я наконец нашел ответ в строке учебника, которую пропустил.
 * По-видимому, кривая определяется только для значений u между uVec[d-1] и uVec[n].
 */

// double blend(QVector<double>& uVec, double u, int k, int d)
//{
//     if (d == 1) {
//         if (uVec[k] <= u && u < uVec[k + 1])
//             return 1;
//         return 0;
//     }
//     double b;
//     b = ((u - uVec[k]) / (uVec[k + d - 1] - uVec[k]) * blend(uVec, u, k, d - 1)) + ((uVec[k + d] - u) / (uVec[k + d] - uVec[k + 1]) * blend(uVec, u, k + 1, d - 1));
//     return b;
// }
double blend(QVector<double>& uVec, double u, int k, int d) {
    if(d == 1) {
        if(uVec[k] <= u && u < uVec[k + 1])
            return 1;
        return 0;
    }
    double b;
    b = ((u - uVec[k]) / (uVec[k + d - 1] - uVec[k]) * blend(uVec, u, k, d - 1)) //
        + ((uVec[k + d] - u) / (uVec[k + d] - uVec[k + 1]) * blend(uVec, u, k + 1, d - 1));
    return b;
}

void drawBSplineCurve(const Spline& poly, QPainterPath& path) {

    int n, d;
    d = poly.degreeOfTheSplineCurve; // Enter degree of curve:
    n = poly.ControlPoints.size();
    QVector<double> uVec;
    //  poly.KnotValues;
    for(int i = 0; i < n + d; i++)
        uVec.push_back(((double)i) / (n + d - 1));
    double x, y, basis, u;
    for(u = 0; u <= 1; u += 0.05) {
        x = 0;
        y = 0;
        for(int i = 0.0; i < poly.ControlPoints.size(); i++) {
            // basis = blend(uVec, u, i, d);
            basis = blend(uVec, u, i, d);
            x += basis * poly.ControlPoints[i].x();
            y += basis * poly.ControlPoints[i].y();
        }

        if(u == 0.0 && x != 0.0 && x != 0.0)
            path.moveTo(x, y);
        else if(x != 0.0 && x != 0.0)
            path.lineTo(x, y);
        //        putpixel(roundOff(x), roundOff(y), YELLOW);
    }
}

Spline::Spline(SectionParser* sp)
    : Entity(sp) {
}

void Spline::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(static_cast<DataEnum>(code.code())) {
        case SubclassMarker:      // 100
            break;
        case ExtrusionDirectionX: // 210
            break;
        case ExtrusionDirectionY: // 220
            break;
        case ExtrusionDirectionZ: // 230
            break;
        case SplineFlag:          // 70
            splineFlag = code;
            break;
        case DegreeOfTheSplineCurve: // 71
            degreeOfTheSplineCurve = code;
            break;
        case NumberOfKnots: // 72
            numberOfKnots = code;
            KnotValues.reserve(numberOfKnots);
            break;
        case NumberOfControlPoints: // 73
            numberOfControlPoints = code;
            ControlPoints.reserve(numberOfControlPoints);
            break;
        case NumberOfFitPoints: // 74
            numberOfFitPoints = code;
            FitPoints.reserve(numberOfFitPoints);
            break;
        case KnotTolerance: // 42
            knotTolerance = code;
            break;
        case ControlPointTolerance: // 43
            controlPointTolerance = code;
            break;
        case FitTolerance: // 44
            fitTolerance = code;
            break;
        case StartTangentX: // 12
            StartTangent.setX(code);
            break;
        case StartTangentY: // 22
            StartTangent.setY(code);
            break;
        case StartTangentZ: // 32
            break;
        case EndTangentX:   // 13
            EndTangent.setX(code);
            break;
        case EndTangentY: // 23
            EndTangent.setY(code);
            break;
        case EndTangentZ: // 33
            break;
        case KnotValue:   // 40
            KnotValues << double(code);
            break;
        case Weight: // 41
            weight = code;
            break;
        case ControlPointsX: // 10
            ControlPoints.resize(ControlPoints.size() + 1);
            ControlPoints.last().setX(code);
            break;
        case ControlPointsY: // 20
            ControlPoints.last().setY(code);
            break;
        case ControlPointsZ: // 30
            break;
        case FitPointsX:     // 11
            FitPoints.resize(FitPoints.size() + 1);
            FitPoints.last().setX(code);
            break;
        case FitPointsY: // 21
            FitPoints.last().setY(code);
            break;
        case FitPointsZ: // 31
            break;
        default:
            Entity::parse(code);
        }
        code = sp->nextCode();
    } while(code.code() != 0);
}

Entity::Type Spline::type() const { return Type::SPLINE; }

DxfGo Spline::toGo() const { return {}; }

void Spline::write(QDataStream& stream) const {
    stream << FitPoints;
    stream << ControlPoints;
    stream << StartTangent;
    stream << EndTangent;

    stream << KnotValues;
    stream << weight;

    stream << knotTolerance;
    stream << controlPointTolerance;
    stream << fitTolerance;

    stream << degreeOfTheSplineCurve;
    stream << numberOfControlPoints;
    stream << numberOfFitPoints;
    stream << numberOfKnots;
    stream << splineFlag;
}

void Spline::read(QDataStream& stream) {
    stream >> FitPoints;
    stream >> ControlPoints;
    stream >> StartTangent;
    stream >> EndTangent;

    stream >> KnotValues;
    stream >> weight;

    stream >> knotTolerance;
    stream >> controlPointTolerance;
    stream >> fitTolerance;

    stream >> degreeOfTheSplineCurve;
    stream >> numberOfControlPoints;
    stream >> numberOfFitPoints;
    stream >> numberOfKnots;
    stream >> splineFlag;
}

////////////////////////////////////

#ifndef M_PI_2
// For Qt <= 4.8.4 M_PI_2 is not known by MinGW-w64
// when compiling with -std=c++11
#define M_PI_2 (1.57079632679489661923)
#endif

#ifndef LOG_MIN
//! Minimum value for logarithmic scales
#define LOG_MIN 1.0e-100
#endif

#ifndef LOG_MAX
//! Maximum value for logarithmic scales
#define LOG_MAX 1.0e100
#endif

// QWT_EXPORT double qwtGetMin(const double* array, int size);
// QWT_EXPORT double qwtGetMax(const double* array, int size);

// QWT_EXPORT double qwtNormalizeRadians(double radians);
// QWT_EXPORT double qwtNormalizeDegrees(double degrees);

/*!
  \brief Compare 2 values, relative to an interval
  Values are "equal", when :
  \f$\cdot value2 - value1 <= abs(intervalSize * 10e^{-6})\f$
  \param value1 First value to compare
  \param value2 Second value to compare
  \param intervalSize interval size
  \return 0: if equal, -1: if value2 > value1, 1: if value1 > value2
*/
inline int qwtFuzzyCompare(double value1, double value2, double intervalSize) {
    const double eps = qAbs(1.0e-6 * intervalSize);

    if(value2 - value1 > eps)
        return -1;

    if(value1 - value2 > eps)
        return 1;

    return 0;
}

inline bool qwtFuzzyGreaterOrEqual(double d1, double d2) {
    return (d1 >= d2) || qFuzzyCompare(d1, d2);
}

inline bool qwtFuzzyLessOrEqual(double d1, double d2) {
    return (d1 <= d2) || qFuzzyCompare(d1, d2);
}

//! Return the sign
inline int qwtSign(double x) {
    if(x > 0.0)
        return 1;
    else if(x < 0.0)
        return (-1);
    else
        return 0;
}

//! Return the square of a number
inline double qwtSqr(double x) {
    return x * x;
}

//! Approximation of arc tangent ( error below 0,005 radians )
inline double qwtFastAtan(double x) {
    if(x < -1.0)
        return -M_PI_2 - x / (x * x + 0.28);

    if(x > 1.0)
        return M_PI_2 - x / (x * x + 0.28);

    return x / (1.0 + x * x * 0.28);
}

//! Approximation of arc tangent ( error below 0,005 radians )
inline double qwtFastAtan2(double y, double x) {
    if(x > 0)
        return qwtFastAtan(y / x);

    if(x < 0) {
        const double d = qwtFastAtan(y / x);
        return (y >= 0) ? d + pi : d - pi;
    }

    if(y < 0.0)
        return -M_PI_2;

    if(y > 0.0)
        return M_PI_2;

    return 0.0;
}

//! Translate degrees into radians
inline double qwtRadians(double degrees) {
    return degrees * pi / 180.0;
}

//! Translate radians into degrees
inline double qwtDegrees(double degrees) {
    return degrees * 180.0 / pi;
}

inline double qwtGetMin(const double* array, int size) {
    if(size <= 0)
        return 0.0;

    double rv = array[0];
    for(int i = 1; i < size; i++)
        rv = qMin(rv, array[i]);

    return rv;
}

/*!
  \brief Find the largest value in an array
  \param array Pointer to an array
  \param size Array size
*/
inline double qwtGetMax(const double* array, int size) {
    if(size <= 0)
        return 0.0;

    double rv = array[0];
    for(int i = 1; i < size; i++)
        rv = std::max(rv, array[i]);

    return rv;
}

/*!
  \brief Normalize an angle to be int the range [0.0, 2 * PI[
  \param radians Angle in radians
  \return Normalized angle in radians
*/
inline double qwtNormalizeRadians(double radians) {
    double a = ::fmod(radians, 2.0 * pi);
    if(a < 0.0)
        a += 2.0 * pi;

    return a;
}

/*!
  \brief Normalize an angle to be int the range [0.0, 360.0[
  \param radians Angle in degrees
  \return Normalized angle in degrees
*/
inline double qwtNormalizeDegrees(double degrees) {
    double a = ::fmod(degrees, 360.0);
    if(a < 0.0)
        a += 360.0;

    return a;
}

/////////////////////////////////////////////
class QwtSpline::PrivateData {
public:
    PrivateData()
        : splineType(QwtSpline::Natural) {
    }

    QwtSpline::SplineType splineType;

    // coefficient vectors
    QVector<double> a;
    QVector<double> b;
    QVector<double> c;

    // control points
    QPolygonF points;
};

static int lookup(double x, const QPolygonF& values) {
#if 0
    //qLowerBound/qHigherBound ???
#endif
    int i1;
    const int size = values.size();

    if(x <= values[0].x())
        i1 = 0;
    else if(x >= values[size - 2].x())
        i1 = size - 2;
    else {
        i1 = 0;
        int i2 = size - 2;
        int i3 = 0;

        while(i2 - i1 > 1) {
            i3 = i1 + ((i2 - i1) >> 1);

            if(values[i3].x() > x)
                i2 = i3;
            else
                i1 = i3;
        }
    }
    return i1;
}

//! Constructor
QwtSpline::QwtSpline() {
    d_data = new PrivateData;
}

/*!
   Copy constructor
   \param other Spline used for initialization
*/
QwtSpline::QwtSpline(const QwtSpline& other) {
    d_data = new PrivateData{*other.d_data};
}

/*!
   Assignment operator
   \param other Spline used for initialization
   \return *this
*/
QwtSpline& QwtSpline::operator=(const QwtSpline& other) {
    *d_data = *other.d_data;
    return *this;
}

//! Destructor
QwtSpline::~QwtSpline() {
    delete d_data;
}

/*!
   Select the algorithm used for calculating the spline
   \param splineType Spline type
   \sa splineType()
*/
void QwtSpline::setSplineType(SplineType splineType) {
    d_data->splineType = splineType;
}

/*!
   \return the spline type
   \sa setSplineType()
*/
QwtSpline::SplineType QwtSpline::splineType() const {
    return d_data->splineType;
}

/*!
  \brief Calculate the spline coefficients
  Depending on the value of \a periodic, this function
  will determine the coefficients for a natural or a periodic
  spline and store them internally.
  \param points Points
  \return true if successful
  \warning The sequence of x (but not y) values has to be strictly monotone
           increasing, which means <code>points[i].x() < points[i+1].x()</code>.
       If this is not the case, the function will return false
*/
bool QwtSpline::setPoints(const QPolygonF& points) {
    const int size = points.size();
    if(size <= 2) {
        reset();
        return false;
    }

    d_data->points = points;

    d_data->a.resize(size - 1);
    d_data->b.resize(size - 1);
    d_data->c.resize(size - 1);

    bool ok;
    if(d_data->splineType == Periodic)
        ok = buildPeriodicSpline(points);
    else
        ok = buildNaturalSpline(points);

    if(!ok)
        reset();

    return ok;
}

/*!
   \return Points, that have been by setPoints()
*/
QPolygonF QwtSpline::points() const {
    return d_data->points;
}

//! \return A coefficients
const QVector<double>& QwtSpline::coefficientsA() const {
    return d_data->a;
}

//! \return B coefficients
const QVector<double>& QwtSpline::coefficientsB() const {
    return d_data->b;
}

//! \return C coefficients
const QVector<double>& QwtSpline::coefficientsC() const {
    return d_data->c;
}

//! Free allocated memory and set size to 0
void QwtSpline::reset() {
    d_data->a.resize(0);
    d_data->b.resize(0);
    d_data->c.resize(0);
    d_data->points.resize(0);
}

//! True if valid
bool QwtSpline::isValid() const {
    return d_data->a.size() > 0;
}

/*!
  Calculate the interpolated function value corresponding
  to a given argument x.
  \param x Coordinate
  \return Interpolated coordinate
*/
double QwtSpline::value(double x) const {
    if(d_data->a.size() == 0)
        return 0.0;

    const int i = lookup(x, d_data->points);

    const double delta = x - d_data->points[i].x();
    return ((((d_data->a[i] * delta) + d_data->b[i])
                    * delta
                + d_data->c[i])
            * delta
        + d_data->points[i].y());
}

/*!
  \brief Determines the coefficients for a natural spline
  \return true if successful
*/
bool QwtSpline::buildNaturalSpline(const QPolygonF& points) {
    int i;

    const QPointF* p = points.data();
    const int size = points.size();

    double* a = d_data->a.data();
    double* b = d_data->b.data();
    double* c = d_data->c.data();

    //  set up tridiagonal equation system; use coefficient
    //  vectors as temporary buffers
    QVector<double> h(size - 1);
    for(i = 0; i < size - 1; i++) {
        h[i] = p[i + 1].x() - p[i].x();
        if(h[i] <= 0)
            return false;
    }

    QVector<double> d(size - 1);
    double dy1 = (p[1].y() - p[0].y()) / h[0];
    for(i = 1; i < size - 1; i++) {
        b[i] = c[i] = h[i];
        a[i] = 2.0 * (h[i - 1] + h[i]);

        const double dy2 = (p[i + 1].y() - p[i].y()) / h[i];
        d[i] = 6.0 * (dy1 - dy2);
        dy1 = dy2;
    }

    //
    // solve it
    //

    // L-U Factorization
    for(i = 1; i < size - 2; i++) {
        c[i] /= a[i];
        a[i + 1] -= b[i] * c[i];
    }

    // forward elimination
    QVector<double> s(size);
    s[1] = d[1];
    for(i = 2; i < size - 1; i++)
        s[i] = d[i] - c[i - 1] * s[i - 1];

    // backward elimination
    s[size - 2] = -s[size - 2] / a[size - 2];
    for(i = size - 3; i > 0; i--)
        s[i] = -(s[i] + b[i] * s[i + 1]) / a[i];
    s[size - 1] = s[0] = 0.0;

    //
    // Finally, determine the spline coefficients
    //
    for(i = 0; i < size - 1; i++) {
        a[i] = (s[i + 1] - s[i]) / (6.0 * h[i]);
        b[i] = 0.5 * s[i];
        c[i] = (p[i + 1].y() - p[i].y()) / h[i]
            - (s[i + 1] + 2.0 * s[i]) * h[i] / 6.0;
    }

    return true;
}

/*!
  \brief Determines the coefficients for a periodic spline
  \return true if successful
*/
bool QwtSpline::buildPeriodicSpline(const QPolygonF& points) {
    int i;

    const QPointF* p = points.data();
    const int size = points.size();

    double* a = d_data->a.data();
    double* b = d_data->b.data();
    double* c = d_data->c.data();

    QVector<double> d(size - 1);
    QVector<double> h(size - 1);
    QVector<double> s(size);

    //
    //  setup equation system; use coefficient
    //  vectors as temporary buffers
    //
    for(i = 0; i < size - 1; i++) {
        h[i] = p[i + 1].x() - p[i].x();
        if(h[i] <= 0.0)
            return false;
    }

    const int imax = size - 2;
    double htmp = h[imax];
    double dy1 = (p[0].y() - p[imax].y()) / htmp;
    for(i = 0; i <= imax; i++) {
        b[i] = c[i] = h[i];
        a[i] = 2.0 * (htmp + h[i]);
        const double dy2 = (p[i + 1].y() - p[i].y()) / h[i];
        d[i] = 6.0 * (dy1 - dy2);
        dy1 = dy2;
        htmp = h[i];
    }

    //
    // solve it
    //

    // L-U Factorization
    a[0] = std::sqrt(a[0]);
    c[0] = h[imax] / a[0];
    double sum = 0;

    for(i = 0; i < imax - 1; i++) {
        b[i] /= a[i];
        if(i > 0)
            c[i] = -c[i - 1] * b[i - 1] / a[i];
        a[i + 1] = std::sqrt(a[i + 1] - qwtSqr(b[i]));
        sum += qwtSqr(c[i]);
    }
    b[imax - 1] = (b[imax - 1] - c[imax - 2] * b[imax - 2]) / a[imax - 1];
    a[imax] = std::sqrt(a[imax] - qwtSqr(b[imax - 1]) - sum);

    // forward elimination
    s[0] = d[0] / a[0];
    sum = 0;
    for(i = 1; i < imax; i++) {
        s[i] = (d[i] - b[i - 1] * s[i - 1]) / a[i];
        sum += c[i - 1] * s[i - 1];
    }
    s[imax] = (d[imax] - b[imax - 1] * s[imax - 1] - sum) / a[imax];

    // backward elimination
    s[imax] = -s[imax] / a[imax];
    s[imax - 1] = -(s[imax - 1] + b[imax - 1] * s[imax]) / a[imax - 1];
    for(i = imax - 2; i >= 0; i--)
        s[i] = -(s[i] + b[i] * s[i + 1] + c[i] * s[imax]) / a[i];

    //
    // Finally, determine the spline coefficients
    //
    s[size - 1] = s[0];
    for(i = 0; i < size - 1; i++) {
        a[i] = (s[i + 1] - s[i]) / (6.0 * h[i]);
        b[i] = 0.5 * s[i];
        c[i] = (p[i + 1].y() - p[i].y())
                / h[i]
            - (s[i + 1] + 2.0 * s[i]) * h[i] / 6.0;
    }

    return true;
}

} // namespace Dxf

#include "moc_dxf_spline.cpp"

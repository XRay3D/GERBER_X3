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
#pragma once

#include "dxf_entity.h"
#include <cmath>
namespace Dxf {
struct Spline final : Entity {
    Spline(SectionParser* sp);

    // Entity interface
public:
    void parse(CodeData& code) override;
    Type type() const override { return Type::SPLINE; }
    GraphicObject toGo() const override;

    enum DataEnum {
        SubclassMarker = 100, //	100	Маркер подкласса (AcDbSpline)

        ExtrusionDirectionX = 210, //	210	Вектор нормали (отсутствует, если сплайн неплоский)
        ExtrusionDirectionY = 220, //        Dxf: x value; App: 3dVector		=		,//		Файл DXF: значение X; приложение: 3D-вектор
        ExtrusionDirectionZ = 230, //        Dxf: y and z valuesOfNormalVector (optional)		=	220, 230	,//	220, 230	Файл DXF: значение Y и Z вектора нормали (необязательно)

        SplineFlag = 70, //	70	Флаг сплайна (кодовый бит):
        //        1 = ClosedSpline		=		,//		1 = замкнутый сплайн
        //        2 = PeriodicSpline		=		,//		2 = периодический сплайн
        //        4 = RationalSpline		=		,//		4 = рациональный сплайн
        //        8 = Planar		=		,//		8 = плоский
        //        16 = Linear (planarBitIsAlsoSet)		=		,//		16 = линейный (также задается бит 8 — "плоский")

        DegreeOfTheSplineCurve = 71, //	71	Степень кривой сплайна
        NumberOfKnots = 72, //	72	Число узлов
        NumberOfControlPoints = 73, //	73	Число управляющих точек
        NumberOfFitPoints = 74, //	74	Число определяющих точек (если есть)

        KnotTolerance = 42, //	42	Допуск узлов (по умолчанию = 0,0000001)
        ControlPointTolerance = 43, //	43	Допуск управляющих точек (по умолчанию = 0,0000001)
        FitTolerance = 44, //	44	Допуск определяющих точек (по умолчанию = 0.0000000001)

        StartTangentX = 12, //	12	Касательная в начальной точке, может быть опущено (в МСК)
        StartTangentY = 22, //        Dxf: x value; App: 3dPoint		=		,//		Файл DXF: значение X; приложение: 3D-точка
        StartTangentZ = 32, //        Dxf: y and z valuesOfStartTangent—mayBeOmitted (inWcs)		=	22, 32	,//	22, 32	Файл DXF: значения Y и Z касательной в начальной точке, может быть опущена (в МСК)

        EndTangentX = 13, //	13	Касательная в конечной точке, может быть опущено (в МСК)
        EndTangentY = 23, //        Dxf: x value; App: 3dPoint		=		,//		Файл DXF: значение X; приложение: 3D-точка
        EndTangentZ = 33, //        Dxf: y and z valuesOfEndTangent—mayBeOmitted (inWcs)		=	23, 33	,//	23, 33	Файл DXF: значения Y и Z касательной в конечной точке, может быть опущена (в МСК)

        KnotValue = 40, //	40	Значение узла (одна запись на узел)
        Weight = 41, //	41	Вес (если значение не равно 1); с несколькими парами групп, которые присутствуют, если всем не присвоено значение 1

        ControlPointsX = 10, //	10	Управляющие точки (в МСК); одна запись на управляющую точку
        ControlPointsY = 20, //        Dxf: x value; App: 3dPoint		=		,//		Файл DXF: значение X; приложение: 3D-точка
        ControlPointsZ = 30, //        Dxf: y and z valuesOfControlPoints (inWcs); OneEntryPerControlPoint		=	20, 30	,//	20, 30	Файл DXF: значения Y и Z управляющих точек (в МСК); одна запись в управляющей точке

        FitPointsX = 11, //	11	Определяющие точки (в МСК); одна запись на определяющую точку
        FitPointsY = 21, //        Dxf: x value; App: 3dPoint		=		,//		Файл DXF: значение X; приложение: 3D-точка
        FitPointsZ = 31, //        Dxf: y and z valuesOfFitPoints (inWcs); OneEntryPerFitPoint		=	21, 31	,//	21, 31	Файл DXF: значения Y и Z определяющих точек (в МСК); одна запись на определяющую точку
    };

    QPolygonF FitPoints;
    QPolygonF ControlPoints;
    QPointF StartTangent;
    QPointF EndTangent;

    QVector<double> KnotValues;
    double weight = 1.0;

    double knotTolerance = 0.0000001;
    double controlPointTolerance = 0.0000001;
    double fitTolerance = 0.0000000001;

    int degreeOfTheSplineCurve = 0.0;
    int numberOfControlPoints = 0;
    int numberOfFitPoints = 0;
    int numberOfKnots = 0;
    int splineFlag = 0;

    Q_ENUM(DataEnum)
    Q_GADGET
};

#include <qpolygon.h>
#include <qvector.h>

/*!
  \brief A class for spline interpolation
  The QwtSpline class is used for cubical spline interpolation.
  Two types of splines, natural and periodic, are supported.
  \par Usage:
  <ol>
  <li>First call setPoints() to determine the spline coefficients
      for a tabulated function y(x).
  <li>After the coefficients have been set up, the interpolated
      function value for an argument x can be determined by calling
      QwtSpline::value().
  </ol>
  \par Example:
  \code
#include <qwt_spline.h>
QPolygonF interpolate(const QPolygonF& points, int numValues)
{
    QwtSpline spline;
    if ( !spline.setPoints(points) )
        return points;
    QPolygonF interpolatedPoints(numValues);
    const double delta =
        (points[numPoints - 1].x() - points[0].x()) / (points.size() - 1);
    for(i = 0; i < points.size(); i++)  / interpolate
    {
        const double x = points[0].x() + i * delta;
        interpolatedPoints[i].setX(x);
        interpolatedPoints[i].setY(spline.value(x));
    }
    return interpolatedPoints;
}
  \endcode
*/

class QwtSpline {
public:
    //! Spline type
    enum SplineType {
        //! A natural spline
        Natural,

        //! A periodic spline
        Periodic
    };

    QwtSpline();
    QwtSpline(const QwtSpline&);

    ~QwtSpline();

    QwtSpline& operator=(const QwtSpline&);

    void setSplineType(SplineType);
    SplineType splineType() const;

    bool setPoints(const QPolygonF& points);
    QPolygonF points() const;

    void reset();

    bool isValid() const;
    double value(double x) const;

    const QVector<double>& coefficientsA() const;
    const QVector<double>& coefficientsB() const;
    const QVector<double>& coefficientsC() const;

protected:
    bool buildNaturalSpline(const QPolygonF&);
    bool buildPeriodicSpline(const QPolygonF&);

private:
    class PrivateData;
    PrivateData* d_data;
};
}

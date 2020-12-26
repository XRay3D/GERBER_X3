/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2020                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#pragma once
#ifndef GERBERAPERTURE_H
#define GERBERAPERTURE_H

#include "gbrvars.h"

#include <QMap>
#include <QtMath>

namespace Gerber {

enum ApertureType {
    Circle,
    Rectangle,
    Obround,
    Polygon,
    Macro,
    Block,
};

struct Format;

class AbstractAperture {
    Q_DISABLE_COPY(AbstractAperture)

public:
    AbstractAperture(const Format* m_format);
    virtual ~AbstractAperture();

    bool isDrilled() const { return m_drillDiam != 0.0; }
    bool isFlashed() const { return m_isFlashed; }

    double drillDiameter() const;
    double apertureSize();

    Path drawDrill(const State& state);
    Paths draw(const State& state, bool fl = false);

    virtual QString name() const = 0;
    virtual ApertureType type() const = 0;

    double minSize() const;

    virtual bool fit(double toolDiam) const = 0;

protected:
    bool m_isFlashed = false;
    double m_drillDiam = 0.0;
    double m_size = 0.0;

    Paths m_paths;
    virtual void draw() = 0;
    const Format* m_format;

    void transform(Path& poligon, const State& state);
};

/////////////////////////////////////////////////////
/// \brief The GACircular class
///
class ApCircle : public AbstractAperture {
public:
    ApCircle(double diam, double drillDiam, const Format* format);

    QString name() const override;
    ApertureType type() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double m_diam = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class ApRectangle : public AbstractAperture {
    friend class Parser;

public:
    ApRectangle(double width, double height, double drillDiam, const Format* format);

    QString name() const override;
    ApertureType type() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class ApObround : public AbstractAperture {
public:
    ApObround(double width, double height, double drillDiam, const Format* format);
    QString name() const override;
    ApertureType type() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class ApPolygon : public AbstractAperture {
public:
    ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const Format* format);

    double rotation() const;
    int verticesCount() const;

    QString name() const override;
    ApertureType type() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double m_diam = 0.0;
    double m_rotation = 0.0;
    int m_verticesCount = 0;
};

/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
class ApMacro : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& coefficients, const Format* format);

    QString name() const override;
    ApertureType type() const override;
    bool fit(double) const override;

protected:
    void draw() override;

private:
    QList<QString> m_modifiers;
    QMap<QString, double> m_coefficients;
    QString m_macro;

    Path drawCenterLine(const QList<double>& mod);
    Path drawCircle(const QList<double>& mod);
    Path drawOutlineCustomPolygon(const QList<double>& mod);
    Path drawOutlineRegularPolygon(const QList<double>& mod);
    Path drawVectorLine(const QList<double>& mod);
    void drawMoire(const QList<double>& mod);
    void drawThermal(const QList<double>& mod);
};
/////////////////////////////////////////////////////
/// \brief The ApBlock class
///
class ApBlock : public AbstractAperture, public QList<GraphicObject> {
public:
    ApBlock(const Format* format);

    QString name() const override;
    ApertureType type() const override;
    bool fit(double) const override;

protected:
    void draw() override;
};
}
#endif // GERBERAPERTURE_H

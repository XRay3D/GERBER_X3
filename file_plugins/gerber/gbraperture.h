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
#include "gbrtypes.h"

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

class ApBlock;
class ApCircle;
class ApMacro;
class ApObround;
class ApPolygon;
class ApRectangle;

class AbstractAperture {
    Q_DISABLE_COPY(AbstractAperture)
    friend QDataStream& operator<<(QDataStream& stream, const QSharedPointer<AbstractAperture>& aperture)
    {
        stream << aperture->type();
        aperture->write(stream);
        return stream;
    }

public:
    AbstractAperture(const Format* m_format);
    virtual ~AbstractAperture();

    bool withHole() const { return m_drillDiam != 0.0; }
    bool isFlashed() const { return m_isFlashed; }

    double drillDiameter() const { return m_drillDiam; }
    double apertureSize();

    Path drawDrill(const State& state);
    Paths draw(const State& state, bool fl = false);

    virtual QString name() const = 0;
    virtual ApertureType type() const = 0;

    double minSize() const { return m_size; }

    virtual bool fit(double toolDiam) const = 0;

protected:
    bool m_isFlashed = false;
    double m_drillDiam = 0.0;
    double m_size = 0.0;

    Paths m_paths;
    virtual void draw() = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual void write(QDataStream& stream) const = 0;
    const Format* m_format;

    void transform(Path& poligon, const State& state);
};

/////////////////////////////////////////////////////
/// \brief The GACircular class
///
class ApCircle final : public AbstractAperture {
public:
    ApCircle(double diam, double drillDiam, const Format* format);
    ApCircle(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    QString name() const;
    ApertureType type() const;
    bool fit(double toolDiam) const;

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;

private:
    double m_diam = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class ApRectangle final : public AbstractAperture {
    friend class Parser;

public:
    ApRectangle(double width, double height, double drillDiam, const Format* format);
    ApRectangle(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    QString name() const;
    ApertureType type() const;
    bool fit(double toolDiam) const;

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class ApObround final : public AbstractAperture {
public:
    ApObround(double width, double height, double drillDiam, const Format* format);
    ApObround(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    QString name() const;
    ApertureType type() const;
    bool fit(double toolDiam) const;

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;

private:
    double m_height = 0.0;
    double m_width = 0.0;
};

/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class ApPolygon final : public AbstractAperture {
public:
    ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const Format* format);
    ApPolygon(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    double rotation() const;
    int verticesCount() const;

    QString name() const;
    ApertureType type() const;
    bool fit(double toolDiam) const;

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;

private:
    double m_diam = 0.0;
    double m_rotation = 0.0;
    int m_verticesCount = 0;
};

/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
class ApMacro final : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QList<QString>& modifiers, const QMap<QString, double>& coefficients, const Format* format);
    ApMacro(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    QString name() const;
    ApertureType type() const;
    bool fit(double) const;

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;

private:
    QString m_macro;
    QList<QString> m_modifiers;
    QMap<QString, double> m_coefficients;

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
class ApBlock final : public AbstractAperture, public QList<GraphicObject> {
public:
    ApBlock(const Format* format);
    ApBlock(QDataStream& stream, const Format* format)
        : AbstractAperture(format)
    {
        read(stream);
    }
    QString name() const;
    ApertureType type() const;
    bool fit(double) const;

protected:
    void draw();
    virtual void read(QDataStream& stream);
    virtual void write(QDataStream& stream) const;
};
}

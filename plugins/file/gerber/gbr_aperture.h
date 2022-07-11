/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  11 November 2021                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#pragma once
#include "gbr_types.h"

#include <QtMath>
#include <numbers>
#include <variant>

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
    friend QDataStream& operator<<(QDataStream& stream, const std::shared_ptr<AbstractAperture>& aperture);
    friend QDataStream& operator>>(QDataStream& stream, std::shared_ptr<AbstractAperture>& aperture);

public:
    AbstractAperture(const File* file);
    virtual ~AbstractAperture();

    bool withHole() const noexcept { return drillDiam_ > 0.0; }
    bool flashed() const noexcept { return isFlashed_; }

    double drillDiameter() const { return drillDiam_; }
    double apertureSize();

    Path drawDrill(const State& state);
    Paths draw(const State& state, bool notApBlock = {});

    virtual QString name() const = 0;
    virtual ApertureType type() const = 0;

    double minSize() const noexcept { return size_; }

    virtual bool fit(double toolDiam) const = 0;

    bool used() const noexcept { return isUsed_; }
    void setUsed(bool isUsed = true) noexcept { isUsed_ = isUsed; }

protected:
    bool isFlashed_ {};
    bool isUsed_ {};
    double drillDiam_ {};
    double size_ {};

    Paths paths_;
    virtual void draw() = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual void write(QDataStream& stream) const = 0;
    const File* file_;

    void transform(Path& poligon, const State& state);
};

/////////////////////////////////////////////////////
/// \brief The GACircular class
///
class ApCircle final : public AbstractAperture {
public:
    ApCircle(double diam, double drillDiam, const File* file);
    ApCircle(QDataStream& stream, const File* file)
        : AbstractAperture(file) {
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
    double diam_ {};
};

/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class ApRectangle final : public AbstractAperture {
    friend class Parser;

public:
    ApRectangle(double width, double height, double drillDiam, const File* file);
    ApRectangle(QDataStream& stream, const File* file)
        : AbstractAperture(file) {
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
    double height_ {};
    double width_ {};
};

/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class ApObround final : public AbstractAperture {
public:
    ApObround(double width, double height, double drillDiam, const File* file);
    ApObround(QDataStream& stream, const File* file)
        : AbstractAperture(file) {
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
    double height_ {};
    double width_ {};
};

/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class ApPolygon final : public AbstractAperture {
public:
    ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const File* file);
    ApPolygon(QDataStream& stream, const File* file)
        : AbstractAperture(file) {
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
    double diam_ {};
    double rotation_ {};
    int verticesCount_ = 0;
};

/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
class ApMacro final : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QList<QString>& modifiers, const VarMap& coefficients, const File* file);
    ApMacro(QDataStream& stream, const File* file)
        : AbstractAperture(file) {
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
    QString macro_;
    QList<QString> modifiers_;
    VarMap coefficients_;

    double Angle(const IntPoint& pt1, const IntPoint& pt2) {
        const double dx = pt2.X - pt1.X;
        const double dy = pt2.Y - pt1.Y;
        const double theta = atan2(-dy, dx) * 360.0 / two_pi;
        const double theta_normalized = theta < 0 ? theta + 360 : theta;
        if (qFuzzyCompare(theta_normalized, double(360)))
            return 0.0;
        else
            return theta_normalized;
    }

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
    ApBlock(const File* file);
    ApBlock(QDataStream& stream, const File* file)
        : AbstractAperture(file) {
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

using ApertureV = std::variant<ApCircle, ApRectangle, ApObround, ApPolygon, ApMacro, ApBlock>;

} // namespace Gerber

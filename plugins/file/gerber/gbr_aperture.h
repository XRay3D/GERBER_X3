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
#pragma once
#include "gbr_types.h"
// #define MT 1
// #include "mathparser.h"

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
    virtual ~AbstractAperture() = default;

    virtual ApertureType type() const = 0;
    virtual QString name() const = 0;
    virtual bool fit(double toolDiam) const = 0;

    Poly drawDrill(const State& state);
    Polys draw(const State& state, bool notApBlock = {});

    bool flashed() const noexcept { return isFlashed_; }
    bool used() const noexcept { return isUsed_; }
    bool withHole() const noexcept { return drillDiam_ > 0.0; }

    double apSize();
    double drillDiameter() const noexcept { return drillDiam_; }
    double minSize() const noexcept { return minSize_; }

    void setUsed(bool isUsed = true) noexcept { isUsed_ = isUsed; }

protected:
    double drillDiam_{};
    double size_{};
    double minSize_{};
    const File* file_;
    Polys paths_;
    bool isFlashed_{};
    bool isUsed_{};

    virtual void draw() = 0;
    virtual void read(QDataStream& stream) = 0;
    virtual void write(QDataStream& stream) const = 0;

    void transform(Poly& poligon, const State& state);
};

/////////////////////////////////////////////////////
/// \brief The GACircular class
///
class ApCircle final : public AbstractAperture {
public:
    ApCircle(double diam, double drillDiam, const File* file);
    ApCircle(QDataStream& stream, const File* file)
        : AbstractAperture{file} {
        read(stream);
    }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;
    void read(QDataStream& stream) override;
    void write(QDataStream& stream) const override;

private:
    double diam_{};
};

/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class ApRectangle final : public AbstractAperture {
    friend class Parser;

public:
    ApRectangle(double width, double height, double drillDiam, const File* file);
    ApRectangle(QDataStream& stream, const File* file)
        : AbstractAperture{file} {
        read(stream);
    }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;
    void read(QDataStream& stream) override;
    void write(QDataStream& stream) const override;

private:
    double height_{};
    double width_{};
};

/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class ApObround final : public AbstractAperture {
public:
    ApObround(double width, double height, double drillDiam, const File* file);
    ApObround(QDataStream& stream, const File* file)
        : AbstractAperture{file} {
        read(stream);
    }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;
    void read(QDataStream& stream) override;
    void write(QDataStream& stream) const override;

private:
    double height_{};
    double width_{};
};

/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class ApPolygon final : public AbstractAperture {
public:
    ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const File* file);
    ApPolygon(QDataStream& stream, const File* file)
        : AbstractAperture{file} {
        read(stream);
    }
    double rotation() const;
    int verticesCount() const;

    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;
    void read(QDataStream& stream) override;
    void write(QDataStream& stream) const override;

private:
    double diam_{};
    double rotation_{};
    int verticesCount_ = 0;
};

/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
using VarMap = std::map<QString, double>;
class ApMacro final : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QList<QString>& modifiers, const VarMap& coefficients, const File* file);
    ApMacro(QDataStream& stream, const File* file)
        : AbstractAperture{file} {
        read(stream);
    }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double) const override;

protected:
    void draw() override;
    void read(QDataStream& stream) override;
    void write(QDataStream& stream) const override;

private:
    QString macro_;
    QList<QString> modifiers_;
    VarMap coefficients_;

    double Angle(const Vec2& pt1, const Vec2& pt2) {
        const double dx = pt2.x() - pt1.x();
        const double dy = pt2.y() - pt1.y();
        const double theta = atan2(-dy, dx) * 360.0 / (2 * pi);
        const double theta_normalized = theta < 0 ? theta + 360 : theta;
        if(qFuzzyCompare(theta_normalized, double(360)))
            return 0.0;
        else
            return theta_normalized;
    }

    Poly drawCenterLine(const mvector<double>& mod);
    Poly drawCircle(const mvector<double>& mod);
    Poly drawOutlineCustomPolygon(const mvector<double>& mod);
    Poly drawOutlineRegularPolygon(const mvector<double>& mod);
    Poly drawVectorLine(const mvector<double>& mod);
    void drawMoire(const mvector<double>& mod);
    void drawThermal(const mvector<double>& mod);
};
/////////////////////////////////////////////////////
/// \brief The ApBlock class
///
class ApBlock final : public AbstractAperture, public QVector<GrObject> {
public:
    ApBlock(const File* file);
    ApBlock(QDataStream& stream, const File* file)
        : AbstractAperture{file} {
        read(stream);
    }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double) const override;

protected:
    void draw() override;
    void read(QDataStream& stream) override;
    void write(QDataStream& stream) const override;
};

using ApertureV = std::variant<ApCircle, ApRectangle, ApObround, ApPolygon, ApMacro, ApBlock>;

} // namespace Gerber

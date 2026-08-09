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
public:
    AbstractAperture(const File* file);
    virtual ~AbstractAperture() = default;

    virtual ApertureType type() const = 0;
    virtual QString name() const = 0;
    virtual bool fit(double toolDiam) const = 0;

    Geo::Polyline drawDrill(const State& state);
    Geo::Polygons draw(const State& state, bool notApBlock = {});

    bool flashed() const noexcept { return isFlashed_; }
    bool used() const noexcept { return isUsed_; }
    bool withHole() const noexcept { return drillDiam_ > 0.0; }

    double size();
    double drillDiameter() const noexcept { return drillDiam_; }
    double minSize() const noexcept { return minSize_; }

    void setUsed(bool isUsed = true) noexcept { isUsed_ = isUsed; }

public:
    // Восстановить геометрию после чтения полей (зовёт движок Serial).
    void postLoad() { draw(); }

protected:
    double drillDiam_{};
    double size_{};
    [[= Serial::skip]] double minSize_{};    // пересчитывает draw()
    [[= Serial::skip]] const File* file_;    // задаёт конструктор (crutch)
    [[= Serial::skip]] Geo::Polygons paths_; // кэш геометрии, пересоберёт draw()
    bool isFlashed_{};
    [[= Serial::skip]] bool isUsed_{}; // рантайм-флаг использования

    virtual void draw() = 0;
};

/////////////////////////////////////////////////////
/// \brief The GACircular class
///
class[[= Serial::name("Circle")]] ApCircle final : public AbstractAperture {
public:
    ApCircle(double diam, double drillDiam, const File* file);
    explicit ApCircle(const File* file)
        : AbstractAperture{file} { }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double diam_{};
};

/////////////////////////////////////////////////////
/// \brief The GARectangle class
///
class[[= Serial::name("Rectangle")]] ApRectangle final : public AbstractAperture {
    friend class Parser;

public:
    ApRectangle(double width, double height, double drillDiam, const File* file);
    explicit ApRectangle(const File* file)
        : AbstractAperture{file} { }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double height_{};
    double width_{};
};

/////////////////////////////////////////////////////
/// \brief The GAObround class
///
class[[= Serial::name("Obround")]] ApObround final : public AbstractAperture {
public:
    ApObround(double width, double height, double drillDiam, const File* file);
    explicit ApObround(const File* file)
        : AbstractAperture{file} { }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double height_{};
    double width_{};
};

/////////////////////////////////////////////////////
/// \brief The GAPolygon class
///
class[[= Serial::name("Polygon")]] ApPolygon final : public AbstractAperture {
public:
    ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const File* file);
    explicit ApPolygon(const File* file)
        : AbstractAperture{file} { }
    double rotation() const;
    int verticesCount() const;

    ApertureType type() const override;
    QString name() const override;
    bool fit(double toolDiam) const override;

protected:
    void draw() override;

private:
    double diam_{};
    double rotation_{};
    int verticesCount_{};
};

/////////////////////////////////////////////////////
/// \brief The GAMacro class
///
using VarMap = std::map<QString, double>;
class[[= Serial::name("Macro")]] ApMacro final : public AbstractAperture {
public:
    ApMacro(const QString& macro, const QStringList& modifiers, const VarMap& coefficients, const File* file);
    explicit ApMacro(const File* file)
        : AbstractAperture{file} { }
    ApertureType type() const override;
    QString name() const override;
    bool fit(double) const override;

protected:
    void draw() override;

private:
    QString macro_;
    QStringList modifiers_;
    VarMap coefficients_;

    double Angle(const QPointF& pt1, const QPointF& pt2) {
        const QPointF d = pt2 - pt1;
        const double theta = atan2(-d.y(), d.x()) * 360.0 / (2 * pi);
        const double theta_normalized = theta < 0 ? theta + 360 : theta;
        if(qFuzzyCompare(theta_normalized, double(360)))
            return 0.0;
        else
            return theta_normalized;
    }

    Geo::Polyline drawCenterLine(const std::vector<double>& mod);
    Geo::Polyline drawCircle(const std::vector<double>& mod);
    Geo::Polyline drawOutlineCustomPolygon(const std::vector<double>& mod);
    Geo::Polyline drawOutlineRegularPolygon(const std::vector<double>& mod);
    Geo::Polyline drawVectorLine(const std::vector<double>& mod);
    void drawMoire(const std::vector<double>& mod);
    void drawThermal(const std::vector<double>& mod);
};
/////////////////////////////////////////////////////
/// \brief The ApBlock class
///
class[[= Serial::name("Block")]] ApBlock final : public AbstractAperture, public QVector<GrObject> {
public:
    using V = QVector<GrObject>;
    ApBlock(const File* file);
    ApertureType type() const override;
    QString name() const override;
    bool fit(double) const override;

protected:
    void draw() override;
};

using ApertureV = std::variant<ApCircle, ApRectangle, ApObround, ApPolygon, ApMacro, ApBlock>;

} // namespace Gerber

// Полиморфные апертуры: {"type":"Circle", <поля flatten>}; имя типа — из
// [[=Serial::name]] на конкретном классе, создание — фабрикой по имени.
// Тела в gbr_aperture.cpp.
template <>
struct Serial::Adapter<std::shared_ptr<Gerber::AbstractAperture>> {
    static void write(Writer& sb, const std::shared_ptr<Gerber::AbstractAperture>& aperture);
    static simdjson::error_code read(simdjson::ondemand::value& val, std::shared_ptr<Gerber::AbstractAperture>& aperture);
};

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
#include "gbr_aperture.h"
#include "gbr_file.h"

#include <QDebug>
#include <QLineF>
#include <QtQml/QJSEngine>
#include <QtQml/QJSValue>

using namespace std::numbers;

namespace Gerber {

AbstractAperture::AbstractAperture(const File* file)
    : file_{file} { }

Geo::Polygons AbstractAperture::draw(const State& state, bool notApBlock) {
    if(state.dCode() == D03 && state.imgPolarity() == Positive && notApBlock)
        isFlashed_ = true;

    if(paths_.empty())
        draw();

    Geo::Polygons retPaths{paths_};

    // Апертура нарисована один раз в своей системе координат; здесь она лишь
    // переносится в точку вспышки с поправками кадра. Методы QTransform
    // домножают справа, поэтому записанное ПОЗЖЕ применяется к точке РАНЬШЕ --
    // отсюда обратный порядок: сначала дюймы, потом зеркало с масштабом,
    // поворот и лишь затем перенос.
    QTransform m;

    if(!state.curPos().isNull())
        m.translate(state.curPos().x(), state.curPos().y());

    if(!qFuzzyIsNull(state.rotating()))
        m.rotate(state.rotating());

    if(!qFuzzyCompare(state.scaling(), 1.0) || state.mirroring())
        m.scale(
            state.mirroring() & X_Mirroring ? -state.scaling() : state.scaling(),
            state.mirroring() & Y_Mirroring ? -state.scaling() : state.scaling());

    // Модификаторы макроса задаются в единицах файла, а тело апертуры мы держим
    // в миллиметрах: дюймовый файл приводится здесь.
    if(file_->format().unitMode == Inches && type() == Macro)
        m.scale(25.4, 25.4);

    // Полярность здесь НЕ применяется: она живёт в state графического объекта,
    // и вычитание отрицательных вспышек делает слой при слиянии (File::merge).
    // Прежний разворот контуров выражал то же самое в плоском списке.
    if(m.type()) retPaths = Geo::transformed(retPaths, m);

    return retPaths;
}

double AbstractAperture::size() {
    if(paths_.empty())
        draw();
    return size_;
}

// Тело отверстия, а не пустота: разворачивать контур незачем, вызывающий
// вычитает его из апертуры сам (см. Parser::addFlash).
Geo::Polyline AbstractAperture::drawDrill(const State& state) {
    if(qFuzzyIsNull(drillDiam_)) return {};
    return Geo::circle(drillDiam_, state.curPos());
}

/////////////////////////////////////////////////////
/// \brief ApCircle::ApCircle
/// \param diam
/// \param drillDiam
/// \param format
///
ApCircle::ApCircle(double diam, double drillDiam, const File* format)
    : AbstractAperture{format} {
    diam_ = diam;
    drillDiam_ = drillDiam;
    // GerberAperture interface
}

QString ApCircle::name() const { return u"C(Ø%1)"_s.arg(diam_); } // CIRCLE

ApertureType ApCircle::type() const { return Circle; }

bool ApCircle::fit(double toolDiam) const { return diam_ > toolDiam; }

void ApCircle::draw() {
    paths_ = Geo::Polygons{Geo::Polylines{Geo::circle(diam_)}};
    minSize_ = size_ = diam_;
}

/////////////////////////////////////////////////////
/// \brief ApRectangle::ApRectangle
/// \param width
/// \param height
/// \param drillDiam
/// \param format
///
ApRectangle::ApRectangle(double width, double height, double drillDiam, const File* format)
    : AbstractAperture{format} {
    width_ = width;
    height_ = height;
    drillDiam_ = drillDiam;
}

QString ApRectangle::name() const // RECTANGLE
{
    if(qFuzzyCompare(width_, height_))
        return u"R(SQ %1)"_s.arg(width_);
    else
        return u"R(%1 x %2)"_s.arg(width_).arg(height_);
}

ApertureType ApRectangle::type() const { return Rectangle; }

bool ApRectangle::fit(double toolDiam) const { return qMin(height_, width_) > toolDiam; }

void ApRectangle::draw() {
    paths_ = Geo::Polygons{Geo::Polylines{Geo::rectangle(width_, height_)}};
    size_ = std::sqrt(width_ * width_ + height_ * height_);
    minSize_ = std::min(width_, height_);
}

/////////////////////////////////////////////////////
/// \brief ApObround::ApObround
/// \param width
/// \param height
/// \param drillDiam
/// \param format
///
ApObround::ApObround(double width, double height, double drillDiam, const File* format)
    : AbstractAperture{format} {
    width_ = width;
    height_ = height;
    drillDiam_ = drillDiam;
}

QString ApObround::name() const { return u"O(%1 x %2)"_s.arg(width_).arg(height_); } // OBROUND

ApertureType ApObround::type() const { return Obround; }

bool ApObround::fit(double toolDiam) const { return qMin(height_, width_) > toolDiam; }

void ApObround::draw() {
    if(qFuzzyCompare(width_, height_)) {
        paths_ = Geo::Polygons{Geo::Polylines{Geo::circle(width_)}};
    } else {
        paths_ = Geo::Polygons{Geo::Polylines{Geo::obround(width_, height_)}};
    }
    size_ = std::max(height_, width_);
    minSize_ = std::min(width_, height_);
}

/////////////////////////////////////////////////////
/// \brief ApPolygon::ApPolygon
/// \param diam
/// \param nVertices
/// \param rotation
/// \param drillDiam
/// \param format
///
ApPolygon::ApPolygon(double diam, int nVertices, double rotation, double drillDiam, const File* format)
    : AbstractAperture{format} {
    diam_ = diam;
    verticesCount_ = nVertices;
    rotation_ = rotation;
    drillDiam_ = drillDiam;
}

double ApPolygon::rotation() const { return rotation_; }

int ApPolygon::verticesCount() const { return verticesCount_; }

QString ApPolygon::name() const { return u"P(Ø%1, N%2)"_s.arg(diam_).arg(verticesCount_); } // POLYGON

ApertureType ApPolygon::type() const { return Polygon; }

bool ApPolygon::fit(double toolDiam) const { return diam_ * cos(pi / verticesCount_) > toolDiam; }

void ApPolygon::draw() {
    Geo::Polyline polygon;
    const double step = 360.0 / verticesCount_;
    const double diam = diam_;
    for(int i: v::iota(0, verticesCount_))
        polygon.emplace_back(
            std::cos(qDegreesToRadians(step * i)) * diam * 0.5,
            std::sin(qDegreesToRadians(step * i)) * diam * 0.5);
    if(!qFuzzyIsNull(rotation_)) rotate(polygon, rotation_);
    // Замкнутость -- ФЛАГ, и без него точный домен контур просто не возьмёт
    // (Polygons отбрасывает незамкнутые), а апертура пропадёт из слоя.
    polygon.close();
    paths_ = Geo::Polygons{Geo::Polylines{polygon}};
    minSize_ = size_ = diam_;
}

/////////////////////////////////////////////////////
/// \brief ApMacro::ApMacro
/// \param macro
/// \param modifiers
/// \param coefficients
/// \param format
///
ApMacro::ApMacro(const QString& macro, const QStringList& modifiers, const VarMap& coefficients, const File* format)
    : AbstractAperture{format}
    , macro_(macro)
    , modifiers_(modifiers)
    , coefficients_(coefficients) {
    while(modifiers_.size() && modifiers_.back().isEmpty())
        modifiers_.removeLast();
}

QString ApMacro::name() const { return u"M(%1)"_s.arg(macro_); } // MACRO

ApertureType ApMacro::type() const { return Macro; }

bool ApMacro::fit(double) const { return true; }

void ApMacro::draw() {
    enum {
        Comment = 0,
        Circle = 1,
        OutlineCustomPolygon = 4,  // MAXIMUM 5000 POINTS
        OutlineRegularPolygon = 5, // 3-12 POINTS
        Moire = 6,
        Thermal = 7,
        VectorLine = 20,
        CenterLine = 21,
    };

    VarMap macroCoefficients{coefficients_};
    using pair = std::pair<bool, Geo::Polyline>;
    std::vector<pair> items;
    try {
        // for (int i{}; i < modifiers_.size(); ++i) {
        // QString var{modifiers_[i]};

        QJSEngine js;
        for(auto&& [name, value]: macroCoefficients)
            js.globalObject().setProperty(name, value);
        for(QString& var: modifiers_) {
            if(var.at(0) == u'0') // Skip Comment
                continue;

            std::vector<double> mod;

            if(var.contains(u'=')) {
                QStringList stringList = var.split(u'=');
                stringList.last().replace(u'x', u'*', Qt::CaseInsensitive);

                auto val = js.evaluate(stringList.last());
                if(val.errorType()) qWarning() << val.toString();
                js.globalObject().setProperty(stringList.first(), val.toNumber());
                continue;
            } else {
                for(auto&& var2: var.split(u',')) {
                    var2.replace(u'x', u'*', Qt::CaseInsensitive);
                    if(var2.contains(u'$')) {
                        auto val = js.evaluate(var2);
                        if(val.errorType()) qWarning() << val.toString();
                        mod.push_back(val.toNumber());
                    } else
                        mod.push_back(var2.toDouble());
                }
            }

            if(mod.size() < 2)
                continue;

            const bool exposure = !qFuzzyIsNull(mod[1]);
            Geo::Polyline path;

            switch(static_cast<int>(mod[0])) {
            case Comment:
                continue;
            case Circle               : path = drawCircle(mod); break;
            case OutlineCustomPolygon : path = drawOutlineCustomPolygon(mod); break;
            case OutlineRegularPolygon: path = drawOutlineRegularPolygon(mod); break;
            case Moire:
                drawMoire(mod);
                return;
            case Thermal:
                drawThermal(mod);
                return;
            case VectorLine: path = drawVectorLine(mod); break;
            case CenterLine: path = drawCenterLine(mod); break;
            }

            // Все примитивы копим ТЕЛАМИ, обходом против часовой стрелки:
            // добавляет примитив материал или вычитает, говорит exposure, а не
            // ориентация контура. В точном домене это разные вещи.
            if(path.size() > 2 && !path.isPositive())
                path.reverse();

            items.emplace_back(exposure, path);
        }
    } catch(...) {
        qWarning() << u"Macro draw error"_s;
        throw u"Macro draw error"_s;
    }

    if(items.empty()) return;

    // Примитивы накладываются по порядку: exposure == 1 добавляет материал,
    // exposure == 0 вычитает. Идущие подряд с одинаковым знаком собираются
    // пачкой -- одна булева операция вместо цепочки из стольких же.
    constexpr auto sameExp = +[](const pair& l, const pair& r) { return l.first == r.first; };
    for(auto&& chunk: v::chunk_by(items, sameExp)) {
        Geo::Polylines contours{std::from_range, v::transform(chunk, &pair::second)};
        const Geo::Polygons part{contours};
        if(chunk.front().first)
            paths_ |= part;
        else
            paths_ -= part;
    }

    {
        auto rect = paths_.boundingRect();
        const double x = rect.width();
        const double y = rect.height();
        size_ = std::sqrt(x * x + y * y);
        minSize_ = std::min(x, y);
    }
}

Geo::Polyline ApMacro::drawCenterLine(const std::vector<double>& mod) {
    enum {
        Width = 2,
        Height,
        CenterX,
        CenterY,
        RotationAngle
    };

    const QPointF center(
        mod[CenterX],
        mod[CenterY]);

    Geo::Polyline polygon = Geo::rectangle(mod[Width], mod[Height], center);

    if(mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        Geo::rotate(polygon, mod[RotationAngle]);

    return polygon;
}

Geo::Polyline ApMacro::drawCircle(const std::vector<double>& mod) {
    enum {
        Diameter = 2,
        CenterX,
        CenterY,
        RotationAngle
    };

    const QPointF center(mod[CenterX], mod[CenterY]);

    Geo::Polyline polygon = Geo::circle(mod[Diameter], center);

    if(mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        Geo::rotate(polygon, mod[RotationAngle]);

    return polygon;
}

void ApMacro::drawMoire(const std::vector<double>& mod) {
    enum {
        CenterX = 1,
        CenterY,
        Diameter,
        Thickness,
        Gap,
        NumberOfRings,
        CrossThickness,
        CrossLength,
        RotationAngle,
    };

    double diameter = mod[Diameter];
    const double thickness = mod[Thickness];
    const double gap = mod[Gap];
    const double ct = mod[CrossThickness];
    const double cl = mod[CrossLength];

    const QPointF center(
        mod[CenterX],
        mod[CenterY]);

    Geo::Polygons moire;

    // Кольца: каждое -- разность двух окружностей, следующее меньше на две
    // толщины и два зазора. Прежде кольцо собиралось из внешней окружности и
    // развёрнутой внутренней в общей куче; теперь это прямая разность.
    if(thickness > 0.0 && gap > 0.0)
        for(int num{}; num < mod[NumberOfRings] && diameter > 0.0; ++num) {
            Geo::Polygons ring{Geo::Polylines{Geo::circle(diameter)}};
            diameter -= thickness * 2;
            if(diameter > 0.0)
                ring -= Geo::Polygons{Geo::Polylines{Geo::circle(diameter)}};
            moire |= ring;
            diameter -= gap * 2;
        }

    if(cl > 0.0 && ct > 0.0) // перекрестье -- два прямоугольника внахлёст
        moire |= Geo::Polygons{
            Geo::Polylines{Geo::rectangle(cl, ct), Geo::rectangle(ct, cl)}
        };

    // Поворот -- вокруг начала координат макроса, уже ПОСЛЕ сдвига в центр
    // примитива. QTransform домножает справа, так что записывается наоборот.
    QTransform m;
    if(mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        m.rotate(mod[RotationAngle]);
    m.translate(center.x(), center.y());

    paths_ = m.type() ? Geo::transformed(moire, m) : moire;
}

Geo::Polyline ApMacro::drawOutlineCustomPolygon(const std::vector<double>& mod) {
    enum {
        NumberOfVertices = 2,
        X,
        Y,
    };

    const size_t num = mod[NumberOfVertices];

    Geo::Polyline polygon;
    for(size_t j: v::iota(0u, num))
        polygon.emplace_back(mod[X + j * 2], mod[Y + j * 2]);
    if(mod.size() > (num * 2u + 3u) && mod.back() > 0)
        Geo::rotate(polygon, mod.back());

    // Контур примитива 4 по спецификации приходит с повтором первой точки в
    // конце; close() снимает повтор и ставит флаг -- без него точный домен
    // контур отбросит, а signedArea() и вовсе нарушит контракт.
    polygon.close();

    return polygon;
}

Geo::Polyline ApMacro::drawOutlineRegularPolygon(const std::vector<double>& mod) {
    enum {
        NumberOfVertices = 2,
        CenterX,
        CenterY,
        Diameter,
        RotationAngle
    };

    const int num = static_cast<int>(mod[NumberOfVertices]);
    if(3 > num || num > 12)
        throw GbrObj::tr("Bad outline (regular polygon) macro!");

    // Именно double: радиус здесь в миллиметрах, и целочисленный тип,
    // оставшийся от uScale, обращал любой реальный многоугольник в точку.
    const double radius = mod[Diameter] * 0.5;
    const QPointF center(
        mod[CenterX],
        mod[CenterY]);

    Geo::Polyline polygon;
    for(int j{}; j < num; ++j) {
        auto angle = qDegreesToRadians(j * 360.0 / num);
        polygon.emplace_back(QPointF(
            qCos(angle) * radius,
            qSin(angle) * radius));
    }
    polygon.close();

    if(mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        Geo::rotate(polygon, mod[RotationAngle]);

    Geo::translate(polygon, center);

    return polygon;
}

void ApMacro::drawThermal(const std::vector<double>& mod) {
    enum {
        CenterX = 1,
        CenterY,
        OuterDiameter,
        InnerDiameter,
        GapThickness,
        RotationAngle
    };

    if(mod[OuterDiameter] <= mod[InnerDiameter] || mod[InnerDiameter] < 0.0 || mod[GapThickness] >= (mod[OuterDiameter] / qPow(2.0, 0.5)))
        throw GbrObj::tr("Bad thermal macro!");

    // Именно double: диаметры здесь в миллиметрах, и целочисленный тип,
    // оставшийся от uScale, обращал термал в 1 мм на любом реальном размере.
    const double outer = mod[OuterDiameter];
    const double inner = mod[InnerDiameter];
    const double gap = mod[GapThickness];

    const QPointF center(
        mod[CenterX],
        mod[CenterY]);

    // Термал -- кольцо, разрезанное крестом на четыре сектора: из внешней
    // окружности вычитается внутренняя и две полосы перекрестья.
    Geo::Polygons thermal{Geo::Polylines{Geo::circle(outer)}};
    thermal -= Geo::Polygons{Geo::Polylines{Geo::circle(inner)}};
    thermal -= Geo::Polygons{
        Geo::Polylines{Geo::rectangle(gap, outer), Geo::rectangle(outer, gap)}
    };

    // Как и у moire: сдвиг в центр примитива, затем поворот вокруг начала
    // координат макроса -- в записи QTransform обратным порядком.
    QTransform m;
    if(mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        m.rotate(mod[RotationAngle]);
    m.translate(center.x(), center.y());

    paths_ = m.type() ? Geo::transformed(thermal, m) : thermal;
}

Geo::Polyline ApMacro::drawVectorLine(const std::vector<double>& mod) {
    enum {
        Width = 2,
        StartX,
        StartY,
        EndX,
        EndY,
        RotationAngle,
    };

    const QPointF start(
        mod[StartX],
        mod[StartY]);
    const QPointF end(
        mod[EndX],
        mod[EndY]);
    const QPointF center(
        0.5 * start.x() + 0.5 * end.x(),
        0.5 * start.y() + 0.5 * end.y());

    Geo::Polyline polygon = Geo::rectangle(Geo::distance(start, end), mod[Width]);
    double angle = 180 - (Geo::angleTo(start, end) - 360); // FIXME ???
    Geo::rotate(polygon, angle);
    Geo::translate(polygon, center);

    if(mod.size() > RotationAngle && mod[RotationAngle] != 0.0)
        Geo::rotate(polygon, mod[RotationAngle]);

    return polygon;
}

/////////////////////////////////////////////////////
/// \brief ApBlock::ApBlock
/// \param macro
/// \param modifiers
/// \param coefficients
/// \param format
///
ApBlock::ApBlock(const File* format)
    : AbstractAperture{format} {
}

QString ApBlock::name() const { return u"BLOCK"_s; }

ApertureType ApBlock::type() const { return Block; }

bool ApBlock::fit(double) const { return true; }

void ApBlock::draw() {

    paths_ = {};

    // Блок -- последовательность вспышек со своей полярностью: положительные
    // добавляют материал, отрицательные вычитают. Идущие подряд с одинаковой
    // полярностью собираются пачкой -- по одной булевой операции на пачку.
    for(std::size_t i{}; i < V::size();) {
        const auto polarity = at(i).state.imgPolarity();
        Geo::Polygons part;
        do
            part |= at(i++).fill;
        while(i < V::size() && at(i).state.imgPolarity() == polarity);

        if(polarity == Positive)
            paths_ |= part;
        else
            paths_ -= part;
    }

    const QRectF rect = paths_.boundingRect();
    size_ = std::hypot(rect.width(), rect.height());
    minSize_ = std::min(rect.width(), rect.height());
}

} // namespace Gerber

void Serial::Adapter<std::shared_ptr<Gerber::AbstractAperture>>::write(
    Writer& sb, const std::shared_ptr<Gerber::AbstractAperture>& aperture) {
    using namespace Gerber;
    sb.start_object();
    sb.append_raw("\"type\":");
    auto dispatch = [&sb]<typename T>(const T& ap) {
        sb.escape_and_append_with_quotes(Serial::typeNameOf<T>());
        Serial::writeInto(sb, ap);
    };
    switch(aperture->type()) {
    case Gerber::Circle   : dispatch(static_cast<const ApCircle&>(*aperture)); break;
    case Gerber::Rectangle: dispatch(static_cast<const ApRectangle&>(*aperture)); break;
    case Gerber::Obround  : dispatch(static_cast<const ApObround&>(*aperture)); break;
    case Gerber::Polygon  : dispatch(static_cast<const ApPolygon&>(*aperture)); break;
    case Gerber::Macro    : dispatch(static_cast<const ApMacro&>(*aperture)); break;
    case Gerber::Block    : dispatch(static_cast<const ApBlock&>(*aperture)); break;
    }
    sb.end_object();
}

simdjson::error_code Serial::Adapter<std::shared_ptr<Gerber::AbstractAperture>>::read(
    simdjson::ondemand::value& val, std::shared_ptr<Gerber::AbstractAperture>& aperture) {
    using namespace Gerber;
    std::string_view slice; // сырой текст элемента: тип подсмотреть + поля прочесть
    if(auto err = simdjson::to_json_string(val).get(slice); err) return err;
    Serial::Parsed peek{slice};
    simdjson::ondemand::object obj;
    if(peek.error || peek.doc.get_object().get(obj)) return simdjson::INCORRECT_TYPE;
    std::string_view type;
    if(auto err = obj["type"].get_string().get(type); err) return err;
    auto make = [&]<typename T>() {
        auto ap = std::make_shared<T>(File::crutch);
        Serial::loadInto(slice, *ap);
        aperture = std::move(ap);
    };
    if(type == Serial::typeNameOf<ApCircle>()) make.template operator()<ApCircle>();
    else if(type == Serial::typeNameOf<ApRectangle>()) make.template operator()<ApRectangle>();
    else if(type == Serial::typeNameOf<ApObround>()) make.template operator()<ApObround>();
    else if(type == Serial::typeNameOf<ApPolygon>()) make.template operator()<ApPolygon>();
    else if(type == Serial::typeNameOf<ApMacro>()) make.template operator()<ApMacro>();
    else if(type == Serial::typeNameOf<ApBlock>()) make.template operator()<ApBlock>();
    else return simdjson::INCORRECT_TYPE;
    return simdjson::SUCCESS;
}

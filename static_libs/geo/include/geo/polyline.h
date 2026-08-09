#pragma once

#include <QPointF>
#include <QRectF>
#include <vector>

class QPainterPath; // forward-declared at global scope: Qt puts it there, not in namespace Geo
class QPolygonF;

using std::numbers::pi;

namespace Geo {

// Расстояние, ниже которого две точки одной полилинии считаются ОДНОЙ, --
// на выходе из точного домена и во всём, что идёт следом.
//
// Свип режет границу на x-монотонные куски и в местах касания рождает
// обрывки в десятки нанометров: для самого домена они безобидны, но в
// bulge-виде это сегменты, которых нет ни на экране, ни в любом выводе.
// Хуже того, у такого обрывка есть ПРОГИБ, и в G-коде он превращается в
// дугу, чьи X и Y после округления совпадают с предыдущими. Координаты
// подавляются как неизменившиеся, остаются одни I/J -- а строка вида
// «G2 I.. J..» означает для станка ПОЛНЫЙ КРУГ.
//
// Величина выбрана по замеру, а не на глаз: в распределении длин сегментов
// (gerber1.gbr, фреза 1 мм) шум разбиения занимает 1e-9..1e-5 мм, настоящая
// геометрия начинается с 1e-3 мм, а на 1e-4 у офсетных контуров пусто. Порог
// стоит ровно в этом промежутке и вдесятеро мельче разрешения вывода.
//
// Живёт в публичном заголовке, потому что этим же числом задан ШАГ СЕТКИ
// вывода: G-код пишет координаты целыми в единицах этого допуска (см.
// GCode::Units). Иначе домен и вывод разошлись бы -- одно считает точки
// разными, другое пишет их одинаково, и ход пропадает.
inline constexpr double exitWeldTolerance = 1e-4;

// A single vertex of a polyline. `bulge` describes the curvature of the
// segment that starts at this vertex (0 == straight line to the next
// vertex) -- the DXF LWPOLYLINE convention, kept here because it is the
// most compact exact description of an arc through two points.
struct Vertex : QPointF {
    // Направление обхода сегмента, начинающегося в этой вершине. Значения
    // знаковые: направление -- это знак прогиба (и знак угла дуги), а не
    // произвольная метка, поэтому им можно домножать.
    enum Dir {
        Cw = -1,  // по часовой стрелке
        Line = 0, // прямой сегмент
        Ccw = 1   // против часовой стрелки
    };

    Vertex(double x, double y, double b = {}): QPointF{x, y}, bulge{b} { }
    Vertex(const QPointF& pt, double b = {}): QPointF{pt}, bulge{b} { }
    double bulge{};

    constexpr bool isArc() const noexcept { return bulge != 0.0; }
    constexpr Dir dir() const noexcept {
        return bulge > 0.0 ? Ccw : bulge < 0.0 ? Cw
                                               : Line;
    }
};

struct Polyline : std::vector<Vertex> {
    using vector = std::vector<Vertex>;
    using vector::vector;
    using vector::operator=;
    Polyline(const QPolygonF& pgn);

    Polyline() = default;
    Polyline(Polyline&&) = default;
    Polyline(const Polyline&) = default;
    Polyline& operator=(Polyline&&) = default;
    Polyline& operator=(const Polyline&) = default;

    bool closed{};
    double width{}; // constant line width (DXF group 43); 0 == unspecified

    // Builds the QPainterPath for this polyline, expanding bulges into arcs.
    QPainterPath toPath() const;
    operator QPolygonF() const;

    // Разворачивает полилинию в обратном направлении на месте: порядок
    // вершин переворачивается, а прогибы (bulge) переносятся на новый
    // "исходящий" сегмент с обратным знаком -- так форма контура (включая
    // дуги) остаётся прежней, только пройденной в обратную сторону.
    void reverse();

    // То же, но копией -- для выражений, где разворачивать исходник нельзя.
    Polyline reversed() const;

    // Замыкает полилинию: выставляет флаг и убирает последнюю вершину, если
    // она дублирует первую. Замкнутость здесь -- ФЛАГ, а не повтор точки, и
    // парсеры (DXF, Gerber) обычно приносят контур как раз с повтором.
    Polyline& close();

    // Знаковая площадь ЗАМКНУТОЙ полилинии (с учётом дуговых сегментов):
    // положительная при обходе против часовой стрелки, отрицательная по
    // часовой. Именно знаком Polygon отличает внешнюю границу от
    // отверстия, поэтому он нужен отдельно от area().
    double signedArea() const pre(closed);

    // Площадь фигуры, ограниченной ЗАМКНУТОЙ полилинией (по модулю), с
    // учётом дуговых сегментов -- не только хорд, как в обычной формуле
    // шнурков по одним вершинам.
    double area() const pre(closed);

    // Габаритный прямоугольник -- ТОЧНЫЙ, а не по одним вершинам: дуга
    // выпирает за хорду, и её крайние точки (там, где касательная
    // горизонтальна или вертикальна) учитываются, когда попадают в размах.
    QRectF boundingRect() const;

    // Периметр полилинии -- сумма длин всех её сегментов; для дугового
    // сегмента берётся длина самой дуги, а не хорды между её концами.
    double perimeter() const;

    bool isClosed() const { return closed; }
    bool isPositive() const { return signedArea() > 0; }
};

struct Polylines : std::vector<Polyline> {
    using vector = std::vector<Polyline>;
    using vector::vector;
    using vector::operator=;

    QPainterPath toPath() const;
};

// Сериализация. Формат -- ровно хранимое состояние: вершина это точка плюс
// прогиб, полилиния -- счётчик, флаги и вершины. Ни габарит, ни площадь не
// пишутся: они целиком выводятся из вершин.
} // namespace Geo

#pragma once

// Вспомогательная обвязка над плоским bulge-видом: математика дуг, построение
// примитивов и трансформации.
//
// Здесь НЕТ точной геометрии -- всё считается в double и работает с Polyline
// как с данными. Точный домен (CGAL) начинается в polygon.h, и всё, что
// строится здесь, попадает туда обычным путём -- через конструктор Polygons.
//
// Отдельный заголовок, а не довесок к polyline.h: полилиния описывает СЕБЯ
// (площадь, габарит, путь Qt), а тут -- операции НАД ней, которые самому типу
// знать незачем.

#include "geo/polyline.h"

#include <optional>
#include <ranges>
#include <utility>
#include <vector>

// Оба типа объявлены, а не включены: в объявлениях они нужны только по ссылке,
// а <QTransform>, втянутый в публичный заголовок, ломает порядок включения
// заголовков Qt у тех, кто подключает Geo раньше QtCore.
class QPainterPath;
class QTransform;

using namespace std::numbers;

namespace Geo {

class Polygon;
class Polygons;

// Дуга, восстановленная из пары точек и прогиба между ними, -- то самое, что
// Vertex умышленно не хранит. Существует ровно на время расчёта: рисования,
// длины, площади, вывода G2/G3.
struct Arc {
    QPointF center;
    double radius{};     // всегда > 0
    double startAngle{}; // atan2 в начальной точке, радианы
    double theta{};      // знаковый угол дуги, радианы: > 0 -- против часовой

    constexpr Vertex::Dir dir() const noexcept {
        return theta > 0.0 ? Vertex::Ccw : theta < 0.0 ? Vertex::Cw
                                                       : Vertex::Line;
    }
    constexpr double endAngle() const noexcept { return startAngle + theta; }
    double length() const noexcept;
    // Точка на дуге: 0 -- начало, 1 -- конец.
    QPointF pointAt(double t) const noexcept;
    // Знаковая площадь кругового сегмента -- «довеска» между дугой и её
    // хордой. Знак совпадает со знаком theta, так что её достаточно просто
    // прибавить к обычной знаковой площади по вершинам.
    double segmentArea() const noexcept;
};

// Геометрия сегмента p1 -> p2 с прогибом bulge; nullopt -- сегмент прямой
// (bulge == 0) или вырожденный (точки совпали).
std::optional<Arc> arcOf(QPointF p1, QPointF p2, double bulge);

// Знаковый угол дуги p1 -> p2 вокруг центра в заданном направлении:
// (0, 2*pi] для Ccw, [-2*pi, 0) для Cw. Обратная задача к arcOf -- нужна там,
// где дуга приходит извне в виде «центр + направление» (DXF ARC, G2/G3).
double arcSweep(QPointF p1, QPointF p2, QPointF center, Vertex::Dir dir);

// Размах, который прогибом ещё выражается: полный оборот им не выражается
// вовсе (bulge = tan(theta/4), на 2*pi это бесконечность), и окружность
// приходится держать двумя половинами.
inline constexpr double maxBulgeSweep = 2.0 * pi - 1e-9;

// Прогиб по знаковому углу дуги. Полный оборот (|theta| == 2*pi) прогибом не
// выражается вовсе -- окружность приходится резать хотя бы надвое (см. circle).
double bulgeOf(double theta);
double bulgeOf(QPointF p1, QPointF p2, QPointF center, Vertex::Dir dir);

// Расстояние между точками. Мелочь, но своя: geometry.h с его Length ушёл, а
// считать hypot по разностям координат приходится всюду.
double distance(QPointF a, QPointF b);

// Длина сегмента с учётом прогиба: у дуги берётся её длина, а не длина хорды.
double segmentLength(const Vertex& from, const Vertex& to);

// Сшивает подряд идущие дуги ОДНОЙ окружности в одну: соседние вершины, чьи
// дуги сходятся центром и радиусом в пределах `tolerance` и идут в одну
// сторону, заменяются одной с суммарным размахом.
//
// Нужна потому, что дуга приходит нарезанной: точный свип рубит границу по
// вертикальным касательным, а офсет складывает её из отдельных капсул. Для
// вывода это по-прежнему ОДНА дуга -- и на экране, и в G-коде (G2/G3 одной
// строкой вместо десятка).
//
// Прогиб больше единицы -- законная дуга больше полуокружности, центр у неё
// берётся тем же arcOf. Не выражается прогибом только ПОЛНЫЙ оборот
// (tan(theta/4) на 2*pi -- бесконечность), поэтому окружность так и остаётся
// двумя половинами; одной командой её выводит уже G-код.
//
// Замыкающая пара (последняя вершина -- первая) не трогается: сшивка там
// сдвинула бы начало контура, а им задана врезка.
void stitchArcs(Polyline& polyline, double tolerance = exitWeldTolerance);

// Сегменты полилинии парами вершин. У ЗАМКНУТОЙ последним идёт сегмент от
// последней вершины к первой -- тот самый, которого нет в списке вершин.
// Прогиб сегмента лежит на его начальной вершине, поэтому по одной вершине
// сегмент не восстановить, и пары приходится собирать явно.
//
// Отдаётся ПРЕДСТАВЛЕНИЕМ: пары ссылок на сами вершины, без копий и без
// единого выделения памяти. Прежде здесь собирался вектор пар -- 48 байт на
// сегмент за вызов, а зовут его на каждый путь при выводе УП.
//
// «concat(drop(1), take(wrap))» -- это и есть поворот на один: сперва все
// вершины кроме первой, затем первая. У незамкнутой второй кусок пуст, и
// последняя вершина ни с кем в пару не встаёт -- ровно как надо.
//
// Представление ССЫЛАЕТСЯ на полилинию и жить дольше неё не может. В
// range-for это безопасно даже со временной (C++23 продлевает ей жизнь на
// весь цикл); сохранять его в переменную дольше самой полилинии -- нельзя.
inline auto segments(const Polyline& polyline) {
    // У одной-единственной вершины замыкать нечего: сегмент из неё в саму
    // себя -- не сегмент.
    const std::size_t wrap = polyline.closed && polyline.size() > 1;
    return std::views::zip(polyline,
        std::views::concat(polyline | std::views::drop(1), polyline | std::views::take(wrap)));
}

//------------------------------------------------------------------------------
// Примитивы. Все замкнуты, обойдены против часовой стрелки (signedArea > 0) и
// хранят ДУГИ, а не ломаные: прогиб -- точное описание дуги через две точки,
// и до самого выхода в CGAL или на экран огрублять её незачем.

// Окружность -- две полуокружности с прогибом 1: полный оборот прогибом не
// выражается (tan(2*pi/4) -- бесконечность).
Polyline circle(double diameter, QPointF center = {});

Polyline rectangle(double width, double height, QPointF center = {});

// Прямоугольник со скруглёнными торцами по короткой стороне -- апертура
// Gerber «obround». При width == height вырождается в окружность.
Polyline obround(double width, double height, QPointF center = {});

// Дуга по центру и знаковому размаху (радианы, > 0 -- против часовой). Режется
// на куски не длиннее полуокружности -- иначе прогиб уходит в бесконечность.
// |sweep| >= 2*pi даёт замкнутую окружность.
Polyline arc(QPointF center, double radius, double startAngle, double sweep);

//------------------------------------------------------------------------------
// Трансформации.
//
// Прогиб безразмерен (отношение стрелы к полухорде), поэтому сдвиг, поворот и
// РАВНОМЕРНЫЙ масштаб его не трогают вовсе, а зеркало лишь разворачивает обход
// дуги -- меняет знак. Неравномерный же масштаб превращает дугу в эллипс,
// которого в bulge-виде нет, поэтому для полилиний С ДУГАМИ он запрещён.

// Линейная часть -- подобие (поворот + равномерный масштаб, возможно с
// зеркалом)? Только при нём дуга остаётся дугой.
bool isSimilarity(const QTransform& tr);

// Есть ли в полилинии хоть один дуговой сегмент.
bool hasArcs(const Polyline& polyline);

Polyline& transform(Polyline& polyline, const QTransform& tr)
    pre(isSimilarity(tr) || !hasArcs(polyline));
Polylines& transform(Polylines& polylines, const QTransform& tr);

Polyline& translate(Polyline& polyline, QPointF offset);
Polylines& translate(Polylines& polylines, QPointF offset);

// Угол в ГРАДУСАХ, как и везде в Qt.
Polyline& rotate(Polyline& polyline, double angle, QPointF center = {});
Polylines& rotate(Polylines& polylines, double angle, QPointF center = {});

// Для точного домена трансформация -- не операция на месте: регион пересобирается
// из контуров тем же свипом, что и при любом другом входе. Зеркало вдобавок
// разворачивает обход каждого контура, и канон ориентаций (внешняя против
// часовой, дырки по часовой) восстанавливается явно.
//
// Чистый перенос (QTransform::TxTranslate) уходит в Geo::translated и
// остаётся точным до бита -- ни double, ни повторная сборка дуг тут не
// участвуют. Поворот и масштаб точного пути не имеют (иррациональный
// cos/sin негде хранить в рациональном домене CGAL) и проходят через
// bulge-вид -- ровно тем же transform(), что и выше, с тем же контрактом:
// дуга требует подобия (поворот + равномерный масштаб, возможно с
// зеркалом), неравномерный масштаб для контуров с дугами не определён.
Polygon transformed(const Polygon& polygon, const QTransform& tr);
Polygons transformed(const Polygons& polygons, const QTransform& tr);

//------------------------------------------------------------------------------
// Операторы: тонкая обёртка над функциями выше, для тех, кому естественнее
// писать `contour * tr`, чем `Geo::transform(contour, tr)`. `*=` -- на
// месте (bulge-вид это умеет буквально), `*` -- копией. У Polygon/Polygons
// «на месте» иллюзорно -- точный домен пересобирается всегда, `*=` там
// просто присваивает результат обратно, -- но синтаксис одинаков везде.
Polyline operator*(Polyline polyline, const QTransform& tr);
Polyline& operator*=(Polyline& polyline, const QTransform& tr);
Polylines operator*(Polylines polylines, const QTransform& tr);
Polylines& operator*=(Polylines& polylines, const QTransform& tr);

Polygon operator*(const Polygon& polygon, const QTransform& tr);
Polygon& operator*=(Polygon& polygon, const QTransform& tr);
Polygons operator*(const Polygons& polygons, const QTransform& tr);
Polygons& operator*=(Polygons& polygons, const QTransform& tr);

//------------------------------------------------------------------------------

// Контуры пути Qt. Кубики, оказавшиеся дугой окружности, восстанавливаются в
// прогиб; остальные дробятся на отрезки не грубее `tolerance`.
Polylines fromPath(const QPainterPath& path, double tolerance = 5e-3);

// Все контуры одним путём -- каждый своим подпутём. Заливка получившегося пути
// зависит от правила заполнения, которое выставляет уже вызывающий: набор
// контуров сам по себе не говорит, где тело, а где дырка.
QPainterPath toPath(const Polylines& polylines);

constexpr double angleTo(const QPointF& pt1, const QPointF& pt2) noexcept {
    const QPointF d               = pt2 - pt1;
    const double theta            = atan2(-d.y(), d.x()) * 360.0 / (pi * 2);
    const double theta_normalized = theta < 0 ? theta + 360 : theta;
    if(qFuzzyCompare(theta_normalized, double(360)))
        return 0.0;
    else
        return theta_normalized;
}

constexpr double angleRadTo(const QPointF& pt1, const QPointF& pt2) noexcept {
    const QPointF d    = pt2 - pt1;
    const double theta = atan2(-d.y(), d.x());
    return theta;
    const double theta_normalized = theta < 0 ? theta + (pi * 2) : theta; // NOTE theta_normalized
    if(qFuzzyCompare(theta_normalized, (pi * 2)))
        return 0.0;
    else
        return theta_normalized;
}

} // namespace Geo

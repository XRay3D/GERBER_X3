#pragma once

// Полигон и набор полигонов -- ОБЁРТКИ НАД CGAL.
//
// Хранимое состояние здесь -- точное (рациональное) представление CGAL:
// Polygon_with_holes_2 у Polygon и General_polygon_set_2 у Polygons. Наружу
// геометрия отдаётся только путём Qt (toPath) -- обратного перевода в
// bulge-вид (Geo::Polyline) здесь нет, он и не нужен никому.
//
// Это принципиально. Пока результат операции живёт в CGAL-домене, цепочка
// операций (раздуть -> объединить -> вычесть) остаётся ТОЧНОЙ: центры и
// радиусы дуг проходят её неизменными, а самокасания не возникают вовсе.
// Стоит же вернуть геометрию в double и подать обратно -- и микрофрагменты
// разбиения превращаются в вырожденные кривые, на которых точный свип
// спотыкается. Обёртка ровно для того и нужна: держать данные в точном
// домене, а округлять лишь на самом выходе -- в DXF или на экран.
//
// Сам CGAL спрятан за PIMPL: его заголовки тяжёлые (EPECK + GMP), и тянуть
// их в каждый файл проекта незачем. Держится он в std::indirect -- это
// PIMPL с ЗНАЧЕНИЙНОЙ семантикой: копирование, перемещение и уничтожение
// достаются готовыми (в отличие от unique_ptr, которому глубокую копию
// пришлось бы писать руками), а const прокидывается внутрь, так что
// константный Polygon не отдаёт неконстантную ссылку на своё нутро.
// Неполный тип Impl std::indirect держать умеет; определения специальных
// функций-членов просто уезжают в .cpp, где Impl уже полон.

#include "geo/polyline.h"

#include <memory>
#include <vector>

class QPainterPath;
class QDataStream;

namespace Geo {
#if 1

// Возьмёт ли точная геометрия такой контур: замкнут, невырожден и не
// касается сам себя. Ровно этот отбор идёт внутри Polygons -- контур, не
// прошедший его, в регион просто не попадает.
//
// Наружу вынесено ради тех, кто контуры ПРАВИТ (см. ArcRecovery.h): им
// нужно узнать, не сделала ли правка контур непригодным, не таща к себе
// заголовки CGAL.
bool isExactContour(const Polyline& contour);

// Полигон с отверстиями: одна внешняя граница плюс сколько угодно дырок
// внутри неё. Плоский список полилиний такого не выражает -- там дырка
// такой же контур, как всё прочее, и связывать её со «своим» телом
// приходится вызывающему.
//
// Канон Dxf-вида: внешняя граница обходится против часовой стрелки
// (signedArea > 0), дырки -- по часовой. При нём toPath() с WindingFill
// заливает ровно тело полигона.
class Polygon {
public:
    Polygon();
    ~Polygon();
    Polygon(const Polygon&);
    Polygon(Polygon&&) noexcept;
    Polygon& operator=(const Polygon&);
    Polygon& operator=(Polygon&&) noexcept;

    // Из Dxf-контуров: границы переводятся в точное представление, дырки
    // берутся как есть (их принадлежность этому телу задаёт вызывающий).
    explicit Polygon(const Polyline& outer, const Polylines& holes = {});

    bool empty() const;
    // std::size_t size() const;

    // Внешняя граница и отверстия в Dxf-виде -- материализуются из точного
    // представления при первом обращении и кэшируются.
    const Polyline& outer() const;
    const Polylines& holes() const;

    // Все контуры разом (внешний, затем дырки) -- для кода, который ещё
    // работает полилиниями.
    Polylines contours() const;

    double area() const;      // тело: внешняя минус дырки
    double perimeter() const; // вся граница: внешняя плюс дырки
    QRectF boundingRect() const;
    bool contains(QPointF point) const;

    // Путь с WindingFill: дырки идут навстречу внешней границе и
    // вычитаются сами.
    QPainterPath toPath() const;

    struct Impl;
    explicit Polygon(std::unique_ptr<Impl> impl); // из точного представления, внутренний
    const Impl& impl() const { return *impl_; }

private:
    std::unique_ptr<Impl> impl_;
};

// Набор полигонов -- регион (в терминах CGAL, General_polygon_set_2), а не
// просто вектор: булевы операции над ним точные и выполняются НЕ выходя из
// CGAL-домена, поэтому цепочку операций можно строить сколь угодно длинной
// без потери точности.
class Polygons {
public:
    Polygons();
    ~Polygons();
    Polygons(const Polygons&);
    Polygons(Polygons&&) noexcept;
    Polygons& operator=(const Polygons&);
    Polygons& operator=(Polygons&&) noexcept;

    Polygons(std::initializer_list<Polygon> polygons);
    explicit Polygons(const Polygon& polygon);

    // Из плоского списка контуров: вложенность задаёт ориентация (контур с
    // отрицательной площадью -- пустота внутри чьего-то тела), ровно как
    // её и выражает DXF.
    explicit Polygons(const Polylines& contours);

    bool empty() const;
    std::size_t size() const;

    // Разложение региона на полигоны с отверстиями -- материализуется при
    // первом обращении и кэшируется.
    const std::vector<Polygon>& all() const;
    const Polygon& operator[](std::size_t index) const { return all()[index]; }
    auto begin() const { return all().begin(); }
    auto end() const { return all().end(); }

    Polylines contours() const; // плоский список всех контуров
    double area() const;
    QRectF boundingRect() const;
    bool contains(QPointF point) const;
    QPainterPath toPath() const;

    // Сумма Минковского ГРАНИЦЫ региона с кругом радиуса `radius` -- полоса
    // толщиной 2*radius вдоль всей границы, и внешней, и границ отверстий.
    //
    // Это общий кирпич обоих офсетов сразу (см. geo/boolean.h): раздутый
    // регион -- это он же, объединённый с самим регионом, а сжатый --
    // вычтенный из него. Точка остаётся в сжатом регионе ровно тогда, когда
    // она в регионе и дальше radius от его границы, то есть вне полосы, --
    // так что вычитание и есть эрозия, без всякого приближения.
    Polygons boundaryBand(double radius) const;

    // Булевы операции -- точные, целиком внутри CGAL-домена.
    Polygons& operator|=(const Polygons& other); // объединение
    Polygons& operator&=(const Polygons& other); // пересечение
    Polygons& operator-=(const Polygons& other); // разность
    Polygons& operator^=(const Polygons& other); // симметрическая разность

    friend Polygons operator|(Polygons l, const Polygons& r) { return l |= r; }
    friend Polygons operator&(Polygons l, const Polygons& r) { return l &= r; }
    friend Polygons operator-(Polygons l, const Polygons& r) { return l -= r; }
    friend Polygons operator^(Polygons l, const Polygons& r) { return l ^= r; }

    struct Impl;
    explicit Polygons(std::unique_ptr<Impl> impl); // из точного представления, внутренний
    Impl& impl() { return *impl_; }
    const Impl& impl() const { return *impl_; }

private:
    std::unique_ptr<Impl> impl_;
};

// Сериализация -- через КОНТУРЫ, а не через точное представление: последнее
// рационально (числа неограниченной длины), и укладывать его в поток незачем
// -- при чтении оно всё равно строится заново тем же свипом, что и при любом
// другом входе. Плата -- округление координат до double, ровно то же, что и
// при всякой выдаче геометрии наружу.
//
// Полигоны пишутся поштучно (граница + свои дырки), а не плоским списком
// контуров: в плоском виде вложенность выражена одной ориентацией, и остров
// внутри дырки при чтении вычелся бы вместе с ней.
QDataStream& operator<<(QDataStream& stream, const Polygon& polygon);
QDataStream& operator>>(QDataStream& stream, Polygon& polygon);
QDataStream& operator<<(QDataStream& stream, const Polygons& polygons);
QDataStream& operator>>(QDataStream& stream, Polygons& polygons);

#else
// Возьмёт ли точная геометрия такой контур: замкнут, невырожден и не
// касается сам себя. Ровно этот отбор идёт внутри Polygons -- контур, не
// прошедший его, в регион просто не попадает.
//
// Наружу вынесено ради тех, кто контуры ПРАВИТ (см. geo/arcrecovery.h): им
// нужно узнать, не сделала ли правка контур непригодным, не таща к себе
// заголовки CGAL.
bool isExactContour(const Polyline& contour);

// Полигон с отверстиями: одна внешняя граница плюс сколько угодно дырок
// внутри неё. Плоский список полилиний такого не выражает -- там дырка
// такой же контур, как всё прочее, и связывать её со «своим» телом
// приходится вызывающему.
//
// Канон плоского вида: внешняя граница обходится против часовой стрелки
// (signedArea > 0), дырки -- по часовой. При нём toPath() с WindingFill
// заливает ровно тело полигона.
class Polygon {
public:
    Polygon();
    ~Polygon();
    Polygon(const Polygon&);
    Polygon(Polygon&&) noexcept;
    Polygon& operator=(const Polygon&);
    Polygon& operator=(Polygon&&) noexcept;

    // Путь с WindingFill: дырки идут навстречу внешней границе и
    // вычитаются сами.
    QPainterPath toPath() const;

    struct Impl;
    explicit Polygon(Impl&& impl); // из точного представления, внутренний

private:
    std::indirect<Impl> impl_;
};

// Набор полигонов -- регион (в терминах CGAL, General_polygon_set_2), а не
// просто вектор: булевы операции над ним точные и выполняются НЕ выходя из
// CGAL-домена, поэтому цепочку операций можно строить сколь угодно длинной
// без потери точности.
class Polygons {
public:
    Polygons();
    ~Polygons();
    Polygons(const Polygons&);
    Polygons(Polygons&&) noexcept;
    Polygons& operator=(const Polygons&);
    Polygons& operator=(Polygons&&) noexcept;

    // Из плоского списка контуров: вложенность задаёт ориентация (контур с
    // отрицательной площадью -- пустота внутри чьего-то тела), ровно как
    // её и выражает DXF.
    explicit Polygons(const Polylines& contours);

    // Разложение региона на полигоны с отверстиями -- материализуется при
    // первом обращении и кэшируется.
    const std::vector<Polygon>& all() const;
    auto begin() const { return all().begin(); }
    auto end() const { return all().end(); }

    QPainterPath toPath() const;

    // Сумма Минковского ГРАНИЦЫ региона с кругом радиуса `radius` -- полоса
    // толщиной 2*radius вдоль всей границы, и внешней, и границ отверстий.
    //
    // Это общий кирпич обоих офсетов сразу (см. geo/boolean.h): раздутый
    // регион -- это он же, объединённый с самим регионом, а сжатый --
    // вычтенный из него. Точка остаётся в сжатом регионе ровно тогда, когда
    // она в регионе и дальше radius от его границы, то есть вне полосы, --
    // так что вычитание и есть эрозия, без всякого приближения.
    Polygons boundaryBand(double radius) const;

    // Булевы операции -- точные, целиком внутри CGAL-домена.
    Polygons& operator|=(const Polygons& other); // объединение
    Polygons& operator&=(const Polygons& other); // пересечение
    Polygons& operator-=(const Polygons& other); // разность
    Polygons& operator^=(const Polygons& other); // симметрическая разность

    friend Polygons operator|(Polygons l, const Polygons& r) { return l |= r; }
    friend Polygons operator&(Polygons l, const Polygons& r) { return l &= r; }
    friend Polygons operator-(Polygons l, const Polygons& r) { return l -= r; }
    friend Polygons operator^(Polygons l, const Polygons& r) { return l ^= r; }

    struct Impl;

private:
    std::indirect<Impl> impl_;
};
#endif

} // namespace Geo

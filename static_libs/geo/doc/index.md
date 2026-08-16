# Geo — справочник

`static_libs/geo` — геометрическое ядро проекта: разбор и хранение
контуров, точные булевы операции, офсет, штриховка, восстановление дуг и
нормализация чертёжной геометрии. Отдельная библиотека от разбора
DXF/Gerber и от GUI — работает с уже готовыми точками и контурами.

Это подробная справка по её публичному API, в духе документации
[Clipper2](https://www.angusj.com/clipper2/Docs/_Body.htm): каждая страница
объясняет одно понятие с нуля, с диаграммой и рабочими примерами кода.

## Два домена

Всё в Geo живёт в одном из двух представлений, и почти любая ошибка в коде,
использующем библиотеку, — это попытка применить операцию не из того
домена.

**Bulge-вид** (`Polyline`/`Polylines`, [заголовок `polyline.h`](../include/geo/polyline.h))
— плоский список вершин с прогибом (`bulge`), считается в `double`.
Дешёвый, годится для рисования, измерения, простых трансформаций. Дуги
хранятся точно (через прогиб), но сама геометрия — приближённая: конец
цепочки операций в `double` накапливает ошибку.

**Точный домен** (`Polygon`/`Polygons`, [заголовок `polygon.h`](../include/geo/polygon.h))
— обёртка над CGAL (`Polygon_with_holes_2`/`General_polygon_set_2`),
рациональная арифметика. Булевы операции ([`boolean.h`](../include/geo/boolean.h))
и офсет здесь точные и не накапливают ошибку сколь угодно длинной цепочкой.
Дороже bulge-вида, но именно в нём совершаются раздутие, объединение,
вычитание.

Переход между доменами односторонний по духу: контуры **входят** в точный
домен через конструкторы `Polygon`/`Polygons`, а **выходят** только путём
Qt (`toPath()`) или контурами (`outer()`/`holes()`/`contours()`) —
обратной сборки в чистый bulge-вид с сохранением точности нет и не нужно
никому, потому что как только геометрия вернулась в `double`, дальше
точного домена уже нет смысла.

## Минимальный пример

От разрозненных сущностей чертежа до траектории:

```cpp
#include "geo/arcrecovery.h"
#include "geo/boolean.h"
#include "geo/fill.h"

using namespace Geo;

// 1. Дуги, разобранные экспортёром на отрезки, -- обратно в bulge.
Polylines layer = /* контуры слоя, как их отдал парсер DXF */;
ArcRecovery::recover(layer);

// 2. DXF: ориентация произвольна, контур разрезан на сущности.
const Normalized norm = normalize(std::move(layer), exitWeldTolerance);

// 3. Точный домен: раздуть границу на ширину фрезы.
const Polygons offset = Inflate(norm.region, -toolDiameter);

// 4. Готовая траектория -- заливка змейкой.
const Polylines toolpath = zigzagFill(offset, toolDiameter * 0.6);
```

## Оглавление

1. [Vertex и bulge](vertex-and-bulge.md) — как дуга кодируется двумя
   точками и одним числом; `Arc`, `arcOf`, `exitWeldTolerance`.
2. [Polyline и Polylines](polyline.md) — плоский bulge-вид: мерки с учётом
   дуг, `segments()`, `stitchArcs`, примитивы (`circle`, `rectangle`,
   `obround`, `arc`).
3. [Polygon и Polygons](polygon-and-polygons.md) — точный CGAL-домен: канон
   ориентаций, три конструктора `Polygons`, `invert()` против
   `complement()`.
4. [Раздутие и сжатие (Inflate)](offsetting.md) — офсет границы, поведение
   дырок, морфологическое замыкание/открытие.
5. [Булевы операции (BooleanOp)](boolean-ops.md) — union/intersection/
   difference/xor, `FillRule`.
6. [Резка открытых путей (clipOpen)](clip-open.md) — клиппинг траекторий, а
   не площадей.
7. [Нормализация чертёжной геометрии](normalize.md) — `stitch`, `evenOdd`,
   почему DXF не проходит канон Geo как есть.
8. [Заполнение региона строками (zigzagFill)](fill.md) — боустрофедон.
9. [Восстановление дуг (ArcRecovery)](arc-recovery.md) — ломаная → дуга по
   регулярности.
10. [Трансформации](transform.md) — перенос/поворот/масштаб, когда дуга
    остаётся дугой.
11. [Отмена долгих вычислений](cancellation.md) — `CancelScope`,
    кооперативная отмена.
12. [Сериализация (JSON)](serialization.md) — компактная кодировка,
    `Serial::Adapter`.
13. [Сборка](building.md) — зависимости, флаги CGAL, тесты.

## Карта заголовков

| Заголовок | Содержимое | Страница |
|---|---|---|
| [`polyline.h`](../include/geo/polyline.h) | `Vertex`, `Polyline`, `Polylines`, `exitWeldTolerance` | [Vertex и bulge](vertex-and-bulge.md), [Polyline](polyline.md) |
| [`polygon.h`](../include/geo/polygon.h) | `Polygon`, `Polygons`, `isExactContour`, `translated` | [Polygon и Polygons](polygon-and-polygons.md) |
| [`util.h`](../include/geo/util.h) | `Arc`, `arcOf`/`arcSweep`/`bulgeOf`, примитивы, `segments`, трансформации | [Vertex и bulge](vertex-and-bulge.md), [Polyline](polyline.md), [Трансформации](transform.md) |
| [`boolean.h`](../include/geo/boolean.h) | `Inflate`, `BooleanOp`, `clipOpen`, `stitch`/`evenOdd`/`normalize` | [Inflate](offsetting.md), [BooleanOp](boolean-ops.md), [clipOpen](clip-open.md), [Нормализация](normalize.md) |
| [`fill.h`](../include/geo/fill.h) | `zigzagFill` | [Заполнение](fill.md) |
| [`arcrecovery.h`](../include/geo/arcrecovery.h) | `ArcRecovery::recover`/`stitch`, `Settings` | [ArcRecovery](arc-recovery.md) |
| [`cancel.h`](../include/geo/cancel.h) | `CancelScope`, `Cancelled` | [Отмена](cancellation.md) |
| [`geo_json.h`](../include/geo/geo_json.h) | `Serial::Adapter` для типов Geo | [Сериализация](serialization.md) |

## Тесты как примеры

Каждая страница ссылается на конкретные тесты из [`tests/`](../tests/) —
это не выдуманные примеры, а проверенные сценарии с реальными числами:
[`test_polygon.cpp`](../tests/test_polygon.cpp),
[`test_polyline.cpp`](../tests/test_polyline.cpp),
[`test_inflate.cpp`](../tests/test_inflate.cpp),
[`test_clipopen.cpp`](../tests/test_clipopen.cpp),
[`test_normalize.cpp`](../tests/test_normalize.cpp),
[`test_fill.cpp`](../tests/test_fill.cpp),
[`test_cancel.cpp`](../tests/test_cancel.cpp),
[`test_util.cpp`](../tests/test_util.cpp).

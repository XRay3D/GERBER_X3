#pragma once

#include "Enums.h"
#include "xmlserializertypes.h"
#include <QPainterPath>

class QGraphicsItem;
class QPainterPath;

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

#define COMPONENTSONBOARD 1
#define CONNECTIVITY      1
#define CONSTRUCTIVE      1
#define DIALOGSETTINGS    1
#define DISPLAYCONTROL    1
#define GROUPS            1
#define HEADER            1
#define HISPEEDRULES      1
#define LAYERS            1
#define LOCALLIBRARY      1
#define NETLIST           1
#define RULES             1
#define SETTINGS          1
#define TEXTSTYLES        1

namespace TopoR {

namespace Reference_Types {
// базовый класс ссылок.
struct BaseRef {
    // Имя объекта или ссылка на именованный объект.
    Xml::Attr<QString> name;
    operator QString() const { return name; }
};

#define ELEMENT_REF(NAME)                         \
    struct NAME {                                 \
        Xml::Attr<QString> name;                  \
        operator QString() const { return name; } \
    };

// Ссылка на слой.
ELEMENT_REF(LayerRef)
/// \note !Если в дизайне определён только один слой с заданным именем, то тип слоя не указывается.
// struct LayerRef : BaseRef { };
// Тип слоя или ссылка на именованный cлой
/// \note В документации сказано ещё и про возможность установки типа, если имя слоя неуникально, в данный момент это отключено
// TODO:
// Xml::Attribute("type", typeof(type_layer)),

// Ссылка на атрибут.
ELEMENT_REF(AttributeRef)
// Ссылка на тип слоя.
ELEMENT_REF(LayerTypeRef)
// Ссылка на группу слоёв.
ELEMENT_REF(LayerGroupRef)
// Ссылка на тип переходного отверстия.
ELEMENT_REF(ViastackRef)
// Ссылка на стек контактных площадок.
ELEMENT_REF(NetRef)
// Ссылка на группу компонентов.
ELEMENT_REF(CompGroupRef)
// Ссылка на компонент на плате.
ELEMENT_REF(CompInstanceRef)
// Ссылка на группу цепей.
ELEMENT_REF(NetGroupRef)
// Ссылка на волновое сопротивление.
ELEMENT_REF(ImpedanceRef)
// Ссылка на сигнал.
ELEMENT_REF(SignalRef)
// Ссылка на группу сигналов..
ELEMENT_REF(SignalGroupRef)
// Ссылка на дифференциальный сигнал.
ELEMENT_REF(DiffSignalRef)
// Ссылка на стек контактных площадок.
ELEMENT_REF(PadstackRef)
// Ссылка на стиль надписей.
ELEMENT_REF(TextStyleRef)
// Ссылка на схемный компонент.
ELEMENT_REF(ComponentRef)
// Ссылка на посадочное место.
ELEMENT_REF(FootprintRef)

// Ссылка на контакт.
struct PinRef {
    // Имя компонента, используется для ссылки на компонент.
    Xml::Attr<QString> compName;
    // Имя контакта компонента, используется для ссылки.
    Xml::Attr<QString> pinName;
};

// Ссылка на контакт источника сигнала.
struct SourcePinRef /*: PinRef */ {
    // using PinRef::PinRef;
    // Имя компонента, используется для ссылки на компонент.
    Xml::Attr<QString> compName;
    // Имя контакта компонента, используется для ссылки.
    Xml::Attr<QString> pinName;
};

// Ссылка на контакт приёмника сигнала.
struct ReceiverPinRef /*: PinRef */ {
    // using PinRef::PinRef;
    // Имя компонента, используется для ссылки на компонент.
    Xml::Attr<QString> compName;
    // Имя контакта компонента, используется для ссылки.
    Xml::Attr<QString> pinName;
};

// Ссылка на вывод посадочного места.
struct PadRef {
    // Ссылка на имя компонента
    // Имя компонента, используется для ссылки на компонент.
    Xml::Attr<QString> compName;
    // Номер контактной площадки (вывода) посадочного места.
    Xml::Attr<int> padNum;
};

} // namespace Reference_Types
using namespace Reference_Types;

namespace Coordinates {

struct BaseCoordinat {
    BaseCoordinat(double x, double y)
        : x{x}, y{y} { }

    Xml::Attr<double, NoOpt> x, y;
    operator QPointF() const { return {x, y}; }
    QPointF toPoint() const { return *this; }
    template <int I> friend auto get(const BaseCoordinat&);
};

// координаты точки, вершины.
// using Dot = Xml::Named<BaseCoordinat, "Dot">;
// Центр круга (окружности), овала.
// using Center = Xml::Named<BaseCoordinat, "Center">;
// Начальная точка линии, дуги.
// using Start = Xml::Named<BaseCoordinat, "Start">;
// Средняя точка дуги.
// using Middle = Xml::Named<BaseCoordinat, "Middle">;
// Конечная точка линии, дуги.
// using End = Xml::Named<BaseCoordinat, "End">;
// Точка привязки объекта.
// using Org = Xml::Named<BaseCoordinat, "Org">;
// Cмещение точки привязки или объекта по осям x и y.
// using Shift = Xml::Named<BaseCoordinat, "Shift">;
// Вытягивание по осям x и y.
// using Stretch = Xml::Named<BaseCoordinat, "Stretch">;

#define ELEMENT_COORD(NAME)                         \
    struct NAME {                                   \
        Xml::Attr<double, NoOpt> x, y;              \
        operator QPointF() const { return {x, y}; } \
        QPointF toPoint() const { return *this; }   \
    };

// Центр круга (окружности), овала.
ELEMENT_COORD(Center)
// координаты точки, вершины.
ELEMENT_COORD(Dot)
// Конечная точка линии, дуги.
ELEMENT_COORD(End)
// Средняя точка дуги.
ELEMENT_COORD(Middle)
// Точка привязки объекта.
ELEMENT_COORD(Org)
// Cмещение точки привязки или объекта по осям x и y.
ELEMENT_COORD(Shift)
// Начальная точка линии, дуги.
ELEMENT_COORD(Start)
// Вытягивание по осям x и y.
ELEMENT_COORD(Stretch)

} // namespace Coordinates
using namespace Coordinates;

namespace Segments {
// Интерфейс IBaseSegment создан для реализации удобного доступа к одинаковым методам разных объектов
struct IBaseFigure { };
// Описание прямолинейного сегмента контура.
struct SegmentLine /*: IBaseSegment */ {
    // Конечная точка линии, дуги.
    End end;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание дугообразного сегмента контура.
// Дуга, задаётся центром. Обход против часовой стрелки.
struct SegmentArcCCW /*: SegmentLine */ {
    // SegmentLine
    End end;
    // Центр круга (окружности), овала.
    Center center;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание дугообразного сегмента контура.
struct SegmentArcCW /*: SegmentArcCCW */ {
    // Дуга, задаётся центром. Обход по часовой стрелки.
    // SegmentLine
    End end;
    // Центр круга (окружности), овала. SegmentArcCCW
    Center center;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание дугообразного сегмента контура.
struct SegmentArcByAngle /*: SegmentLine */ {
    // Дуга, задаётся углом. Отрицательный угол означает обход по часовой стрелке.
    // SegmentLine
    End end;
    // Задаёт угол в градусах c точностью до тысячных долей.
    Xml::Attr<double> angle;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание дугообразного сегмента контура.
struct SegmentArcByMiddle /*: SegmentLine */ {
    // Дуга, задаётся тремя точками: начало, середина и конец.
    // SegmentLine
    End end;
    // Конечная точка линии, дуги.
    Middle middle;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

} // namespace Segments
using namespace Segments;

namespace Figures {

// Интерфейс BaseFigure создан для реализации удобного доступа к одинаковым методам разных объектов
struct IBaseFigure { };

// Дуга, заданная центром. Обход против часовой стрелки.
struct ArcCCW /*: IBaseFigure */ {
    // Центр круга (окружности), овала.
    Center center;
    // Начальная точка линии, дуги.
    Start start;
    // Конечная точка линии, дуги.
    End end;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Дуга, заданная центром. Обход по часовой стрелке.
struct ArcCW /*: ArcCCW */ {
    // Центр круга (окружности), овала. ArcCCW
    Center center;
    // Начальная точка линии, дуги. ArcCCW
    Start start;
    // Конечная точка линии, дуги. ArcCCW
    End end;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Дуга, заданная углом. Отрицательный угол означает обход по часовой стрелке.
struct ArcByAngle /*: IBaseFigure */ {
    // Задаёт угол в градусах c точностью до тысячных долей.
    Xml::Attr<double> angle;
    // Начальная точка линии, дуги.
    Start start;
    // Конечная точка линии, дуги.
    End end;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Дуга, заданная тремя точками: начало, середина и конец.
struct ArcByMiddle /*: IBaseFigure */ {
    // Начальная точка линии, дуги.
    Start start;
    // Конечная точка линии, дуги.
    Middle middle;
    // Конечная точка линии, дуги.
    End end;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание окружности (незалитого круга).
struct Circle /*: IBaseFigure */ {
    // Диаметр окружности, круга, овала.
    Xml::Attr<double> diameter;
    // Центр круга (окружности), овала.
    Center center;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание линии.
struct Line /*: IBaseFigure */ {
    // Массив координат точек, вершин.
    Xml::Array<Dot> Dots;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

using Segments = Xml::Array<Xml::Variant<SegmentArcByAngle, SegmentArcByMiddle, SegmentArcCCW, SegmentArcCW, SegmentLine>>;

// Описание полилинии.
struct Polyline /*: IBaseFigure */ {
    // Начальная точка линии, дуги.
    Start start;
    // Сегменты.
    Segments segments;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание незалитого контура.
struct Contour /*: Polyline */ {
    // Если конечная точка последнего сегмента не совпадает с начальной точкой контура, контур замыкается линейным сегментом.
    Start start;
    // Сегменты.
    Segments segments;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание незалитого прямоугольника. Указываются верхняя левая и правая нижняя вершины
struct Rect /*: Line */ {
    Xml::Array<Dot> Dots;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание залитого контура.
struct FilledContour /*: Polyline */ {
    // Если конечная точка последнего сегмента не совпадает с начальной точкой контура, контур замыкается линейным сегментом.
    Start start;
    // Сегменты.
    Segments segments;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание круга.// TODO: требует уточнения
struct FilledCircle /*: Circle */ {
    Xml::Attr<double> diameter;
    Center center;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание залитого прямоугольника.
struct FilledRect /*: Rect */ {
    Xml::Array<Dot> Dots;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание многоугольника.
struct Polygon /*: Line */ {
    // Тег поддерживается, но является устаревшим.Следует использовать тег FilledContour.
    Xml::Array<Dot> Dots;
    /*[[deprecated("Следует использовать тег FilledContour")]]*/ void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание дугообразного сегмента проводника (дуга по часовой стрелке).
struct TrackArcCW /*: IBaseFigure */ {
    /// \note Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.
    // Центр круга (окружности), овала.
    Center center;
    // Конечная точка линии, дуги.
    End end;
    // Ссылка на змейку. Строка должна содержать идентификатор описанной змейки Serpent.
    Xml::Attr<QString> serpRef;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание дугообразного сегмента проводника (дуга против часовой стрелки).
struct TrackArc {
    /// \note Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.
    // TrackArcCW
    Center center;
    // TrackArcCW
    End end;
    Xml::Attr<QString> serpRef;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

// Описание прямолинейного сегмента проводника.
struct TrackLine /*: IBaseFigure */ {
    /// \note Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.
    // Конечная точка линии, дуги.
    End end;
    // Ссылка на змейку. Строка должна содержать идентификатор описанной змейки Serpent.
    Xml::Attr<QString> serpRef;
    void drawTo(QPainterPath& path) const;
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); }
};

} // namespace Figures
using namespace Figures;

// Xml::Element empty tag always
namespace Rules_Area {
// Устанавливает область действия правила: все слои.
struct AllLayers { };
// Устанавливает область действия правила: все компоненты.
struct AllComps { };
// Устанавливает область действия правила: все цепи.
struct AllNets { };
// Устанавливает область действия правила: все внутренние слои.
struct AllLayersInner { };
// Устанавливает область действия правила: все внутренние сигнальные слои.
struct AllLayersInnerSignal { };
// Устанавливает область действия правила: все сигнальные слои.
struct AllLayersSignal { };
// Устанавливает область действия правила: все внешние слои.
struct AllLayersOuter { };
// Устанавливает доступные типы переходных отверстий для правила: все типы.
struct AllViastacks { };
// Устанавливает доступные типы переходных отверстий для правила: все сквозные типы.
struct AllViastacksThrough { };
// Устанавливает доступные типы переходных отверстий для правила: все несквозные типы.
struct AllViastacksNotThrough { };
} // namespace Rules_Area
using namespace Rules_Area;

namespace Thermal_Detail_Text_ObjectSignal {
// Описание термобарьера.
struct Thermal {
    // Параметры термобарьера:
    // Число спиц.! В TopoR поддерживается только одно значение – 4.
    Xml::Attr<int> spokeNum{4};
    // Минимальное число спиц.
    Xml::Attr<int> minSpokeNum;
    // Задаёт угол в градусах c точностью до тысячных долей.
    Xml::Attr<double> angle;
    // Ширина спицы.
    Xml::Attr<double> spokeWidth;
    // Зазор между контактной площадкой и областью металлизации.
    Xml::Attr<double> backoff;
};

// Описание детали.
struct Detail {
    // Толщина линии.
    Xml::Attr<double> lineWidth;
    // Ссылка на слой.
    LayerRef layerRef;
    // Описание фигуры.
    Xml::Variant<
        ArcByAngle,
        ArcByMiddle,
        ArcCCW,
        ArcCW,
        Circle,
        FilledCircle,
        FilledContour,
        FilledRect,
        Line,
        Polygon,
        Polyline,
        Rect>
        Figure;
};

// Описание надписи.
struct Text {
    // Параметр надписи: текст надписи.
    Xml::Attr<QString> text;
    // Параметр надписей (ярлыков): способ выравнивания текста.
    Xml::Attr<align> align_;
    // Задаёт угол в градусах c точностью до тысячных долей.
    Xml::Attr<double> angle;
    // Параметр надписей и ярлыков: зеркальность отображения.
    Xml::Attr<Bool> mirror;
    // Ссылка на слой.
    LayerRef layerRef;
    // Ссылка на стиль надписей.
    TextStyleRef textStyleRef;
    // Точка привязки объекта.
    Org org;
    // TODO: конвертировать текстовые стили по ссылке
    // <param name="in_units"></param>
    // <param name="out_units"></param>
};

// Сигналы воздействия правила
struct ObjectSignal {
    // FIXME ???
    Xml::Variant<SignalRef, DiffSignalRef, SignalGroupRef> objectSignal /* NOTE Refs_*/;
};

} // namespace Thermal_Detail_Text_ObjectSignal
using namespace Thermal_Detail_Text_ObjectSignal;

// Различные сервисные функции
struct Ut final {
    // Конвертация единиц измерения

    /// \param value \brief значение
    /// \param inUnits \brief текущие единицы измерения
    /// \param outUnits \brief выходные единицы измерения
    /// \return  Возвращает сконвертированное значение
    static double UnitsConvert(dist inUnits, dist outUnits);
};

} // namespace TopoR
using namespace TopoR;

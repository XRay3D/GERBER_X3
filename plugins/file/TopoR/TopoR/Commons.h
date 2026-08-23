#pragma once
#include "lxml.hpp"
#include <QPainterPath>
#include <meta>
#include <optional>
#include <qtransform.h>
#include <string>
#include <utility>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {

enum ArcDir {
    CW,
    CCW
};

template <XML::IsEnum E>
constexpr auto operator+(E e) noexcept -> std::underlying_type_t<E> { return std::to_underlying(e); }

inline namespace Enumerations { // Все enum в алфавитном порядке
// Параметр надписей (ярлыков): способ выравнивания текста. Значение по умолчанию – CM.
enum class align {
    CM, // по центру
    LT, // по левому верхнему углу
    CT, // по верхнему краю
    RT, // по правому верхнему углу
    LM, // по левому краю
    RM, // по правому краю
    LB, // по левому нижнему углу
    CB, // по нижнему краю
    RB  // по правому нижнему углу
};
// Параметр автоматической трассировки: использование функциональной эквивалентности. Значение по умолчанию – None.
enum class autoEqu {
    None,  // не использовать функциональную эквивалентность
    Pins,  // переназначать выводы компонента
    Gates, // переназначать вентили компонентов (не поддерживается)
    Full   // разрешить все переназначения (не поддерживается)
};
// Настройка автоматической подвижки. Значение по умолчанию – MoveVias.
enum class automove {
    MoveVias,           // двигаются только переходы
    MoveViasWithRefine, // двигаются только переходы; в процессе движения выполняется перекладка проводников
    MoveCompsWithRefine // двигаются компоненты и переходы; в процессе движения выполняется перекладка проводников
};
// Флаг, значение по умолчанию – off.
enum class Bool : bool {
    off,
    on
};
// Параметр области металлизации (полигона) стека: подключение контактных площадок. Значение по умолчанию – Direct.
enum class connectPad {
    Direct, // прямое подключение
    Thermal // подключение с помощью термобарьера
};
// Параметр области металлизации (полигона): подключение площадок переходных отверстий. Значение по умолчанию – Direct.
enum class connectVia {
    Direct, // прямое подключение
    Thermal // подключение с помощью термобарьера
};
// Единицы измерения длины для всего файла. Значение по умолчанию – mm (миллиметр).
enum class dist {
    mm,  // миллиметр
    mkm, // микрометр
    cm,  // сантиметр
    dm,  // дециметр
    m,   // метр
    mil, // мил(тысячная дюйма)
    inch // дюйм
};
// Параметр области металлизации (полигона): тип заливки. Значение по умолчанию – Solid.
enum class fillType {
    Solid,    // сплошная заливка
    Hatched,  // штриховка сеткой
    CRHatched // диагональная штриховка сеткой
};
// Настройка отображения сетки: тип сетки.
enum class gridKind {
    Dots,
    Lines
};
// Тип слоя. Значение по умолчанию – Signal.
enum class layertype {
    Signal,     // сигнальный слой
    Assy,       // сборочный слой (слой очертаний компонентов)
    Paste,      // слой паяльной пасты
    Silk,       // слой шелкографии
    Mask,       // слой маски
    Plane,      // опорный слой
    Mechanical, // механический слой
    Doc,        // документирующий слой
    Dielectric  // диэлектрический слой
};
// Настройка автоматической трассировки: режим трассировки. Значение по умолчанию – Multilayer.
enum class AutorouteMode {
    MultiLayer,       // многослойная трассировка
    SingleLayerTop,   // однослойная трассировка на верхнем слое
    SingleLayerBottom // однослойная трассировка на нижнем слое
};
// Настройка подключения к углам прямоугольных контактных площадок: режим подключения.
enum class PadConnectSettingsMode {
    AutoConnect, // возможность подключения к углам КП определяется автоматически.
    AllPads      // разрешено подключаться к углам всех КП
};
// Параметр области металлизации (полигона): точность аппроксимации контура. Значение по умолчанию – Med.
enum class precision {
    Med, // средняя точность
    Low, // низкая точность
    High // высокая точность
};
// Настройка отображения: единицы измерения. Значение по умолчанию – Metric.
enum class preference {
    Metric,   // метрические (конкретные единицы выбираются в зависимости от параметра)
    mkm,      // микрометр
    mm,       // миллиметр
    cm,       // сантиметр
    dm,       // дециметр
    m,        // метр
    Imperial, // английские (конкретные единицы выбираются в зависимости от параметра)
    mil,      // мил(тысячная дюйма)
    inch      // дюйм
};
// Настройка автоматической перекладки проводников. Значение по умолчанию – ChangeLayer.
enum class refine {
    ChangeLayer,  // разрешён перенос проводников на другой слой.
    NoChangeLayer // без переноса проводников на другой слой.
};
// Тип запрета трассировки. Значение по умолчанию – Wires
enum class role {
    Wires,       // запрет проводников
    Vias,        // запрет переходных отверстий
    WiresАndVias // запрет проводников и переходных отверстий
};
// Настройка фильтра сообщений: режим показа предупреждений. Значение по умолчанию – ShowChecked.
enum class showWarnings {
    ShowChecked, // показывать только отмеченные предупреждения
    ShowAll,     // показывать все предупреждения
    ShowNothing  // ничего не показывать
};
// Сторона объекта.
// ! Значение Both возможно только при описании запретов размещения.
enum class side {
    Top,    // верх
    Bottom, // низ
    Both    // обе стороны
};
// Параметр области металлизации (полигона): состояние. Значение по умолчанию – Unpoured.
enum class state {
    Unpoured, // незалитая
    Poured,   // залитая
    Locked    // залитая и зафиксированная
};
// Единица измерения времени для всего файла. Значение по умолчанию – ps (пикосекунда).
enum class time {
    ps, // пикосекунда
    fs, // фемтосекунда
    ns, // наносекунда
    us  // микросекунда
};
// Тип предопределённого атрибута компонента. Значение по умолчанию - RefDes
enum class type {
    RefDes,  // позиционное обозначение
    PartName // PartName
};
// Параметр стека контактной площадки: подключение к области металлизации (полигону). Значение по умолчанию – NoneConnect.
enum class ConnectToCopper {
    NoneConnect, // тип подключения не задан(используются настройки полигона)
    Direct,      // прямое подключение
    Thermal      // подключение с помощью термобарьера
};
// Тип обработки углов прямоугольной контактной площадки.
enum class Handling {
    None,     // без обработки
    Rounding, // скругление
    Chamfer   // срез
};
// Тип стека контактных площадок. Значение по умолчанию – Through.
enum class TypePadstack {
    Through,     // сквозной
    SMD,         // планарный
    MountingHole // монтажное отверстие
};
// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения. Значение по умолчанию – mm.
enum class units {
    mm, // миллиметр
    mil // мил (тысячная дюйма)
};
// Параметр правил выравниXmlArrayвания задержек: тип значений констант и допусков. Значение по умолчанию: Dist
enum class ValueType {
    Dist, // длина
    Time  // время
};
// Параметр автоматической трассировки: форма проводников.
enum class wireShape {
    Polyline, // Polyline
    Arcs      // Arcs
};
} // namespace Enumerations
inline namespace ReferenceTypes {
// базовый класс ссылок.
struct BaseRef {
    // Имя объекта или ссылка на именованный объект.
    [[= XML::Attr]] std::string name /*ReferenceName*/;
    operator std::string_view() const { return name; }
    operator QString() const { return QString::fromStdString(name); }
    constexpr auto operator<=>(const BaseRef&) const noexcept = default;
};
// Ссылка на атрибут.
struct AttributeRef : public BaseRef { };
// Ссылка на тип слоя.
struct LayerTypeRef {
    // Тип слоя.
    [[= XML::AttrF]] layertype type{};
    operator std::string_view() const { return XML::toString(type); }
};
// Ссылка на группу слоёв.
struct LayerGroupRef : public BaseRef { };
// Ссылка на слой.
// ! Если в дизайне определён только один слой с заданным именем, то тип слоя не указывается.
struct LayerRef : public BaseRef {
    // Тип слоя или ссылка на именованный cлой
    // В документации сказано ещё и про возможность установки типа, если имя слоя неуникально, в данный момент это отключено
    // TODO:
    //   XmlAttribute("type", typeof(type_layer)),
};

// Ссылка на тип переходного отверстия.
struct ViastackRef : public BaseRef { };
// Ссылка на стек контактных площадок.
struct NetRef : public BaseRef { };
// Ссылка на группу компонентов.
struct CompGroupRef : public BaseRef { };
// Ссылка на компонент на плате.
struct CompInstanceRef : public BaseRef { };
// Ссылка на группу цепей.
struct NetGroupRef : public BaseRef { };
// Ссылка на волновое сопротивление.
struct ImpedanceRef : public BaseRef { };
// Ссылка на сигнал.
struct SignalRef : public BaseRef { };
// Ссылка на группу сигналов..
struct SignalGroupRef : public BaseRef { };
// Ссылка на дифференциальный сигнал.
struct DiffSignalRef : public BaseRef { };
// Ссылка на контакт.
struct PinRef {
    // Имя компонента, используется для ссылки на компонент.
    [[= XML::Attr]] std::string compName;
    // Имя контакта компонента, используется для ссылки.
    [[= XML::Attr]] std::string pinName;
};
// Ссылка на контакт источника сигнала.
struct SourcePinRef : public PinRef { };
// Ссылка на контакт приёмника сигнала.
struct ReceiverPinRef : public PinRef { };
// Ссылка на стек контактных площадок.
struct PadstackRef : public BaseRef { };
// Ссылка на стиль надписей.
struct TextStyleRef : public BaseRef { };
// Ссылка на схемный компонент.
struct ComponentRef : public BaseRef { };
// Ссылка на посадочное место.
struct FootprintRef : public BaseRef { };
// Ссылка на вывод посадочного места.
struct PadRef {
    // Ссылка на имя компонента
    [[= XML::Attr]] std::string compName;
    // Номер контактной площадки (вывода) посадочного места.
    [[= XML::Attr]] int padNum{};
};
} // namespace ReferenceTypes
inline namespace Coordinates {
struct Coord {
    [[= XML::AttrF]] double x{};
    [[= XML::AttrF]] double y{};
    QPointF toPoint() const { return {x, y}; }
    /*explicit*/ operator QPointF() const { return toPoint(); }
    explicit operator bool() const { return x && y; }

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
    void Shift(double x, double y);
    void UnitsConvert(dist in_units, dist out_units);
};
// координаты точки, вершины.
struct Dot : public Coord { };
// Центр круга (окружности), овала.
struct Center : public Coord { };
// Начальная точка линии, дуги.
struct Start : public Coord { };
// Средняя точка дуги.
struct Middle : public Coord { };
// Конечная точка линии, дуги.
struct End : public Coord { };
// Точка привязки объекта.
struct Org : public Coord { };
// Cмещение точки привязки или объекта по осям x и y.
struct Shift : public Coord { };
// Вытягивание по осям x и y.
struct Stretch : public Coord { };
} // namespace Coordinates
inline namespace Segments {
struct IBaseSegment {
    virtual void Shift(double x, double y) { };
    virtual void UnitsConvert(dist in_units, dist out_units) { };
};
// Описание прямолинейного сегмента контура.
struct SegmentLine : public IBaseSegment {
    // Конечная точка линии, дуги.

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
    /*[[= XML::Elem]]*/ End End; /*("End")*/
    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Описание дугообразного сегмента контура.
// Дуга, задаётся центром. Обход против часовой стрелки.
struct SegmentArcCCW : public SegmentLine {
    // Центр круга (окружности), овала.
    [[= XML::ElemF]] Center Center; /*("Center")*/

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
    void Shift(double x, double y);
    void UnitsConvert(dist in_units, dist out_units);
};
// Описание дугообразного сегмента контура.
// Дуга, задаётся центром. Обход по часовой стрелки.
struct SegmentArcCW : public SegmentArcCCW {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание дугообразного сегмента контура.
// Дуга, задаётся углом. Отрицательный угол означает обход по часовой стрелке.
struct SegmentArcByAngle : public SegmentLine {
    // Задаёт угол в градусах c точностью до тысячных долей.
    [[= XML::Attr]] double angle{};

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание дугообразного сегмента контура.
// Дуга, задаётся тремя точками: начало, середина и конец.
struct SegmentArcByMiddle : public SegmentLine {
    // Конечная точка линии, дуги.
    /*[[= XML::Elem]]*/ Middle Middle; /*("Middle")*/

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
    void Shift(double x, double y);
    void UnitsConvert(dist in_units, dist out_units);
};
} // namespace Segments
inline namespace Figures {
// Интерфейс BaseFigure создан для реализации удобного доступа к одинаковым методам разных объектов
struct IBaseFigure {
    virtual void UnitsConvert(dist in_units, dist out_units) { };
    virtual void Shift(double x, double y) { };
};
// Дуга, заданная центром. Обход против часовой стрелки.
struct ArcCCW : public IBaseFigure {
    // Центр круга (окружности), овала.
    [[= XML::ElemF]] Center Center;  /*("Center")*/
                                     // Начальная точка линии, дуги.
    /*[[= XML::Elem]]*/ Start Start; /*("Start")*/
                                     // Конечная точка линии, дуги.
    /*[[= XML::Elem]]*/ End End;     /*("End")*/

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Дуга, заданная центром. Обход по часовой стрелке.
struct ArcCW : public ArcCCW {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Дуга, заданная углом. Отрицательный угол означает обход по часовой стрелке.
struct ArcByAngle : public IBaseFigure {
    // Задаёт угол в градусах c точностью до тысячных долей.
    [[= XML::Attr]] double angle{};
    // Начальная точка линии, дуги.
    /*[[= XML::Elem]]*/ Start Start; /*("Start")*/
                                     // Конечная точка линии, дуги.
    /*[[= XML::Elem]]*/ End End;     /*("End")*/

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Дуга, заданная тремя точками: начало, середина и конец.
struct ArcByMiddle : public IBaseFigure {
    // Начальная точка линии, дуги.
    /*[[= XML::Elem]]*/ Start Start;   /*("Start")*/
                                       // Конечная точка линии, дуги.
    /*[[= XML::Elem]]*/ Middle Middle; /*("Middle")*/
                                       // Конечная точка линии, дуги.
    /*[[= XML::Elem]]*/ End End;       /*("End")*/

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Описание окружности (незалитого круга).
struct Circle : public IBaseFigure {
    // Диаметр окружности, круга, овала.
    [[= XML::Attr]] double diameter{};
    // Центр круга (окружности), овала.
    [[= XML::ElemF]] Center Center; /*("Center")*/

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Описание линии.
struct Line : public IBaseFigure {
    // Массив координат точек, вершин.
    [[= XML::Elem]] std::vector<Dot> Dots; /*("Dot")*/
    bool ShouldSerialize_Dots();

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Описание полилинии.
struct Polyline : public IBaseFigure {
    // Начальная точка линии, дуги.
    /*[[= XML::Elem]]*/ Start Start; /*("Start")*/
    // Сегменты.
    // public List<Object> Segments;
    [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ SegmentLine, SegmentArcByAngle, SegmentArcCCW, SegmentArcCW, SegmentArcByMiddle>> Segments;
    bool ShouldSerialize_Segments();

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Описание незалитого контура.
// Если конечная точка последнего сегмента не совпадает с начальной точкой контура, контур замыкается линейным сегментом.
struct Contour : public Polyline {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание незалитого прямоугольника. Указываются верхняя левая и правая нижняя вершины
struct Rect : public Line {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание залитого контура.
// Если конечная точка последнего сегмента не совпадает с начальной точкой контура, контур замыкается линейным сегментом.
struct FilledContour : public Polyline {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
}; // TODO: требует уточнения
// Описание круга.
struct FilledCircle : public Circle {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание залитого прямоугольника.
struct FilledRect : public Rect {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание многоугольника.
// Тег поддерживается, но является устаревшим.Следует использовать тег FilledContour.
struct Polygon : public Line {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание дугообразного сегмента проводника (дуга по часовой стрелке).
// Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.
struct TrackArcCW : public IBaseFigure {
    // Центр круга (окружности), овала.
    [[= XML::ElemF]] Center Center; /*("Center")*/
                                    // Конечная точка линии, дуги.
    /*[[= XML::Elem]]*/ End End;    /*("End")*/
    // Ссылка на змейку. Строка должна содержать идентификатор описанной змейки Serpent.
    [[= XML::Attr]] std::string serpRef;

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
// Описание дугообразного сегмента проводника (дуга против часовой стрелки).
// Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.
struct TrackArc : public TrackArcCW {
    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
};
// Описание прямолинейного сегмента проводника.
// Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.
struct TrackLine : public IBaseFigure {
    // Конечная точка линии, дуги.
    /*[[= XML::Elem]]*/ End End; /*("End")*/
    // Ссылка на змейку. Строка должна содержать идентификатор описанной змейки Serpent.
    [[= XML::Attr]] std::string serpRef;

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;

    void Shift(double x, double y) override;
    void UnitsConvert(dist in_units, dist out_units) override;
};
} // namespace Figures
inline namespace RulesArea {
// Устанавливает область действия правила: все слои.
struct AllLayers {
    //    [[= XML::]] std::string allLayers;Elem /*("AllLayers")*/
};
// Устанавливает область действия правила: все компоненты.
struct AllComps {
    //    [[= XML::]] std::string allComps;Elem /*("AllComps")*/
};
// Устанавливает область действия правила: все цепи.
struct AllNets {
    //    [[= XML::]] std::string allNets;Elem /*("AllNets")*/
};
// Устанавливает область действия правила: все внутренние слои.
struct AllLayersInner {
    //    [[= XML::]] std::string allLayersInner;Elem /*("AllLayersInner")*/
};
// Устанавливает область действия правила: все внутренние сигнальные слои.
struct AllLayersInnerSignal {
    //    [[= XML::]] std::string allLayersInnerSignal;Elem /*("AllLayersInnerSignal")*/
};
// Устанавливает область действия правила: все сигнальные слои.
struct AllLayersSignal {
    //    [[= XML::]] std::string allLayersSignal;Elem /*("AllLayersSignal")*/
};
// Устанавливает область действия правила: все внешние слои.
struct AllLayersOuter {
    //    [[= XML::]] std::string allLayersOuter;Elem /*("AllLayersOuter")*/
};
// Устанавливает доступные типы переходных отверстий для правила: все типы.
struct AllViastacks {
    //    [[= XML::]] std::string allViastacks;Elem /*("AllViastacks")*/
};
// Устанавливает доступные типы переходных отверстий для правила: все сквозные типы.
struct AllViastacksThrough {
    //    [[= XML::]] std::string allViastacksThrough;Elem /*("AllViastacksThrough")*/
};
// Устанавливает доступные типы переходных отверстий для правила: все несквозные типы.
struct AllViastacksNotThrough {
    //    [[= XML::]] std::string allViastacksNotThrough;Elem /*("AllViastacksNotThrough")*/
};
} // namespace RulesArea
inline namespace ThermalDetailTextObjectSignal {
// Описание термобарьера.
struct Thermal {
    // Параметр термобарьера: число спиц.! В TopoR поддерживается только одно значение – 4.
    [[= XML::Attr]] int spokeNum{4};
    // Параметр термобарьера: минимальное число спиц.
    [[= XML::Attr]] int minSpokeNum{};
    // Задаёт угол в градусах c точностью до тысячных долей.
    [[= XML::Attr]] double angle{};
    // Параметр термобарьера: ширина спицы.
    [[= XML::Attr]] double spokeWidth{};
    // Параметр термобарьера: зазор между контактной площадкой и областью металлизации.
    [[= XML::Attr]] double backoff{};
    void UnitsConvert(dist in_units, dist out_units);
};
// Описание детали.
struct Detail {
    // Толщина линии.
    [[= XML::Attr]] double lineWidth{};
    // Ссылка на слой.
    /*[[= XML::Elem]]*/ LayerRef LayerRef; /*("LayerRef")*/
                                           // Описание фигуры.
                                           // public Object figure;
    /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon, Polyline, FilledContour> Figure;

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
    void Shift(double x, double y);
    void UnitsConvert(dist in_units, dist out_units);
};
// Описание надписи.
struct Text {
    // Параметр надписи: текст надписи.
    [[= XML::Attr]] std::string text;
    // Параметр надписей (ярлыков): способ выравнивания текста.
    [[= XML::Attr]] align align{};
    // Задаёт угол в градусах c точностью до тысячных долей.
    [[= XML::Attr]] double angle{};
    // Параметр надписей и ярлыков: зеркальность отображения.
    [[= XML::Attr]] Bool mirror{};
    bool getMirrorSpecified() const;
    // Ссылка на слой.
    /*[[= XML::Elem]]*/ LayerRef LayerRef;         /*("LayerRef")*/
                                                   // Ссылка на стиль надписей.
    /*[[= XML::Elem]]*/ TextStyleRef TextStyleRef; /*("TextStyleRef")*/
                                                   // Точка привязки объекта.
    /*[[= XML::Elem]]*/ Org Org;                   /*("Org")*/

    QPainterPath toPPath() const;
    operator QPainterPath() const { return toPPath(); };
    void drawTo(QPainterPath& path) const;
    void Shift(double x, double y);
    // TODO: конвертировать текстовые стили по ссылке
    void UnitsConvert(dist in_units, dist out_units);
};
// Сигналы воздействия правила
struct ObjectSignal {
    // public Object refs;
    /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ SignalRef, DiffSignalRef, SignalGroupRef> Refs;
};
} // namespace ThermalDetailTextObjectSignal
// Различные сервисные функции
struct Ut final {
    // Конвертация единиц измерения
    static double UnitsConvert(double value, dist in_units, dist out_units);
};
} // namespace TopoR

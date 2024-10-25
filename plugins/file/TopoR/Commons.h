#pragma once

#include "qserializer.h"
#include <QDebug>
// #include <any>
#include <string>
#include <type_traits>
#include <variant>
#include <vector>

template <typename E>
    requires std::is_enum_v<E>
QDebug operator<<(QDebug debug, const E& e) {
    QDebugStateSaver saver(debug);
    debug.nospace() << e;
    return debug;
}

// C# TO C++ CONVERTER NOTE: Forward class declarations:
// namespace TopoR_PCB_Classes {
class Shift;
//}// namespace TopoR_PCB_Classes

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {

//	#region Enumerations //Все enum в алфавитном порядке

/// <summary>
/// Параметр надписей (ярлыков): способ выравнивания текста. Значение по умолчанию – CM.
/// </summary>
enum class align {
    /// <summary>
    /// по центру
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("CM")] CM,
    CM,
    /// <summary>
    /// по левому верхнему углу
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("LT")] LT,
    LT,
    /// <summary>
    /// по верхнему краю
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("CT")] CT,
    CT,
    /// <summary>
    /// по правому верхнему углу
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("RT")] RT,
    RT,
    /// <summary>
    /// по левому краю
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("LM")] LM,
    LM,
    /// <summary>
    /// по правому краю
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("RM")] RM,
    RM,
    /// <summary>
    /// по левому нижнему углу
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("LB")] LB,
    LB,
    /// <summary>
    /// по нижнему краю
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("CB")] CB,
    CB,
    /// <summary>
    /// по правому нижнему углу
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("RB")] RB
    RB
};

Q_DECLARE_METATYPE(align)
Q_DECLARE_FLAGS(aligns, align)

/// <summary>
/// Параметр автоматической трассировки: использование функциональной эквивалентности. Значение по умолчанию – None.
/// </summary>
enum class autoEqu {
    /// <summary>
    /// не использовать функциональную эквивалентность
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("None")] None,
    None,
    /// <summary>
    /// переназначать выводы компонента
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Pins")] Pins,
    Pins,
    /// <summary>
    /// переназначать вентили компонентов (не поддерживается)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Gates")] Gates,
    Gates,
    /// <summary>
    /// разрешить все переназначения (не поддерживается)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Full")] Full
    Full
};

Q_DECLARE_METATYPE(autoEqu)
Q_DECLARE_FLAGS(autoEqus, autoEqu)

/// <summary>
/// Настройка автоматической подвижки. Значение по умолчанию – MoveVias.
/// </summary>
enum class automove {
    /// <summary>
    /// двигаются только переходы
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("MoveVias")] MoveVias,
    MoveVias,
    /// <summary>
    /// двигаются только переходы; в процессе движения выполняется перекладка проводников
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("MoveViasWithRefine")] MoveViasWithRefine,
    MoveViasWithRefine,
    /// <summary>
    /// двигаются компоненты и переходы; в процессе движения выполняется перекладка проводников
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("MoveCompsWithRefine")] MoveCompsWithRefine
    MoveCompsWithRefine
};

Q_DECLARE_METATYPE(automove)
Q_DECLARE_FLAGS(automoves, automove)

/// <summary>
/// Флаг, значение по умолчанию – off.
/// </summary>
enum class Bool {

    // ORIGINAL LINE: [XmlEnum("off")] off,
    off,

    // ORIGINAL LINE: [XmlEnum("on")] on
    on
};

Q_DECLARE_METATYPE(Bool)
Q_DECLARE_FLAGS(Bools, Bool)

/// <summary>
/// Параметр области металлизации (полигона) стека: подключение контактных площадок. Значение по умолчанию – Direct.
/// </summary>
enum class connectPad {
    /// <summary>
    /// прямое подключение
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Direct")] Direct,
    Direct,
    /// <summary>
    /// подключение с помощью термобарьера
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Thermal")] Thermal
    Thermal
};

Q_DECLARE_METATYPE(connectPad)
Q_DECLARE_FLAGS(connectPads, connectPad)

/// <summary>
/// Параметр области металлизации (полигона): подключение площадок переходных отверстий. Значение по умолчанию – Direct.
/// </summary>
enum class connectVia {
    /// <summary>
    /// прямое подключение
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Direct")] Direct,
    Direct,
    /// <summary>
    /// подключение с помощью термобарьера
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Thermal")] Thermal
    Thermal
};

Q_DECLARE_METATYPE(connectVia)
Q_DECLARE_FLAGS(connectVias, connectVia)

/// <summary>
/// Единицы измерения длины для всего файла. Значение по умолчанию – mm (миллиметр).
/// </summary>
enum class dist_ {
    /// <summary>
    /// миллиметр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mm")] mm,
    mm,
    /// <summary>
    /// микрометр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mkm")] mkm,
    mkm,
    /// <summary>
    /// сантиметр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("cm")] cm,
    cm,
    /// <summary>
    /// дециметр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("dm")] dm,
    dm,
    /// <summary>
    /// метр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("m")] m,
    m,
    /// <summary>
    /// мил(тысячная дюйма)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mil")] mil,
    mil,
    /// <summary>
    /// дюйм
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("inch")] inch
    inch
};

Q_DECLARE_METATYPE(dist_)
Q_DECLARE_FLAGS(dists, dist_)

/// <summary>
/// Параметр области металлизации (полигона): тип заливки. Значение по умолчанию – Solid.
/// </summary>
enum class fillType {
    /// <summary>
    /// сплошная заливка
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Solid")] Solid,
    Solid,
    /// <summary>
    /// штриховка сеткой
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Hatched")] Hatched,
    Hatched,
    /// <summary>
    /// диагональная штриховка сеткой
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("CRHatched")] CRHatched
    CRHatched
};

Q_DECLARE_METATYPE(fillType)
Q_DECLARE_FLAGS(fillTypes, fillType)

/// <summary>
/// Настройка отображения сетки: тип сетки.
/// </summary>
enum class gridKind {

    // ORIGINAL LINE: [XmlEnum("Dots")] Dots,
    Dots,

    // ORIGINAL LINE: [XmlEnum("Lines")] Lines
    Lines
};

Q_DECLARE_METATYPE(gridKind)
Q_DECLARE_FLAGS(gridKinds, gridKind)

/// <summary>
/// Тип слоя. Значение по умолчанию – Signal.
/// </summary>
enum class layer_type {
    /// <summary>
    /// сигнальный слой
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Signal")] Signal,
    Signal,
    /// <summary>
    /// сборочный слой (слой очертаний компонентов)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Assy")] Assy,
    Assy,
    /// <summary>
    /// слой паяльной пасты
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Paste")] Paste,
    Paste,
    /// <summary>
    /// слой шелкографии
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Silk")] Silk,
    Silk,
    /// <summary>
    /// слой маски
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Mask")] Mask,
    Mask,
    /// <summary>
    /// опорный слой
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Plane")] Plane,
    Plane,
    /// <summary>
    /// механический слой
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Mechanical")] Mechanical,
    Mechanical,
    /// <summary>
    /// документирующий слой
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Doc")] Doc,
    Doc,
    /// <summary>
    /// диэлектрический слой
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Dielectric")] Dielectric
    Dielectric
};

Q_DECLARE_METATYPE(layer_type)
Q_DECLARE_FLAGS(layer_types, layer_type)

/// <summary>
/// Настройка автоматической трассировки: режим трассировки. Значение по умолчанию – Multilayer.
/// </summary>
enum class mode_Autoroute {
    /// <summary>
    /// многослойная трассировка
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Multilayer")] Multilayer,
    Multilayer,
    /// <summary>
    /// однослойная трассировка на верхнем слое
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("SinglelayerTop")] SinglelayerTop,
    SinglelayerTop,
    /// <summary>
    /// однослойная трассировка на нижнем слое
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("SinglelayerBottom")] SinglelayerBottom
    SinglelayerBottom
};

Q_DECLARE_METATYPE(mode_Autoroute)
Q_DECLARE_FLAGS(mode_Autoroutes, mode_Autoroute)

/// <summary>
/// Настройка подключения к углам прямоугольных контактных площадок: режим подключения.
/// </summary>
enum class mode_PadConnectSettings {
    /// <summary>
    /// возможность подключения к углам КП определяется автоматически.
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("AutoConnect")] AutoConnect,
    AutoConnect,
    /// <summary>
    /// разрешено подключаться к углам всех КП
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("AllPads")] AllPads
    AllPads
};

Q_DECLARE_METATYPE(mode_PadConnectSettings)
Q_DECLARE_FLAGS(mode_PadConnectSettingss, mode_PadConnectSettings)

/// <summary>
/// Параметр области металлизации (полигона): точность аппроксимации контура. Значение по умолчанию – Med.
/// </summary>
enum class precision {
    /// <summary>
    /// средняя точность
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Med")] Med,
    Med,
    /// <summary>
    /// низкая точность
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Low")] Low,
    Low,
    /// <summary>
    /// высокая точность
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("High")] High
    High
};

Q_DECLARE_METATYPE(precision)
Q_DECLARE_FLAGS(precisions, precision)

/// <summary>
/// Настройка отображения: единицы измерения. Значение по умолчанию – Metric.
/// </summary>
enum class preference {
    /// <summary>
    /// метрические (конкретные единицы выбираются в зависимости от параметра)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Metric")] Metric,
    Metric,
    /// <summary>
    /// микрометр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mkm")] mkm,
    mkm,
    /// <summary>
    /// миллиметр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mm")] mm,
    mm,
    /// <summary>
    /// сантиметр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("cm")] cm,
    cm,
    /// <summary>
    /// дециметр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("dm")] dm,
    dm,
    /// <summary>
    /// метр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("m")] m,
    m,
    /// <summary>
    /// английские (конкретные единицы выбираются в зависимости от параметра)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Imperial")] Imperial,
    Imperial,
    /// <summary>
    /// мил(тысячная дюйма)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mil")] mil,
    mil,
    /// <summary>
    /// дюйм
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("inch")] inch
    inch
};

Q_DECLARE_METATYPE(preference)
Q_DECLARE_FLAGS(preferences, preference)

/// <summary>
/// Настройка автоматической перекладки проводников. Значение по умолчанию – ChangeLayer.
/// </summary>
enum class refine {
    /// <summary>
    /// разрешён перенос проводников на другой слой.
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("ChangeLayer")] ChangeLayer,
    ChangeLayer,
    /// <summary>
    /// без переноса проводников на другой слой.
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("NoChangeLayer")] NoChangeLayer
    NoChangeLayer
};

Q_DECLARE_METATYPE(refine)
Q_DECLARE_FLAGS(refines, refine)

/// <summary>
/// Тип запрета трассировки. Значение по умолчанию – Wires
/// </summary>
enum class role {
    /// <summary>
    /// запрет проводников
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Wires")] Wires,
    Wires,
    /// <summary>
    /// запрет переходных отверстий
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Vias")] Vias,
    Vias,
    /// <summary>
    /// запрет проводников и переходных отверстий
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Wires and Vias")] WiresАndVias
    // WiresАndVias, FIXME
};

Q_DECLARE_METATYPE(role)
Q_DECLARE_FLAGS(roles, role)

/// <summary>
/// Настройка фильтра сообщений: режим показа предупреждений. Значение по умолчанию – ShowChecked.
/// </summary>
enum class showWarnings {
    /// <summary>
    /// показывать только отмеченные предупреждения
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("ShowChecked")] ShowChecked,
    ShowChecked,
    /// <summary>
    /// показывать все предупреждения
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("ShowAll")] ShowAll,
    ShowAll,
    /// <summary>
    /// ничего не показывать
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("ShowNothing")] ShowNothing
    ShowNothing
};

Q_DECLARE_METATYPE(showWarnings)
Q_DECLARE_FLAGS(showWarningss, showWarnings)

/// <summary>
/// Сторона объекта.
/// </summary>
/// <remarks>! Значение Both возможно только при описании запретов размещения.</remarks>
enum class side {
    /// <summary>
    /// верх
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Top")] Top,
    Top,
    /// <summary>
    /// низ
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Bottom")] Bottom,
    Bottom,
    /// <summary>
    /// обе стороны
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Both")] Both
    Both
};

Q_DECLARE_METATYPE(side)
Q_DECLARE_FLAGS(sides, side)

/// <summary>
/// Параметр области металлизации (полигона): состояние. Значение по умолчанию – Unpoured.
/// </summary>
enum class state {
    /// <summary>
    /// незалитая
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Unpoured")] Unpoured,
    Unpoured,
    /// <summary>
    /// залитая
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Poured")] Poured,
    Poured,
    /// <summary>
    /// залитая и зафиксированная
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Locked")] Locked
    Locked
};

Q_DECLARE_METATYPE(state)
Q_DECLARE_FLAGS(states, state)

/// <summary>
/// Единица измерения времени для всего файла. Значение по умолчанию – ps (пикосекунда).
/// </summary>
enum class time_ {
    /// <summary>
    /// пикосекунда
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("ps")] ps,
    ps,
    /// <summary>
    /// фемтосекунда
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("fs")] fs,
    fs,
    /// <summary>
    /// наносекунда
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("ns")] ns,
    ns,
    /// <summary>
    /// микросекунда
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("us")] us
    us
};

Q_DECLARE_METATYPE(enum time_)
Q_DECLARE_FLAGS(times, enum time_)

/// <summary>
/// Тип предопределённого атрибута компонента. Значение по умолчанию - RefDes
/// </summary>
enum class type {
    /// <summary>
    /// позиционное обозначение
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("RefDes")] RefDes,
    RefDes,
    /// <summary>
    /// PartName
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("PartName")] PartName
    PartName
};

Q_DECLARE_METATYPE(type)
Q_DECLARE_FLAGS(types, type)

/// <summary>
/// Параметр стека контактной площадки: подключение к области металлизации (полигону). Значение по умолчанию – NoneConnect.
/// </summary>
enum class type_connectToCopper {
    /// <summary>
    /// тип подключения не задан(используются настройки полигона)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("NoneConnect")] NoneConnect,
    NoneConnect,
    /// <summary>
    /// прямое подключение
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Direct")] Direct,
    Direct,
    /// <summary>
    /// подключение с помощью термобарьера
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Thermal")] Thermal
    Thermal
};

Q_DECLARE_METATYPE(type_connectToCopper)
Q_DECLARE_FLAGS(type_connectToCoppers, type_connectToCopper)

/// <summary>
/// Тип обработки углов прямоугольной контактной площадки.
/// </summary>
enum class type_handling {
    /// <summary>
    /// без обработки
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("None")] None,
    None,
    /// <summary>
    /// скругление
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Rounding")] Rounding,
    Rounding,
    /// <summary>
    /// срез
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Chamfer")] Chamfer
    Chamfer
};

Q_DECLARE_METATYPE(type_handling)
Q_DECLARE_FLAGS(type_handlings, type_handling)

/// <summary>
/// Тип стека контактных площадок. Значение по умолчанию – Through.
/// </summary>
enum class type_padstack {
    /// <summary>
    /// сквозной
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Through")] Through,
    Through,
    /// <summary>
    /// планарный
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("SMD")] SMD,
    SMD,
    /// <summary>
    /// монтажное отверстие
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("MountHole")] MountHole
    MountHole
};

Q_DECLARE_METATYPE(type_padstack)
Q_DECLARE_FLAGS(type_padstacks, type_padstack)

/// <summary>
/// Настройка вывода файлов Gerber, DXF, Drill: единицы измерения. Значение по умолчанию – mm.
/// </summary>
enum class units {
    /// <summary>
    /// миллиметр
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mm")] mm,
    mm,
    /// <summary>
    /// мил (тысячная дюйма)
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("mil")] mil
    mil
};

Q_DECLARE_METATYPE(units)
Q_DECLARE_FLAGS(unitss, units)

/// <summary>
/// Параметр правил выравнивания задержек: тип значений констант и допусков. Значение по умолчанию: Dist
/// </summary>
enum class valueType {
    /// <summary>
    /// длина
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Dist")] Dist,
    Dist,
    /// <summary>
    /// время
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Time")] Time
    Time
};

Q_DECLARE_METATYPE(valueType)
Q_DECLARE_FLAGS(valueTypes, valueType)

/// <summary>
/// Параметр автоматической трассировки: форма проводников.
/// </summary>
enum class wireShape {
    /// <summary>
    /// Polyline
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Polyline")] Polyline,
    Polyline,
    /// <summary>
    /// Arcs
    /// </summary>

    // ORIGINAL LINE: [XmlEnum("Arcs")] Arcs
    Arcs
};

Q_DECLARE_METATYPE(wireShape)
Q_DECLARE_FLAGS(wireShapes, wireShape)

//	#endregion Enumerations

//	#region Reference Types
/// <summary>
/// базовый класс ссылок.
/// </summary>
class BaseRef {
    /// <summary>
    /// Имя объекта или ссылка на именованный объект.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("name")] public string _ReferenceName;
    QString _ReferenceName;
};

/// <summary>
/// Ссылка на атрибут.
/// </summary>
class AttributeRef : public BaseRef {
};

/// <summary>
/// Ссылка на тип слоя.
/// </summary>
class LayerTypeRef {
    /// <summary>
    /// Тип слоя.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("type")] public layer_type _type;
    layer_type _type = static_cast<layer_type>(0);
};

/// <summary>
/// Ссылка на группу слоёв.
/// </summary>
class LayerGroupRef : public BaseRef {
};

/// <summary>
/// Ссылка на слой.
/// </summary>
/// <remarks>! Если в дизайне определён только один слой с заданным именем, то тип слоя не указывается.</remarks>
class LayerRef : public BaseRef {
    /// <summary>
    /// Тип слоя или ссылка на именованный cлой
    /// </summary>
    /// <remarks>В документации сказано ещё и про возможность установки типа, если имя слоя неуникально, в данный момент это отключено</remarks>
    // TODO:
    //   XmlAttribute("type", typeof(type_layer)),
};

/// <summary>
/// Ссылка на тип переходного отверстия.
/// </summary>
class ViastackRef : public BaseRef {
};

/// <summary>
/// Ссылка на стек контактных площадок.
/// </summary>
class NetRef : public BaseRef {
};

/// <summary>
/// Ссылка на группу компонентов.
/// </summary>
class CompGroupRef : public BaseRef {
};

/// <summary>
/// Ссылка на компонент на плате.
/// </summary>
class CompInstanceRef : public BaseRef {
};

/// <summary>
/// Ссылка на группу цепей.
/// </summary>
class NetGroupRef : public BaseRef {
};

/// <summary>
/// Ссылка на волновое сопротивление.
/// </summary>
class ImpedanceRef : public BaseRef {
};

/// <summary>
/// Ссылка на сигнал.
/// </summary>
class SignalRef : public BaseRef {
};

/// <summary>
/// Ссылка на группу сигналов..
/// </summary>
class SignalGroupRef : public BaseRef {
};

/// <summary>
/// Ссылка на дифференциальный сигнал.
/// </summary>
class DiffSignalRef : public BaseRef {
};

/// <summary>
/// Ссылка на контакт.
/// </summary>
class PinRef {
    /// <summary>
    /// Имя компонента, используется для ссылки на компонент.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("compName")] public string _compName;
    QString _compName;

    /// <summary>
    /// Имя контакта компонента, используется для ссылки.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("pinName")] public string _pinName;
    QString _pinName;
};

/// <summary>
/// Ссылка на контакт источника сигнала.
/// </summary>
class SourcePinRef : public PinRef {
};

/// <summary>
/// Ссылка на контакт приёмника сигнала.
/// </summary>
class ReceiverPinRef : public PinRef {
};

/// <summary>
/// Ссылка на стек контактных площадок.
/// </summary>
class PadstackRef : public BaseRef {
};

/// <summary>
/// Ссылка на стиль надписей.
/// </summary>
class TextStyleRef : public BaseRef {
};

/// <summary>
/// Ссылка на схемный компонент.
/// </summary>
class ComponentRef : public BaseRef {
};

/// <summary>
/// Ссылка на посадочное место.
/// </summary>
class FootprintRef : public BaseRef {
};

/// <summary>
/// Ссылка на вывод посадочного места.
/// </summary>
class PadRef {
    /// <summary>
    /// Ссылка на имя компонента
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("compName")] public string _compName;
    QString _compName;

    /// <summary>
    /// Номер контактной площадки (вывода) посадочного места.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("padNum", DataType = "int")] public int _padNum;
    int _padNum = 0;
};
//	#endregion Reference Type

//	#region Coordinates

class base_coordinat {
public:
    // ORIGINAL LINE: [XmlAttribute("x", DataType = "float")] public float _x;
    float _x = 0.0F;

    // ORIGINAL LINE: [XmlAttribute("y", DataType = "float")] public float _y;
    float _y = 0.0F;

    void Shift(float x, float y);

    void UnitsConvert(dist_ in_units, dist_ out_units);
};

/// <summary>
/// координаты точки, вершины.
/// </summary>
class Dot : public base_coordinat {
};

/// <summary>
/// Центр круга (окружности), овала.
/// </summary>
class Center : public base_coordinat {
};

/// <summary>
/// Начальная точка линии, дуги.
/// </summary>
class Start : public base_coordinat {
};

/// <summary>
/// Средняя точка дуги.
/// </summary>
class Middle : public base_coordinat {
};

/// <summary>
/// Конечная точка линии, дуги.
/// </summary>
class End : public base_coordinat {
};

/// <summary>
/// Точка привязки объекта.
/// </summary>
class Org : public base_coordinat {
};

/// <summary>
/// Cмещение точки привязки или объекта по осям x и y.
/// </summary>
class Shift : public base_coordinat {
};

/// <summary>
/// Вытягивание по осям x и y.
/// </summary>
class Stretch : public base_coordinat {
};

//	#endregion Coordinates

//	#region Segments
class IBaseSegment {
public:
    virtual void Shift(float x, float y) = 0;
    virtual void UnitsConvert(dist_ in_units, dist_ out_units) = 0;
};

/// <summary>
/// Описание прямолинейного сегмента контура.
/// </summary>
class SegmentLine : public IBaseSegment {
    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("End")] public End _End;
    End* _End;

    virtual ~SegmentLine() {
        delete _End;
    }

    void Shift(float x, float y) override;

    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Описание дугообразного сегмента контура.
/// Дуга, задаётся центром. Обход против часовой стрелки.
/// </summary>
class SegmentArcCCW : public SegmentLine {
    /// <summary>
    /// Центр круга (окружности), овала.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("Center")] public Center _Center;
    Center* _Center;

    virtual ~SegmentArcCCW() {
        delete _Center;
    }

    void Shift(float x, float y);

    void UnitsConvert(dist_ in_units, dist_ out_units);
};

/// <summary>
/// Описание дугообразного сегмента контура.
/// Дуга, задаётся центром. Обход по часовой стрелки.
/// </summary>
class SegmentArcCW : public SegmentArcCCW {
};

/// <summary>
/// Описание дугообразного сегмента контура.
/// Дуга, задаётся углом. Отрицательный угол означает обход по часовой стрелке.
/// </summary>
class SegmentArcByAngle : public SegmentLine {
    /// <summary>
    /// Задаёт угол в градусах c точностью до тысячных долей.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
    float _angle = 0.0F;
};

/// <summary>
/// Описание дугообразного сегмента контура.
/// Дуга, задаётся тремя точками: начало, середина и конец.
/// </summary>
class SegmentArcByMiddle : public SegmentLine {
    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("Middle")] public Middle _Middle;
    Middle* _Middle;

    virtual ~SegmentArcByMiddle() {
        delete _Middle;
    }

    void Shift(float x, float y);

    void UnitsConvert(dist_ in_units, dist_ out_units);
};
//	#endregion Segments

//	#region Figures
/// <summary>
/// Интерфейс BaseFigure создан для реализации удобного доступа к одинаковым методам разных объектов
/// </summary>
class IBaseFigure {
public:
    virtual void UnitsConvert(dist_ in_units, dist_ out_units) = 0;
    virtual void Shift(float x, float y) = 0;
};

/// <summary>
/// Дуга, заданная центром. Обход против часовой стрелки.
/// </summary>
class ArcCCW : public IBaseFigure {
    /// <summary>
    /// Центр круга (окружности), овала.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("Center")] public Center _Center;
    Center* _Center;

    /// <summary>
    /// Начальная точка линии, дуги.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Start")] public Start _Start;
    Start* _Start;

    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("End")] public End _End;
    End* _End;

    virtual ~ArcCCW() {
        delete _Center;
        delete _Start;
        delete _End;
    }

    void Shift(float x, float y) override;

    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Дуга, заданная центром. Обход по часовой стрелке.
/// </summary>
class ArcCW : public ArcCCW {
};

/// <summary>
/// Дуга, заданная углом. Отрицательный угол означает обход по часовой стрелке.
/// </summary>
class ArcByAngle : public IBaseFigure {
    /// <summary>
    /// Задаёт угол в градусах c точностью до тысячных долей.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
    float _angle = 0.0F;

    /// <summary>
    /// Начальная точка линии, дуги.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Start")] public Start _Start;
    Start* _Start;

    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("End")] public End _End;
    End* _End;

    virtual ~ArcByAngle() {
        delete _Start;
        delete _End;
    }

    void Shift(float x, float y) override;

    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Дуга, заданная тремя точками: начало, середина и конец.
/// </summary>
class ArcByMiddle : public IBaseFigure {
    /// <summary>
    /// Начальная точка линии, дуги.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("Start")] public Start _Start;
    Start* _Start;

    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Middle")] public Middle _Middle;
    Middle* _Middle;

    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("End")] public End _End;
    End* _End;

    virtual ~ArcByMiddle() {
        delete _Start;
        delete _Middle;
        delete _End;
    }

    void Shift(float x, float y) override;

    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Описание окружности (незалитого круга).
/// </summary>
class Circle : public IBaseFigure {
    /// <summary>
    /// Диаметр окружности, круга, овала.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("diameter", DataType = "float")] public float _diameter;
    float _diameter = 0.0F;

    /// <summary>
    /// Центр круга (окружности), овала.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Center")] public Center _Center;
    Center* _Center;

    virtual ~Circle() {
        delete _Center;
    }

    void Shift(float x, float y) override;

    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Описание линии.
/// </summary>
class Line : public IBaseFigure {
    /// <summary>
    /// Массив координат точек, вершин.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("Dot")] public List<Dot> _Dots;
    std::vector<Dot*> _Dots;
    bool ShouldSerialize_Dots();

    void Shift(float x, float y) override;

    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Описание полилинии.
/// </summary>
class Polyline : public IBaseFigure {
    /// <summary>
    /// Начальная точка линии, дуги.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("Start")] public Start _Start;
    Start* _Start;

    /// <summary>
    /// Сегменты.
    /// </summary>
    // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

    // ORIGINAL LINE: [XmlElement("SegmentLine", typeof(SegmentLine)), XmlElement("SegmentArcByAngle", typeof(SegmentArcByAngle)), XmlElement("SegmentArcCCW", typeof(SegmentArcCCW)), XmlElement("SegmentArcCW", typeof(SegmentArcCW)), XmlElement("SegmentArcByMiddle", typeof(SegmentArcByMiddle))] public List<Object> _Segments;

    std::vector<std::variant<SegmentLine, SegmentArcByAngle, SegmentArcCCW, SegmentArcCW, SegmentArcByMiddle>> _Segments;
    virtual ~Polyline() {
        delete _Start;
    }

    bool ShouldSerialize_Segments();
    void Shift(float x, float y) override;
    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Описание незалитого контура.
/// Если конечная точка последнего сегмента не совпадает с начальной точкой контура, контур замыкается линейным сегментом.
/// </summary>
class Contour : public Polyline {
};

/// <summary>
/// Описание незалитого прямоугольника. Указываются верхняя левая и правая нижняя вершины
/// </summary>
class Rect : public Line {
};

/// <summary>
/// Описание залитого контура.
/// Если конечная точка последнего сегмента не совпадает с начальной точкой контура, контур замыкается линейным сегментом.
/// </summary>
class FilledContour : public Polyline {
}; /// TODO: требует уточнения

/// <summary>
/// Описание круга.
/// </summary>
class FilledCircle : public Circle {
};

/// <summary>
/// Описание залитого прямоугольника.
/// </summary>
class FilledRect : public Rect {
};

/// <summary>
/// Описание многоугольника.
/// Тег поддерживается, но является устаревшим.Следует использовать тег FilledContour.
/// </summary>
class Polygon : public Line {
};

/// <summary>
/// Описание дугообразного сегмента проводника (дуга по часовой стрелке).
/// </summary>
/// <remarks>Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.</remarks>
class TrackArcCW : public IBaseFigure {
    /// <summary>
    /// Центр круга (окружности), овала.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("Center")] public Center _Center;
    Center* _Center;

    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("End")] public End _End;
    End* _End;

    /// <summary>
    /// Ссылка на змейку. Строка должна содержать идентификатор описанной змейки Serpent.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("serpRef")] public string _serpRef;
    QString _serpRef;

    virtual ~TrackArcCW() {
        delete _Center;
        delete _End;
    }

    void Shift(float x, float y) override;
    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

/// <summary>
/// Описание дугообразного сегмента проводника (дуга против часовой стрелки).
/// </summary>
/// <remarks>Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.</remarks>
class TrackArc : public TrackArcCW {
};

/// <summary>
/// Описание прямолинейного сегмента проводника.
/// </summary>
/// <remarks>Начальная точка сегмента определяется по предыдущему сегменту или по тегу Start, заданному в SubWire. ! Если сегмент принадлежит змейке, указывается ссылка на змейку serpRef.</remarks>
class TrackLine : public IBaseFigure {
    /// <summary>
    /// Конечная точка линии, дуги.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlElement("End")] public End _End;
    End* _End;

    /// <summary>
    /// Ссылка на змейку. Строка должна содержать идентификатор описанной змейки Serpent.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("serpRef")] public string _serpRef;
    QString _serpRef;

    virtual ~TrackLine() {
        delete _End;
    }

    void Shift(float x, float y) override;

    void UnitsConvert(dist_ in_units, dist_ out_units) override;
};

//	#endregion Figures

//	#region Rules area
/// <summary>
/// Устанавливает область действия правила: все слои.
/// </summary>
class AllLayers {
public:
    // ORIGINAL LINE: [XmlElement("AllLayers")] public string _AllLayers;
    QString _AllLayers;
};

/// <summary>
/// Устанавливает область действия правила: все компоненты.
/// </summary>
class AllComps {
public:
    // ORIGINAL LINE: [XmlElement("AllComps")] public string _AllComps;
    QString _AllComps;
};

/// <summary>
/// Устанавливает область действия правила: все цепи.
/// </summary>
class AllNets {
public:
    // ORIGINAL LINE: [XmlElement("AllNets")] public string _AllNets;
    QString _AllNets;
};
/// <summary>
/// Устанавливает область действия правила: все внутренние слои.
/// </summary>
class AllLayersInner {
public:
    // ORIGINAL LINE: [XmlElement("AllLayersInner")] public string _AllLayersInner;
    QString _AllLayersInner;
};

/// <summary>
/// Устанавливает область действия правила: все внутренние сигнальные слои.
/// </summary>
class AllLayersInnerSignal {
public:
    // ORIGINAL LINE: [XmlElement("AllLayersInnerSignal")] public string _AllLayersInnerSignal;
    QString _AllLayersInnerSignal;
};

/// <summary>
/// Устанавливает область действия правила: все сигнальные слои.
/// </summary>
class AllLayersSignal {
public:
    // ORIGINAL LINE: [XmlElement("AllLayersSignal")] public string _AllLayersSignal;
    QString _AllLayersSignal;
};

/// <summary>
/// Устанавливает область действия правила: все внешние слои.
/// </summary>
class AllLayersOuter {
public:
    // ORIGINAL LINE: [XmlElement("AllLayersOuter")] public string _AllLayersOuter;
    QString _AllLayersOuter;
};

/// <summary>
/// Устанавливает доступные типы переходных отверстий для правила: все типы.
/// </summary>
class AllViastacks {
public:
    // ORIGINAL LINE: [XmlElement("AllViastacks")] public string _AllViastacks;
    QString _AllViastacks;
};

/// <summary>
/// Устанавливает доступные типы переходных отверстий для правила: все сквозные типы.
/// </summary>
class AllViastacksThrough {
public:
    // ORIGINAL LINE: [XmlElement("AllViastacksThrough")] public string _AllViastacksThrough;
    QString _AllViastacksThrough;
};

/// <summary>
/// Устанавливает доступные типы переходных отверстий для правила: все несквозные типы.
/// </summary>
class AllViastacksNotThrough {
public:
    // ORIGINAL LINE: [XmlElement("AllViastacksNotThrough")] public string _AllViastacksNotThrough;
    QString _AllViastacksNotThrough;
};
//	#endregion Rules area

//	#region Thermal Detail Text ObjectSignal

/// <summary>
/// Описание термобарьера.
/// </summary>
class Thermal {
    /// <summary>
    /// Параметр термобарьера: число спиц.! В TopoR поддерживается только одно значение – 4.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("spokeNum", DataType = "int")] public int _spokeNum;
    int _spokeNum = 0;

    /// <summary>
    /// Параметр термобарьера: минимальное число спиц.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("minSpokeNum", DataType = "int")] public int _minSpokeNum;
    int _minSpokeNum = 0;

    /// <summary>
    /// Задаёт угол в градусах c точностью до тысячных долей.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
    float _angle = 0.0F;

    /// <summary>
    /// Параметр термобарьера: ширина спицы.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("spokeWidth", DataType = "float")] public float _spokeWidth;
    float _spokeWidth = 0.0F;

    /// <summary>
    /// Параметр термобарьера: зазор между контактной площадкой и областью металлизации.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("backoff", DataType = "float")] public float _backoff;
    float _backoff = 0.0F;

    void UnitsConvert(dist_ in_units, dist_ out_units);
};

/// <summary>
/// Описание детали.
/// </summary>
class Detail {
    /// <summary>
    /// Толщина линии.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("lineWidth", DataType = "float")] public float _lineWidth;
    float _lineWidth = 0.0F;

    /// <summary>
    /// Ссылка на слой.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
    LayerRef* _LayerRef;

    /// <summary>
    /// Описание фигуры.
    /// </summary>
    // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

    // ORIGINAL LINE: [XmlElement("ArcCCW", typeof(ArcCCW)), XmlElement("ArcCW", typeof(ArcCW)), XmlElement("ArcByAngle", typeof(ArcByAngle)), XmlElement("ArcByMiddle", typeof(ArcByMiddle)), XmlElement("Line", typeof(Line)), XmlElement("Circle", typeof(Circle)), XmlElement("Rect", typeof(Rect)), XmlElement("FilledCircle", typeof(FilledCircle)), XmlElement("FilledRect", typeof(FilledRect)), XmlElement("Polygon", typeof(Polygon)), XmlElement("Polyline", typeof(Polyline)), XmlElement("FilledContour", typeof(FilledContour))] public Object _Figure;

    std::variant<
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

        /*std::any*/ _Figure;

    virtual ~Detail() {
        delete _LayerRef;
    }

    void Shift(float x, float y);

    void UnitsConvert(dist_ in_units, dist_ out_units);
};

/// <summary>
/// Описание надписи.
/// </summary>
class Text {
    /// <summary>
    /// Параметр надписи: текст надписи.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("text")] public string _text;
    QString _text;

    /// <summary>
    /// Параметр надписей (ярлыков): способ выравнивания текста.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("align")] public align _align;
    align _align = static_cast<align>(0);

    /// <summary>
    /// Задаёт угол в градусах c точностью до тысячных долей.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
    float _angle = 0.0F;

    /// <summary>
    /// Параметр надписей и ярлыков: зеркальность отображения.
    /// </summary>

    // ORIGINAL LINE: [XmlAttribute("mirror")] public Bool _mirror;
    Bool _mirror = static_cast<Bool>(0);
    virtual ~Text() {
        delete _LayerRef;
        delete _TextStyleRef;
        delete _Org;
    }

    // ORIGINAL LINE: [XmlIgnore] public bool _mirrorSpecified
    bool getMirrorSpecified() const;

    /// <summary>
    /// Ссылка на слой.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
    LayerRef* _LayerRef;

    /// <summary>
    /// Ссылка на стиль надписей.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("TextStyleRef")] public TextStyleRef _TextStyleRef;
    TextStyleRef* _TextStyleRef;

    /// <summary>
    /// Точка привязки объекта.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
    Org* _Org;

    void Shift(float x, float y);
    /// <summary>
    /// TODO: конвертировать текстовые стили по ссылке
    /// </summary>
    /// <param name="in_units"></param>
    /// <param name="out_units"></param>
    void UnitsConvert(dist_ in_units, dist_ out_units);
};

/// <summary>
/// Сигналы воздействия правила
/// </summary>
class ObjectSignal {
public:
    // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

    // ORIGINAL LINE: [XmlElement("SignalRef", typeof(SignalRef)), XmlElement("DiffSignalRef", typeof(DiffSignalRef)), XmlElement("SignalGroupRef", typeof(SignalGroupRef)),] public Object _Refs;
    std::variant<SignalRef, DiffSignalRef, SignalGroupRef> _Refs;
};
//	#endregion

/// <summary>
/// Различные сервисные функции
/// </summary>
class Ut final {
    /// <summary>
    /// Конвертация единиц измерения
    /// </summary>
    /// <param name="value">значение</param>
    /// <param name="in_units">текущие единицы измерения</param>
    /// <param name="out_units">выходные единицы измерения</param>
    /// <returns>Возвращает сконвертированное значение</returns>
public:
    static float UnitsConvert(float value, dist_ in_units, dist_ out_units);
};
// } // namespace TopoR_PCB_Classes

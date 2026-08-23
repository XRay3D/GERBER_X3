#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

class QGraphicsItem;
namespace TopoR {
// Раздел «Соединения на плате».
// В этом разделе описывается конкретная реализация соединений: печатные проводники, межслойные переходы и области металлизации.
struct Connectivity {
    // Переходное отверстие на плате.
    struct Via {
        // Признак фиксации.
        [[= XML::Attr]] Bool fixed{};
        // public bool fixedSpecified
        bool getFixedSpecified() const;
        // Ссылка на тип переходного отверстия.
        /*[[= XML::Elem]]*/ ViastackRef ViastackRef;
        // Ссылка на цепь.
        /*[[= XML::Elem]]*/ NetRef NetRef;
        // Точка привязки объекта.
        /*[[= XML::Elem]]*/ Org Org;
    };
    // Описание змейки.
    // ! Проводники, реализующие змейку, описываются в секции Wires (см. описание раздела Connectivity)
    struct Serpent {
        // Идентификатор неименованных объектов.
        [[= XML::Attr]] std::string id;
        // Параметр змейки: требуемая длина.
        [[= XML::Attr]] double length{};
        // Параметр змейки: зазор между соседними изломами.
        [[= XML::Attr]] double gap{};
        // Параметр змейки: высота h1 (см. описание змейки Serpent).
        [[= XML::Attr]] double h1{};
        // Параметр змейки: высота h2 (см. описание змейки Serpent).
        [[= XML::Attr]] double h2{};
        // Параметр змейки: высота h3 (см. описание змейки Serpent).
        [[= XML::Attr]] double h3{};
        // Параметр змейки: высота h4 (см. описание змейки Serpent).
        [[= XML::Attr]] double h4{};
    };
    // Описание застёгнутой пары проводников.
    // ! Сегменты(Track) описывают осевую линию пары.Форма проводников пары рассчитывается автоматически.
    struct ZippedWire {
        // Идентификатор неименованных объектов.
        [[= XML::Attr]] std::string id;
        // Признак фиксации.
        [[= XML::Attr]] Bool fixed{};
        // public bool fixedSpecified
        bool getFixedSpecified() const;
        // Ссылка на слой.
        /*[[= XML::Elem]]*/ LayerRef LayerRef;
        // Ссылка на дифференциальный сигнал.
        /*[[= XML::Elem]]*/ DiffSignalRef DiffSignalRef;
        // Начальная точка линии, дуги.
        /*[[= XML::Elem]]*/ Start Start;
        // Описание(я) сегмента проводника.
        // ! В случае отсутствия - предупреждение. Весь ZippedWire будет проигнорирован.
        // public List<Object> Tracks;
        [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ TrackLine, TrackArc, TrackArcCW>> Tracks;
        // bool ShouldSerialize_Tracks();
    };
    // Описание проводника.
    struct Wire {
        // Описание части проводника (последовательность сегментов с одной шириной и одинаковым признаком фиксации).
        // ! Атрибут zipwireRef (ссылка на застёгнутую пару проводников) используется, если описываемая часть проводника входит в застёгнутую пару проводников ZippedWire (см. пример описания проводника дифференциальной пары).
        struct Subwire {
            // Описание «капельки» четырёхугольником. Первая вершина соответствует точке привязки контакта (переходного отверстия). Остальные вершины описывают контур четырёхугольника против часовой стрелки.
            //  ! TopoR при импорте игнорирует информацию о капельках.
            struct Teardrop {
                // координаты точки, вершины.
                ///*[[= XML::Elem]]*/ // public List<Dot> Dots;
                [[= XML::Elem]] std::vector<Dot> Dots;
                // bool ShouldSerialize_Dots();
            };
            // Признак фиксации.
            [[= XML::Attr]] Bool fixed{};
            // public bool fixedSpecified
            bool getFixedSpecified() const;
            // Ширина проводника.
            [[= XML::Attr]] double width{};
            // Ссылка на застёгнутую пару проводников. Строка должна содержать идентификатор описанной застёгнутой пары проводников ZippedWire.
            [[= XML::Attr]] std::string zipwireRef;
            // Описание «капелек» для Subwire.
            // От KilkennyCat: сделал как массив, в спецификации не так, но так удобней
            //[XmlArrayItem/*("Teardrop")*/] public List<Teardrop> Teardrops;
            [[= XML::Array]] std::vector<Teardrop> Teardrops;
            // bool ShouldSerialize_Teardrops();
            // Начальная точка линии, дуги.
            /*[[= XML::Elem]]*/ Start Start;
            // Описание(я) сегмента проводника.
            // ! В случае отсутствия - предупреждение. Весь проводник будет проигнорирован.
            // public List<Object> Tracks;
            [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ TrackLine, TrackArc, TrackArcCW>> Tracks;
            // bool ShouldSerialize_Tracks();
            QGraphicsItem* graphicsItem(const QColor& color) const;
        };
        // Ссылка на слой.
        /*[[= XML::Elem]]*/ LayerRef LayerRef;
        // Ссылка на цепь.
        /*[[= XML::Elem]]*/ NetRef NetRef;
        // Части проводника (последовательность сегментов с одной шириной и одинаковым признаком фиксации).
        ///*[[= XML::Elem]]*/ // public List<Subwire> Subwires;
        [[= XML::Elem]] std::vector<Subwire> Subwires; // FIXME
        // /*[[= XML::Elem]]*/ Subwire Subwire;
        bool ShouldSerialize_Subwires();
    };
    // Описание заливаемой области металлизации (полигона).
    // ! Заливка полигона линиями (Fill) записывается только для других САПР. TopoR при импорте её игнорирует. Сплошная заливка (fillType = Solid) не записывается.
    struct Copper {
        // Описание термобарьера для подключения контактных площадок к области металлизации.
        struct ThermalPad {
            // Описание термобарьера.
            /*[[= XML::Elem]]*/ Thermal Thermal;
        };
        // Описание термобарьера для подключения площадок переходных отверстий к области металлизации.
        struct ThermalVia {
            // Описание термобарьера.
            /*[[= XML::Elem]]*/ Thermal Thermal;
        };
        // Описание контура заливаемой области металлизации.
        struct Shape {
            // Описание залитой фигуры.
            // public Object FilledFigure;
            /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ FilledCircle, FilledRect, Polygon, FilledContour> FilledFigure;
        };
        // Описание островка области металлизации.
        struct Island {
            // Описание спицы термобарьера, присутствующего на плате
            struct ThermalSpoke {
                // Толщина линии.
                [[= XML::Attr]] double lineWidth{};
                // Описания координат точек, вершин.
                // ! В случае отсутствия - весь ThermalSpoke будет проигнорирован.
                ///*[[= XML::Elem]]*/ // public List<Dot> Dots;
                [[= XML::Elem]] std::vector<Dot> Dots;
                // bool ShouldSerialize_Dots();
            };
            // Описание многоугольника.
            // public Object Polygon;
            [[= XML::Elem("Polygon")]] std::variant</*XML::Null,*/ Polygon, FilledContour> polygon;
            // Вырезы в островке области металлизации.
            // ! В случае отсутствия - критическая ошибка. Обязан быть пустой тэг.
            //[XmlArrayItem/*("Polygon")*/, XmlArrayItem/*("FilledContour")*/] public List<Object> Voids;
            [[= XML::Array(XML::DontSkip)]] std::vector<std::variant</*XML::Null,*/ Polygon, FilledContour>> Voids;
            // Описание спиц термобарьеров, присутствующих на плате
            ///*[[= XML::Elem]]*/ // public List<ThermalSpoke> ThermalSpokes;
            [[= XML::Elem]] std::vector<ThermalSpoke> ThermalSpokes;
            // bool ShouldSerialize_ThermalSpokes();
        };
        // Параметр области металлизации (полигона): приоритет заливки.
        [[= XML::Attr]] int priority{};
        // Параметр области металлизации (полигона): использовать указанный зазор.
        [[= XML::Attr]] Bool useBackoff{};
        // public bool useBackoffSpecified
        bool getUseBackoffSpecified() const;
        // Параметр области металлизации (полигона): зазор до области металлизации.
        [[= XML::Attr]] double backoff{};
        // Параметр области металлизации (полигона) стека: подключение контактных площадок.
        [[= XML::Attr]] connectPad connectPad{};
        // Параметр области металлизации (полигона): подключение площадок переходных отверстий.
        [[= XML::Attr]] connectVia connectVia{};
        // Толщина линии.
        [[= XML::Attr]] double lineWidth{};
        // Параметр области металлизации (полигона): зазор между линиями штриховки.
        [[= XML::Attr]] double lineClr{};
        // Параметр области металлизации (полигона): минимальная площадь островка.
        [[= XML::Attr]] double minSquare{};
        // Параметр области металлизации (полигона): точность аппроксимации контура.
        [[= XML::Attr]] precision precision{};
        // Параметр области металлизации (полигона): удалять неподключенные островки.
        [[= XML::Attr]] Bool deleteUnconnected{};
        // public bool deleteUnconnectedSpecified
        bool getDeleteUnconnectedSpecified() const;
        // Параметр области металлизации (полигона): состояние.
        [[= XML::Attr]] state state{};
        // Параметр области металлизации (полигона): тип заливки.
        [[= XML::Attr]] fillType fillType{};
        // Ссылка на слой.
        /*[[= XML::Elem]]*/ LayerRef LayerRef;
        // Ссылка на цепь.
        /*[[= XML::Elem]]*/ NetRef NetRef;
        // Описание термобарьера для подключения контактных площадок к области металлизации.
        // ! В случае отсутствия - критическая ошибка. Обязан быть пустой тэг.
        /*[[= XML::Elem]]*/ ThermalPad ThermalPad;
        // Описание термобарьера для подключения площадок переходных отверстий к области металлизации.
        // ! В случае отсутствия - критическая ошибка. Обязан быть пустой тэг.
        /*[[= XML::Elem]]*/ ThermalVia ThermalVia;
        // Описание контура заливаемой области металлизации..
        // ! В случае отсутствия - критическая ошибка. Обязан быть пустой тэг.
        /*[[= XML::Elem]]*/ Shape Shape;
        // Вырезы в областях металлизации (полигонах) заданные пользователем.
        // ! В случае отсутствия - критическая ошибка. Обязан быть пустой тэг.
        //[XmlArrayItem/*("FilledCircle")*/, XmlArrayItem/*("FilledRect")*/, XmlArrayItem/*("Polygon")*/, XmlArrayItem/*("FilledContour")*/] public List<Object> Voids;
        [[= XML::Array(XML::DontSkip)]] std::vector<std::variant</*XML::Null,*/ FilledCircle, FilledRect, Polygon, FilledContour>> Voids;
        // Островки области металлизации.
        // ! В случае отсутствия - критическая ошибка. Обязан быть пустой тэг.
        //[XmlArrayItem/*("Island")*/] public List<Island> Islands;
        [[= XML::Array]] std::vector<Island> Islands;
        // Заливка областей металлизации (полигонов) линиями.
        // ! TopoR при импорте игнорирует эту информацию и строит заливку заново.
        //[XmlArrayItem/*("Line")*/] public List<Line> Fill_lines;
        [[= XML::Array]] std::vector<Line> FillLines;
        // bool ShouldSerialize_Fill_lines();
    };
    // Описание незаливаемой области металлизации.
    struct NonfilledCopper {
        // Описание контура незаливаемой области металлизации.
        struct Shape_NonfilledCopper {
            // Описание фигуры.
            // public Object FigureContPoliline;
            /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Circle, Line, Polyline, Rect, Contour> FigureContPoliline;
        };
        // Толщина линии.
        [[= XML::Attr]] double lineWidth{};
        // Ссылка на слой.
        /*[[= XML::Elem]]*/ LayerRef LayerRef;
        // Ссылка на цепь.
        /*[[= XML::Elem]]*/ NetRef NetRef;
        // Описание контура незаливаемой области металлизации.
        /*[[= XML::Elem]]*/ Shape_NonfilledCopper Shape;
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Переходные отверстия на плате.
    //[XmlArrayItem/*("Via")*/] public List<Via> Vias;
    [[= XML::Array]] std::vector<Via> Vias;
    // bool ShouldSerialize_Vias();
    // Змейки
    //[XmlArrayItem/*("Serpent")*/] public List<Serpent> Serpents;
    [[= XML::Array]] std::vector<Serpent> Serpents;
    // bool ShouldSerialize_Serpents();
    // Застёгнутые пары проводников.
    //[XmlArrayItem/*("ZippedWire")*/] public List<ZippedWire> ZippedWires;
    [[= XML::Array]] std::vector<ZippedWire> ZippedWires;
    // bool ShouldSerialize_ZippedWires();
    // Проводники.
    //[XmlArrayItem/*("Wire")*/] public List<Wire> Wires;
    [[= XML::Array]] std::vector<Wire> Wires;
    // bool ShouldSerialize_Wires();
    // Oбласти металлизации (полигонов).
    //[XmlArrayItem/*("Copper")*/] public List<Copper> Coppers;
    [[= XML::Array]] std::vector<Copper> Coppers;
    // bool ShouldSerialize_Coppers();
    // Незаливаемые области металлизации.
    //[XmlArrayItem/*("NonfilledCopper")*/] public List<NonfilledCopper> NonfilledCoppers;
    [[= XML::Array]] std::vector<NonfilledCopper> NonfilledCoppers;
    // bool ShouldSerialize_NonfilledCoppers();
    /************************************************************************
     * Здесь находятся функции для работы с элементами класса Connectivity. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/
    /************************************************************************/
};
} // namespace TopoR

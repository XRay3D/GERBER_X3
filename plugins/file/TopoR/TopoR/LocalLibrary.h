#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Библиотечные элементы». (Обязательный раздел)
struct LocalLibrary {
    struct BasePad {
        // Ссылка на слой или тип слоя.
        // public Object Reference;
        /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ LayerTypeRef, LayerRef> Reference;
    };
    // Описание круглой контактной площадки.
    struct PadCircle : public BasePad {
        // Диаметр окружности, круга, овала.
        [[= XML::Attr]] double diameter{};
        operator QPainterPath() const;
    };
    // Описание овальной контактной площадки.
    struct PadOval : public BasePad {
        // Диаметр окружности, круга, овала.
        [[= XML::Attr]] double diameter{};
        // Параметр овальной контактной площадки: вытягивание по осям x и y.
        /*[[= XML::Elem]]*/ Stretch Stretch;
        // Параметр контактной площадки: смещение точки привязки по осям x и y.
        /*[[= XML::Elem]]*/ Shift Shift;
        operator QPainterPath() const;
    };
    // Описание прямоугольной контактной площадки.
    // Дополнительные атрибуты(handling и handlingValue) позволяют задавать тип и величину обработки углов.
    // В качестве типа обработки допускается скругление или срез.
    // Тип для всех углов должен быть одинаковым: нельзя скруглять один угол и срезать другой.
    // Если флаг custom не установлен, обрабатываются все углы, иначе будут обработаны только углы,
    // соответствующие установленным флагам - cornerLB, cornerRB, cornerRT, cornerLT.
    // Основные формы КП, которые данный формат позволяет описать:
    // прямоугольные КП;
    // прямоугольные КП со скруглёнными углами;
    // прямоугольные КП со срезанными углами;
    // Finger pads.
    struct PadRect : public BasePad {
        // Ширина прямоугольной контактной площадки.
        [[= XML::Attr]] double width{};
        // Высота прямоугольной контактной площадки.
        [[= XML::Attr]] double height{};
        // Тип обработки углов прямоугольной контактной площадки.
        [[= XML::Attr]] Handling handling{};
        // public bool handlingSpecified
        bool getHandlingSpecified() const;
        // Величина обработки углов прямоугольной контактной площадки. Значение зависит от типа обработки. Для скругления это радиус. Для среза это высота среза.
        [[= XML::Attr]] double handlingValue{};
        // public bool handlingValueSpecified
        bool getHandlingValueSpecified() const;
        // Флаг выборочной обработки углов прямоугольной контактной площадки. Если не установлен, то все углы обрабатываются одинаковым образом.
        [[= XML::Attr]] Bool custom{};
        // public bool customSpecified
        bool getCustomSpecified() const;
        // Флаг обработки левого нижнего угла прямоугольной контактной площадки.
        [[= XML::Attr]] Bool cornerLB{};
        // public bool cornerLBSpecified
        bool getCornerLBSpecified() const;
        // Флаг обработки правого нижнего угла прямоугольной контактной площадки.
        [[= XML::Attr]] Bool cornerRB{};
        // public bool cornerRBSpecified
        bool getCornerRBSpecified() const;
        // Флаг обработки правого нижнего угла прямоугольной контактной площадки.
        [[= XML::Attr]] Bool cornerRT{};
        // public bool cornerRTSpecified
        bool getCornerRTSpecified() const;
        // Флаг обработки левого верхнего угла прямоугольной контактной площадки.
        [[= XML::Attr]] Bool cornerLT{};
        // public bool cornerLTSpecified
        bool getCornerLTSpecified() const;
        // Параметр контактной площадки: смещение точки привязки по осям x и y.
        /*[[= XML::Elem]]*/ Shift Shift;
        operator QPainterPath() const;
    };
    // Описание полигональной контактной площадки.
    struct PadPoly : public BasePad {
        // Массив координат точек, вершин.
        // ! Минимум 3 элемента
        ///*[[= XML::Elem]]*/ // public List<Dot> Dots;//("Dot")
        [[= XML::Elem]] std::vector<Dot> Dots;
        // bool ShouldSerialize_Dots();
        operator QPolygonF() const;
        operator QPainterPath() const;
    };
    // Описание стека контактных площадок.
    struct Padstack {
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name;
        // Тип стека контактных площадок.
        [[= XML::Attr]] TypePadstack type{};
        // Диаметр отверстия.
        [[= XML::Attr]] double holeDiameter{};
        // Параметр стека контактной площадки: металлизация отверстия.
        [[= XML::Attr]] Bool metallized{};
        // public bool metallizedSpecified
        bool getMetallizedSpecified() const;
        // Параметр стека контактной площадки: подключение к области металлизации (полигону).
        [[= XML::Attr]] ConnectToCopper connectToCopper{};
        // Описание термобарьера.
        /*[[= XML::Elem]]*/ Thermal Thermal;
        // Контактные площадки стека.
        // <value>PadCircle, PadOval, PadRect, PadPoly</value>
        //[XmlArrayItem("PadCircle", typeof(PadCircle)), XmlArrayItem("PadOval", typeof(PadOval)), XmlArrayItem("PadRect", typeof(PadRect)), XmlArrayItem("PadPoly", typeof(PadPoly))] public List<Object> Pads;
        [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ PadCircle, PadOval, PadRect, PadPoly>> Pads;
        // bool ShouldSerialize_Pads();
    };
    // Описание типа (стека) переходного отверстия.
    struct Viastack {
        // Диапазон слоев.
        // <value>AllLayers | [LayerRef]</value>
        struct LayerRange {
            // AllLayers - yстанавливает область действия правила: все слои. См. также LayerRefs
            // ! При null необходимо смотреть LayersRefs - там описан список ссылок типа LayerRef.
            // ORIGINAL LINE XmlElement: [AllLayers] public AllLayers AllLayers;
            AllLayers AllLayers;
            // Диапазон слоёв. См. также AllLayers
            // ! При null необходимо смотреть наличие AllLayers.
            // ORIGINAL LINE XmlElement: [LayerRef] public List<LayerRef> LayerRefs;
            std::vector<LayerRef> LayerRefs;
            bool ShouldSerializeLayerRefs();
        };
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name;
        // Диаметр отверстия.
        [[= XML::Attr]] double holeDiameter{};
        // Параметр типа переходного отверстия: возможность установить переходное отверстие на контактной площадке.
        [[= XML::Attr]] Bool viaOnPin{};
        // public bool viaOnPinSpecified
        bool getViaOnPinSpecified() const;
        // Диапазон слоев.
        // <value>AllLayers | [LayerRef]</value>
        // ORIGINAL LINE XmlElement: [LayerRange] public LayerRange LayerRange;
        LayerRange LayerRange;
        // Описание площадок стека переходного отверстия.
        //[XmlArrayItem("PadCircle", typeof(PadCircle))] public List<PadCircle> ViaPads;
        [[= XML::Array]] std::vector<PadCircle> ViaPads;
        // bool ShouldSerialize_ViaPads();
    };
    // Описание посадочного места.
    struct Footprint {
        // Описание области металлизации (полигона) в посадочном месте компонента.
        struct Copper {
            // Толщина линии.
            [[= XML::Attr]] double lineWidth{};
            // Ссылка на слой.
            /*[[= XML::Elem]]*/ LayerRef LayerRef;
            // Описание фигуры.
            // <value>ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon</value>
            // public Object Figure;
            /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon, FilledContour> Figure;
        };
        // Описание запрета в посадочном месте Footprint. Для запрета размещения должен быть указан слой с типом Assy.
        struct Keepout {
            // Ссылка на слой.
            /*[[= XML::Elem]]*/ LayerRef LayerRef;
            // Описание фигуры.
            // <value>ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon</value>
            // public Object Figure;
            /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon, FilledContour> Figure;
        };
        // Описание монтажного отверстия в посадочном месте.
        struct Mnthole {
            // Идентификатор неименованных объектов.
            [[= XML::Attr]] std::string id;
            // Ссылка на стек контактных площадок.
            /*[[= XML::Elem]]*/ PadstackRef PadstackRef;
            // Точка привязки объекта.
            /*[[= XML::Elem]]*/ Org Org;
        };
        // Описание ярлыка в посадочном месте.
        struct Label {
            // Имя объекта или ссылка на именованный объект.
            [[= XML::Attr]] std::string name;
            // Параметр надписей (ярлыков): способ выравнивания текста.
            [[= XML::Attr]] align align{}; //("align")
            // Задаёт угол в градусах c точностью до тысячных долей.
            [[= XML::Attr]] double angle{};
            // Параметр надписей и ярлыков: зеркальность отображения.
            [[= XML::Attr]] Bool mirror{};
            // public bool mirrorSpecified
            bool getMirrorSpecified() const;
            // Ссылка на слой.
            /*[[= XML::Elem]]*/ LayerRef LayerRef;
            // Ссылка на стиль надписей.
            /*[[= XML::Elem]]*/ TextStyleRef TextStyleRef;
            // Точка привязки объекта.
            /*[[= XML::Elem]]*/ Org Org;
            QTransform transform() const {
                QTransform transform;
                if(Org) transform.translate(Org.x, Org.y);
                if(angle) transform.rotate(angle);
                return transform;
            }
        };
        // Описание контактной площадки (вывода) посадочного места.
        // ! В системе TopoR поддерживаются планарные контакты на внешних металлических слоях и не поддерживаются на внутренних.
        // Т.е.у планарного контакта может быть только одна площадка или на верхней стороне, или на нижней.
        // В описании планарного контакта используется только слой Top.
        // Это означает, что контактная площадка будет находиться на одной стороне с компонентом.
        // Если же площадка находится на противоположной стороне, то должен быть установлен флаг flipped.
        // Этот флаг устанавливается в описании контакта посадочного места.
        //
        struct Pad {
            // Номер контактной площадки (вывода) посадочного места.
            [[= XML::Attr]] int padNum{};
            // Имя объекта или ссылка на именованный объект.
            [[= XML::Attr]] std::string name;
            // Задаёт угол в градусах c точностью до тысячных долей.
            [[= XML::Attr]] double angle{};
            // Параметр контакта (вывода) посадочного места: перевёрнутость.
            // Если флаг не установлен, площадка планарного контакта будет находиться на одной стороне с компонентом,
            // иначе площадка будет расположена на противоположной стороне.
            [[= XML::Attr]] Bool flipped{};
            // public bool flippedSpecified
            bool getFlippedSpecified() const;
            // Ссылка на стек контактных площадок.
            [[= XML::ElemF]] PadstackRef PadstackRef; //("PadstackRef")
            // Точка привязки объекта.
            [[= XML::ElemF]] Org Org; //("Org")
            QTransform transform() const {
                QTransform transform;
                if(Org) transform.translate(Org.x, Org.y);
                if(angle) transform.rotate(angle);
                return transform;
            }
        };
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name;
        // Описание контактных площадок посадочного места.
        //[XmlArrayItem("Pad")] public List<Pad> Pads;
        [[= XML::Array]] std::vector<Pad> Pads;
        // bool ShouldSerialize_Pads();
        // Надписи.
        //[XmlArrayItem("Text")] public List<Text> Texts;
        [[= XML::Array]] std::vector<Text> Texts;
        // bool ShouldSerialize_Texts();
        // Детали посадочного места.
        //[XmlArrayItem("Detail")] public List<Detail> Details;
        [[= XML::Array]] std::vector<Detail> Details;
        // bool ShouldSerialize_Details();
        // Области металлизации (полигонов) в посадочных местах компонентов.
        //[XmlArrayItem("Copper")] public List<Copper> Coppers;
        [[= XML::Array]] std::vector<Copper> Coppers;
        // bool ShouldSerialize_Coppers();
        // Запреты размещения в посадочном месте.
        //[XmlArrayItem("Keepout")] public List<Keepout> KeepoutsPlace;
        [[= XML::Array]] std::vector<Keepout> KeepoutsPlace;
        // bool ShouldSerialize_KeepoutsPlace();
        // Запреты трассировки в посадочном месте.
        //[XmlArrayItem("Keepout")] public List<Keepout> KeepoutsTrace;
        [[= XML::Array]] std::vector<Keepout> KeepoutsTrace;
        // bool ShouldSerialize_KeepoutsTrace();
        // Монтажные отверстия.
        //[XmlArrayItem("Mnthole")] public List<Mnthole> Mntholes;
        [[= XML::Array]] std::vector<Mnthole> Mntholes;
        // bool ShouldSerialize_Mntholes();
        // Ярлыки.
        //[XmlArrayItem("Label")] public List<Label> Labels;
        [[= XML::Array]] std::vector<Label> Labels;
        // bool ShouldSerialize_Labels();
        std::string ToString();
    };
    // Описание схемного компонента.
    struct Component {
        // Описание контакта схемного компонента.
        struct Pin {
            // Номер контакта компонента.
            [[= XML::Attr]] int pinNum{};
            // Имя объекта или ссылка на именованный объект.
            [[= XML::Attr]] std::string name;
            // Схемотехническое имя контакта компонента.
            [[= XML::Attr]] std::string pinSymName;
            // Параметр контакта компонента: эквивалентность.
            [[= XML::Attr]] int pinEqual{};
            // Параметр контакта (вывода) компонента: номер вентиля контакта.
            [[= XML::Attr]] int gate{};
            // Параметр контакта (вывода) компонента: эквивалентность вентиля контакта.
            [[= XML::Attr]] int gateEqual{};
        };
        // Описание атрибута схемного компонента.
        struct Attribute {
            // Имя объекта или ссылка на именованный объект.
            [[= XML::Attr]] std::string name;
            // Значение атрибута.
            [[= XML::Attr]] std::string value;
        };
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name;
        // Контакты схемного компонента.
        //[XmlArrayItem("Pin")] public List<Pin> Pins;
        [[= XML::Array]] std::vector<Pin> Pins;
        // bool ShouldSerialize_Pins();
        // Атрибуты компонента.
        //[XmlArrayItem("Attribute")] public List<Attribute> Attributes;
        [[= XML::Array]] std::vector<Attribute> Attributes;
        // bool ShouldSerialize_Attributes();
        std::string ToString();
    };
    // Описание упаковки (соответствие контактов компонента и выводов посадочного места).
    struct Package {
        // Соответствие контакта схемного компонента и вывода посадочного места.
        struct Pinpack {
            // Номер контакта компонента.
            [[= XML::Attr]] int pinNum{};
            // Номер контактной площадки (вывода) посадочного места.
            [[= XML::Attr]] int padNum{};
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            [[= XML::Attr]] ValueType valueType{};
            // Параметр контакта компонента в посадочном месте: задержка сигнала в посадочном месте.
            [[= XML::Attr]] double delay{};
        };
        // Ссылка на схемный компонент.
        /*[[= XML::Elem]]*/ ComponentRef ComponentRef; //("ComponentRef")
                                                       // Ссылка на посадочное место.
        /*[[= XML::Elem]]*/ FootprintRef FootprintRef; //("FootprintRef")
        // Соответствие контакта схемного компонента и вывода посадочного места.
        ///*[[= XML::Elem]]*/ // public List<Pinpack> Pinpacks;//("Pinpack")
        [[= XML::Elem]] std::vector<Pinpack> Pinpacks; //("Pinpack")
        // bool ShouldSerialize_Pinpacks();
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Стеки контактных площадок.
    //[XmlArrayItem("Padstack")] public List<Padstack> Padstacks;
    [[= XML::Array]] std::vector<Padstack> Padstacks;
    // bool ShouldSerialize_Padstacks();
    // Типы (стеки) переходных отверстий.
    //[XmlArrayItem("Viastack")] public List<Viastack> Viastacks;
    [[= XML::Array]] std::vector<Viastack> Viastacks;
    // bool ShouldSerialize_Viastacks();
    // Посадочные места.
    //[XmlArrayItem("Footprint")] public List<Footprint> Footprints;
    [[= XML::Array]] std::vector<Footprint> Footprints;
    // bool ShouldSerialize_Footprints();
    // Схемные компоненты.
    //[XmlArrayItem("Component")] public List<Component> Components;
    [[= XML::Array]] std::vector<Component> Components;
    // bool ShouldSerialize_Components();
    // Упаковки.
    //[XmlArrayItem("Package")] public List<Package> Packages;
    [[= XML::Array]] std::vector<Package> Packages;
    // bool ShouldSerialize_Packages();
    /************************************************************************
     * Здесь находятся функции для работы с элементами класса LocalLibrary. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/

    std::optional<const Padstack&> getPadstack(std::string_view name) const {
        auto ps = r::find(Padstacks, name, &Padstack::name);
        if(ps != Padstacks.end()) return *ps.base();
        return {};
    }
    std::optional<const Viastack&> getViastack(std::string_view name) const {
        auto ps = r::find(Viastacks, name, &Viastack::name);
        if(ps != Viastacks.end()) return *ps.base();
        return {};
    }
    std::optional<const Footprint&> getFootprint(std::string_view name) const {
        auto ps = r::find(Footprints, name, &Footprint::name);
        if(ps != Footprints.end()) return *ps.base();
        return {};
    }
    std::optional<const Component&> getComponent(std::string_view name) const {
        auto ps = r::find(Components, name, &Component::name);
        if(ps != Components.end()) return *ps.base();
        return {};
    }
    /*
    std::optional<const Package&> getPackage(std::string_view name) const {
        auto ps = r::find(Packages, name, &Package::name);
        if(ps != Packages.end()) return *ps.base();
        return {};
    }
*/
    /************************************************************************/
};
} // namespace TopoR

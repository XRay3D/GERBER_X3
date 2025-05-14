#pragma once

#include "Commons.h"

#include <QPainterPath>
#include <QPolygonF>

/* Мною, Константином aka KilkennyCat,05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

class QGraphicsItemGroup;

namespace TopoR {

class TopoR_PCB_File;

// Раздел «Библиотечные элементы». (Обязательный раздел)
struct LocalLibrary {
    // Ссылка на слой или тип слоя.
    using BasePadRef = Xml::Variant<LayerTypeRef, LayerRef>;

    // Описание круглой контактной площадки.
    struct PadCircle {
        BasePadRef Reference;
        // Диаметр окружности, круга, овала.
        Xml::Attr<double> diameter;
        operator QPainterPath() const;
    };
    // Описание овальной контактной площадки.
    struct PadOval {
        BasePadRef Reference;
        // Диаметр окружности, круга, овала.
        Xml::Attr<double> diameter;
        // Параметр овальной контактной площадки: вытягивание по осям x и y.
        Stretch stretch;
        // Параметр контактной площадки: смещение точки привязки по осям x и y.
        Xml::Optional<Shift> shift;
        operator QPainterPath() const;
    };
    // Описание прямоугольной контактной площадки.
    // Дополнительные атрибуты(handling и handlingValue) позволяют задавать тип и величину обработки углов.
    // В качестве типа обработки допускается скругление или срез.
    // Тип для всех углов должен быть одинаковым: нельзя скруглять один угол и срезать другой.
    // Если флаг custom не установлен, обрабатываются все углы, иначе будут обработаны только углы,
    // соответствующие установленным флагам - cornerLB,cornerRB,cornerRT,cornerLT.
    // Основные формы КП, которые данный формат позволяет описать:
    // прямоугольные КП;
    // прямоугольные КП со скруглёнными углами;
    // прямоугольные КП со срезанными углами;
    // Finger pads.
    struct PadRect {
        BasePadRef Reference;
        // Ширина прямоугольной контактной площадки.
        Xml::Attr<double> width;
        // Высота прямоугольной контактной площадки.
        Xml::Attr<double> height;
        // Тип обработки углов прямоугольной контактной площадки.
        Xml::Attr<Handling> handling;
        // Величина обработки углов прямоугольной контактной площадки. Значение зависит от типа обработки. Для скругления это радиус. Для среза это высота среза.
        Xml::Attr<double> handlingValue;
        // Флаг выборочной обработки углов прямоугольной контактной площадки. Если не установлен, то все углы обрабатываются одинаковым образом.
        Xml::Attr<Bool> custom;
        // Флаг обработки левого нижнего угла прямоугольной контактной площадки.
        Xml::Attr<Bool> cornerLB;
        // Флаг обработки правого нижнего угла прямоугольной контактной площадки.
        Xml::Attr<Bool> cornerRB;
        // Флаг обработки правого нижнего угла прямоугольной контактной площадки.
        Xml::Attr<Bool> cornerRT;
        // Флаг обработки левого верхнего угла прямоугольной контактной площадки.
        Xml::Attr<Bool> cornerLT;
        // Параметр контактной площадки: смещение точки привязки по осям x и y.
        Xml::Optional<Shift> shift;
        operator QPainterPath() const;
    };
    // Описание полигональной контактной площадки.
    struct PadPoly {
        BasePadRef Reference;
        // Массив координат точек, вершин.
        /// \note !Минимум 3 элемента
        Xml::Array<Dot> Dots;
        operator QPolygonF() const;
        operator QPainterPath() const;
    };
    // Описание стека контактных площадок.
    struct Padstack {
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Тип стека контактных площадок.
        Xml::Attr<type_padstack> type;
        // Диаметр отверстия.
        Xml::Attr<double> holeDiameter;
        // Параметр стека контактной площадки: металлизация отверстия.
        Xml::Attr<Bool> metallized;
        // Параметр стека контактной площадки: подключение к области металлизации (полигону).
        Xml::Attr<ConnectToCopper> connectToCopper;
        // Описание термобарьера.
        Thermal thermal;
        // Контактные площадки стека.
        Xml::ArrayElem<Xml::Variant<PadCircle, PadOval, PadRect, PadPoly>> Pads;
        static QString getReference(const Xml::Variant<PadCircle, PadOval, PadRect, PadPoly>& padShape);
    };
    // Описание типа (стека) переходного отверстия.
    struct Viastack {
        // Диапазон слоев.
        // <value>AllLayers | [LayerRef]</value>
        struct LayerRange {
            // AllLayers - yстанавливает область действия правила: все слои. См. также LayerRefs_
            /// \note !При null необходимо смотреть LayersRefs_ - там описан список ссылок типа LayerRef.
            AllLayers allLayers;
            // Диапазон слоёв. См. также allLayers
            /// \note !При null необходимо смотреть наличие AllLayers.
            Xml::Array<LayerRef> LayerRefs;
        };
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Диаметр отверстия.
        Xml::Attr<double> holeDiameter;
        // Параметр типа переходного отверстия: возможность установить переходное отверстие на контактной площадке.
        Xml::Attr<Bool> viaOnPin;
        // Диапазон слоев.
        // <value>AllLayers | [LayerRef]</value>
        LayerRange layerRange;
        // Описание площадок стека переходного отверстия.
        Xml::ArrayElem<PadCircle> ViaPads;
    };
    using VariantFig = Xml::Variant<ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon, FilledContour>;
    // Описание посадочного места.
    struct Footprint {
        // Описание области металлизации (полигона) в посадочном месте компонента.
        struct Copper /*_Footprint*/ {
            // Толщина линии.
            Xml::Attr<double> lineWidth;
            // Ссылка на слой.
            LayerRef layerRef;
            // Описание фигуры.
            // <value>ArcCCW,ArcCW,ArcByAngle,ArcByMiddle,Line,Circle,Rect,FilledCircle,FilledRect,Polygon</value>
            VariantFig Figure;
        };
        // Описание запрета в посадочном месте Footprint. Для запрета размещения должен быть указан слой с типом Assy.
        struct Keepout /*_Place_Trace*/ {
            // Ссылка на слой.
            LayerRef layerRef;
            // Описание фигуры.
            // <value>ArcCCW,ArcCW,ArcByAngle,ArcByMiddle,Line,Circle,Rect,FilledCircle,FilledRect,Polygon</value>

            VariantFig Figure;
        };
        // Описание монтажного отверстия в посадочном месте.
        struct Mnthole {
            // Идентификатор неименованных объектов.
            Xml::Attr<QString> id;
            // Ссылка на стек контактных площадок.
            PadstackRef padstackRef;
            // Точка привязки объекта.
            Org org;
        };
        // Описание ярлыка в посадочном месте.
        struct Label /*_Footprint*/ {
            // Имя объекта или ссылка на именованный объект.
            Xml::Attr<QString> name;
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
            Xml::Optional<Org> org;
            QTransform transform() const {
                QTransform transform;
                if(org)
                    transform.translate(org.value().x, org.value().y);
                if(angle)
                    transform.rotate(angle);
                return transform;
            }
        };
        // Описание контактной площадки (вывода) посадочного места.
        /// \note !В системе TopoR поддерживаются планарные контакты на внешних металлических слоях и не поддерживаются на внутренних.
        /// \note Т.е.у планарного контакта может быть только одна площадка или на верхней стороне, или на нижней.
        /// \note В описании планарного контакта используется только слой Top.
        /// \note Это означает, что контактная площадка будет находиться на одной стороне с компонентом.
        /// \note Если же площадка находится на противоположной стороне, то должен быть установлен флаг flipped.
        /// \note Этот флаг устанавливается в описании контакта посадочного места.
        struct Pad {
            // Номер контактной площадки (вывода) посадочного места.
            Xml::Attr<int> padNum;
            // Имя объекта или ссылка на именованный объект.
            Xml::Attr<QString> name;
            // Задаёт угол в градусах c точностью до тысячных долей.
            Xml::Attr<double> angle;
            // Параметр контакта (вывода) посадочного места: перевёрнутость.
            // Если флаг не установлен, площадка планарного контакта будет находиться на одной стороне с компонентом,
            // иначе площадка будет расположена на противоположной стороне.
            Xml::Attr<Bool> flipped;
            // Ссылка на стек контактных площадок.
            PadstackRef padstackRef;
            // Точка привязки объекта.
            Org org;
            QTransform transform() const {
                QTransform transform;
                transform.translate(org.x, org.y);
                if(angle) transform.rotate(angle);
                return transform;
            }
        };
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Описание контактных площадок посадочного места.
        Xml::ArrayElem<Pad> Pads;
        // Надписи.
        Xml::ArrayElem<Text> Texts;
        // Детали посадочного места.
        Xml::ArrayElem<Detail> Details;
        // Области металлизации (полигонов) в посадочных местах компонентов.
        Xml::ArrayElem<Copper> Coppers;
        // Запреты размещения в посадочном месте.
        Xml::ArrayElem<Keepout> KeepoutsPlace;
        // Запреты трассировки в посадочном месте.
        Xml::ArrayElem<Keepout> KeepoutsTrace;
        // Монтажные отверстия.
        Xml::ArrayElem<Mnthole> Mntholes;
        // Ярлыки.
        Xml::ArrayElem<Label> Labels;

        QGraphicsItem* graphicsItem(const TopoR_PCB_File& file) const;
    };
    // Описание схемного компонента.
    struct Component {
        // Описание контакта схемного компонента.
        struct Pin /*_Component*/ {
            // Номер контакта компонента.
            Xml::Attr<int> pinNum;
            // Имя объекта или ссылка на именованный объект.
            Xml::Attr<QString> name;
            // Схемотехническое имя контакта компонента.
            Xml::Attr<QString> pinSymName;
            // Параметр контакта компонента: эквивалентность.
            Xml::Attr<int, NoOpt> pinEqual;
            // Параметр контакта (вывода) компонента: номер вентиля контакта.
            Xml::Attr<int, NoOpt> gate;
            // Параметр контакта (вывода) компонента: эквивалентность вентиля контакта.
            Xml::Attr<int, NoOpt> gateEqual;
        };
        // Описание атрибута схемного компонента.
        struct Attribute /*_Component*/ {
            // Имя объекта или ссылка на именованный объект.
            Xml::Attr<QString> name;
            // Значение атрибута.
            Xml::Attr<QString> value;
        };
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Контакты схемного компонента.
        Xml::ArrayElem<Pin> Pins;
        // Атрибуты компонента.
        Xml::ArrayElem<Attribute> Attributes;
        QString ToString() { return name; }
    };
    // Описание упаковки (соответствие контактов компонента и выводов посадочного места).
    struct Package {
        // Соответствие контакта схемного компонента и вывода посадочного места.
        struct Pinpack {
            // Номер контакта компонента.
            Xml::Attr<int> pinNum;
            // Номер контактной площадки (вывода) посадочного места.
            Xml::Attr<int> padNum;
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            Xml::Attr<valueType> valueType_;

            // Параметр контакта компонента в посадочном месте: задержка сигнала в посадочном месте.
            Xml::Attr<double> delay;
        };
        // Ссылка на схемный компонент.
        ComponentRef componentRef;
        // Ссылка на посадочное место.
        FootprintRef footprintRef;
        // Соответствие контакта схемного компонента и вывода посадочного места.
        Xml::Array<Pinpack> Pinpacks;
    };
    // Версия раздела.
    Xml::Attr<QString> version;
    // Стеки контактных площадок.
    Xml::ArrayElem<Padstack> Padstacks;
    //  Типы (стеки) переходных отверстий.
    Xml::ArrayElem<Viastack> Viastacks;
    // Посадочные места.
    Xml::ArrayElem<Footprint> Footprints;
    // Схемные компоненты.
    Xml::ArrayElem<Component> Components;
    // Упаковки.
    Xml::ArrayElem<Package> Packages;
    /************************************************************************
     * Здесь находятся функции для работы с элементами класса LocalLibrary. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/
    mutable Xml::Skip<std::map<QString, QGraphicsItemGroup*>> footprints;
    const Padstack* getPadstack(const QString& name) const;
    const Footprint* getFootprint(const QString& name) const;
    const Component* getComponent(const QString& name) const;
    const Viastack* getViastack(const QString& name) const;
    /************************************************************************/
};

} // namespace TopoR

#pragma once

#include "Commons.h"
#include <any>
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Раздел «Библиотечные элементы». (Обязательный раздел)
/// </summary>
class LocalLibrary : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
public:
    class BasePad {
        /// <summary>
        /// Ссылка на слой или тип слоя.
        /// </summary>
        /// <value>LayerTypeRef | LayerRef</value>
    public:
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("LayerTypeRef", typeof(LayerTypeRef)), XmlElement("LayerRef", typeof(LayerRef))] public Object _Reference;
        std::any _Reference;
    };
    /// <summary>
    /// Описание круглой контактной площадки.
    /// </summary>
public:
    class PadCircle : public BasePad {
        /// <summary>
        /// Диаметр окружности, круга, овала.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("diameter", DataType = "float")] public float _diameter;
        float _diameter = 0.0F;
    };

    /// <summary>
    /// Описание овальной контактной площадки.
    /// </summary>
public:
    class PadOval : public BasePad {
        /// <summary>
        /// Диаметр окружности, круга, овала.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("diameter", DataType = "float")] public float _diameter;
        float _diameter = 0.0F;

        /// <summary>
        /// Параметр овальной контактной площадки: вытягивание по осям x и y.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Stretch")] public Stretch _Stretch;
        Stretch* _Stretch;

        /// <summary>
        /// Параметр контактной площадки: смещение точки привязки по осям x и y.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Shift")] public Shift _Shift;
        Shift* _Shift;
        virtual ~PadOval() {
            delete _Stretch;
            delete _Shift;
        }
    };

    /// <summary>
    /// Описание прямоугольной контактной площадки.
    /// Дополнительные атрибуты(handling и handlingValue) позволяют задавать тип и величину обработки углов.
    /// В качестве типа обработки допускается скругление или срез.
    /// Тип для всех углов должен быть одинаковым: нельзя скруглять один угол и срезать другой.
    /// Если флаг custom не установлен, обрабатываются все углы, иначе будут обработаны только углы,
    /// соответствующие установленным флагам - cornerLB, cornerRB, cornerRT, cornerLT.
    /// Основные формы КП, которые данный формат позволяет описать:
    /// прямоугольные КП;
    /// прямоугольные КП со скруглёнными углами;
    /// прямоугольные КП со срезанными углами;
    /// Finger pads.
    /// </summary>
public:
    class PadRect : public BasePad {
        /// <summary>
        /// Ширина прямоугольной контактной площадки.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("width", DataType = "float")] public float _width;
        float _width = 0.0F;

        /// <summary>
        /// Высота прямоугольной контактной площадки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("height", DataType = "float")] public float _height;
        float _height = 0.0F;

        /// <summary>
        /// Тип обработки углов прямоугольной контактной площадки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("handling")] public type_handling _handling;
        type_handling _handling = static_cast<type_handling>(0);
        virtual ~PadRect() {
            delete _Shift;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _handlingSpecified
        bool getHandlingSpecified() const;

        /// <summary>
        /// Величина обработки углов прямоугольной контактной площадки. Значение зависит от типа обработки. Для скругления это радиус. Для среза это высота среза.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("handlingValue", DataType = "float")] public float _handlingValue;
        float _handlingValue = 0.0F;

        // ORIGINAL LINE: [XmlIgnore] public bool _handlingValueSpecified
        bool getHandlingValueSpecified() const;

        /// <summary>
        /// Флаг выборочной обработки углов прямоугольной контактной площадки. Если не установлен, то все углы обрабатываются одинаковым образом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("custom")] public Bool _custom;
        Bool _custom = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _customSpecified
        bool getCustomSpecified() const;

        /// <summary>
        /// Флаг обработки левого нижнего угла прямоугольной контактной площадки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("cornerLB")] public Bool _cornerLB;
        Bool _cornerLB = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _cornerLBSpecified
        bool getCornerLBSpecified() const;

        /// <summary>
        /// Флаг обработки правого нижнего угла прямоугольной контактной площадки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("cornerRB")] public Bool _cornerRB;
        Bool _cornerRB = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _cornerRBSpecified
        bool getCornerRBSpecified() const;

        /// <summary>
        /// Флаг обработки правого нижнего угла прямоугольной контактной площадки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("cornerRT")] public Bool _cornerRT;
        Bool _cornerRT = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _cornerRTSpecified
        bool getCornerRTSpecified() const;

        /// <summary>
        /// Флаг обработки левого верхнего угла прямоугольной контактной площадки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("cornerLT")] public Bool _cornerLT;
        Bool _cornerLT = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _cornerLTSpecified
        bool getCornerLTSpecified() const;

        /// <summary>
        /// Параметр контактной площадки: смещение точки привязки по осям x и y.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Shift")] public Shift _Shift;
        Shift* _Shift;
    };

    /// <summary>
    /// Описание полигональной контактной площадки.
    /// </summary>
public:
    class PadPoly : public BasePad {
        /// <summary>
        /// Массив координат точек, вершин.
        /// </summary>
        /// <remarks>! Минимум 3 элемента</remarks>
    public:
        // ORIGINAL LINE: [XmlElement("Dot")] public List<Dot> _Dots;
        std::vector<Dot*> _Dots;
        bool ShouldSerialize_Dots();
    };

    /// <summary>
    /// Описание стека контактных площадок.
    /// </summary>
public:
    class Padstack {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Тип стека контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("type")] public type_padstack _type;
        type_padstack _type = static_cast<type_padstack>(0);

        /// <summary>
        /// Диаметр отверстия.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("holeDiameter", DataType = "float")] public float _holeDiameter;
        float _holeDiameter = 0.0F;

        /// <summary>
        /// Параметр стека контактной площадки: металлизация отверстия.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("metallized")] public Bool _metallized;
        Bool _metallized = static_cast<Bool>(0);
        virtual ~Padstack() {
            delete _Thermal;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _metallizedSpecified
        bool getMetallizedSpecified() const;

        /// <summary>
        /// Параметр стека контактной площадки: подключение к области металлизации (полигону).
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("connectToCopper")] public type_connectToCopper _connectToCopper;
        type_connectToCopper _connectToCopper = static_cast<type_connectToCopper>(0);

        /// <summary>
        /// Описание термобарьера.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Thermal")] public Thermal _Thermal;
        Thermal* _Thermal;

        /// <summary>
        /// Контактные площадки стека.
        /// </summary>
        /// <value>PadCircle, PadOval, PadRect, PadPoly</value>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("Pads")][XmlArrayItem("PadCircle", typeof(PadCircle)), XmlArrayItem("PadOval", typeof(PadOval)), XmlArrayItem("PadRect", typeof(PadRect)), XmlArrayItem("PadPoly", typeof(PadPoly))] public List<Object> _Pads;
        std::vector<std::any> _Pads;
        bool ShouldSerialize_Pads();
    };

    /// <summary>
    /// Описание типа (стека) переходного отверстия.
    /// </summary>
public:
    class Viastack {
        /// <summary>
        /// Диапазон слоев.
        /// </summary>
        /// <value>AllLayers | [LayerRef]</value>
    public:
        class LayerRange {
            /// <summary>
            /// AllLayers - yстанавливает область действия правила: все слои. См. также _LayerRefs
            /// </summary>
            /// <remarks>! При null необходимо смотреть _LayersRefs - там описан список ссылок типа LayerRef. </remarks>
        public:
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("AllLayers", typeof(AllLayers))] public AllLayers _AllLayers;
            AllLayers* _AllLayers;

            /// <summary>
            /// Диапазон слоёв. См. также _AllLayers
            /// </summary>
            /// <remarks>! При null необходимо смотреть наличие _AllLayers. </remarks>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("LayerRef", typeof(LayerRef))] public List<LayerRef> _LayerRefs;
            std::vector<LayerRef*> _LayerRefs;
            virtual ~LayerRange() {
                delete _AllLayers;
            }

            bool ShouldSerialize_LayerRefs();
        };

        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Диаметр отверстия.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("holeDiameter", DataType = "float")] public float _holeDiameter;
        float _holeDiameter = 0.0F;

        /// <summary>
        /// Параметр типа переходного отверстия: возможность установить переходное отверстие на контактной площадке.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("viaOnPin")] public Bool _viaOnPin;
        Bool _viaOnPin = static_cast<Bool>(0);
        virtual ~Viastack() {
            delete _LayerRange;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _viaOnPinSpecified
        bool getViaOnPinSpecified() const;

        /// <summary>
        /// Диапазон слоев.
        /// </summary>
        /// <value>AllLayers | [LayerRef]</value>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("LayerRange", typeof(LayerRange))] public LayerRange _LayerRange;
        LayerRange* _LayerRange;

        /// <summary>
        /// Описание площадок стека переходного отверстия.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("ViaPads")][XmlArrayItem("PadCircle", typeof(PadCircle))] public List<PadCircle> _ViaPads;
        std::vector<PadCircle*> _ViaPads;
        bool ShouldSerialize_ViaPads();
    };

    /// <summary>
    /// Описание посадочного места.
    /// </summary>
public:
    class Footprint {
        /// <summary>
        /// Описание области металлизации (полигона) в посадочном месте компонента.
        /// </summary>
    public:
        class Copper_Footprint {
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
            /// <value>ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon</value>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("ArcCCW", typeof(ArcCCW)), XmlElement("ArcCW", typeof(ArcCW)), XmlElement("ArcByAngle", typeof(ArcByAngle)), XmlElement("ArcByMiddle", typeof(ArcByMiddle)), XmlElement("Line", typeof(Line)), XmlElement("Circle", typeof(Circle)), XmlElement("Rect", typeof(Rect)), XmlElement("FilledCircle", typeof(FilledCircle)), XmlElement("FilledRect", typeof(FilledRect)), XmlElement("Polygon", typeof(Polygon)), XmlElement("FilledContour", typeof(FilledContour))] public Object _Figure;
            std::any _Figure;
            virtual ~Copper_Footprint() {
                delete _LayerRef;
            }
        };

        /// <summary>
        /// Описание запрета в посадочном месте Footprint. Для запрета размещения должен быть указан слой с типом Assy.
        /// </summary>
    public:
        class Keepout_Place_Trace {
            /// <summary>
            /// Ссылка на слой.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
            LayerRef* _LayerRef;

            /// <summary>
            /// Описание фигуры.
            /// </summary>
            /// <value>ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Line, Circle, Rect, FilledCircle, FilledRect, Polygon</value>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("ArcCCW", typeof(ArcCCW)), XmlElement("ArcCW", typeof(ArcCW)), XmlElement("ArcByAngle", typeof(ArcByAngle)), XmlElement("ArcByMiddle", typeof(ArcByMiddle)), XmlElement("Line", typeof(Line)), XmlElement("Circle", typeof(Circle)), XmlElement("Rect", typeof(Rect)), XmlElement("FilledCircle", typeof(FilledCircle)), XmlElement("FilledRect", typeof(FilledRect)), XmlElement("Polygon", typeof(Polygon)), XmlElement("FilledContour", typeof(FilledContour))] public Object _Figure;
            std::any _Figure;
            virtual ~Keepout_Place_Trace() {
                delete _LayerRef;
            }
        };

        /// <summary>
        /// Описание монтажного отверстия в посадочном месте.
        /// </summary>
    public:
        class Mnthole {
            /// <summary>
            /// Идентификатор неименованных объектов.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("id")] public string _id;
            QString _id;

            /// <summary>
            /// Ссылка на стек контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("PadstackRef")] public PadstackRef _PadstackRef;
            PadstackRef* _PadstackRef;

            /// <summary>
            /// Точка привязки объекта.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
            Org* _Org;
            virtual ~Mnthole() {
                delete _PadstackRef;
                delete _Org;
            }
        };

        /// <summary>
        /// Описание ярлыка в посадочном месте.
        /// </summary>
    public:
        class Label_Footprint {
            /// <summary>
            /// Имя объекта или ссылка на именованный объект.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
            QString _name;

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
            virtual ~Label_Footprint() {
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
        };

        /// <summary>
        /// Описание контактной площадки (вывода) посадочного места.
        /// </summary>
        /// <remarks>! В системе TopoR поддерживаются планарные контакты на внешних металлических слоях и не поддерживаются на внутренних.
        /// Т.е.у планарного контакта может быть только одна площадка или на верхней стороне, или на нижней.
        /// В описании планарного контакта используется только слой Top.
        /// Это означает, что контактная площадка будет находиться на одной стороне с компонентом.
        /// Если же площадка находится на противоположной стороне, то должен быть установлен флаг flipped.
        /// Этот флаг устанавливается в описании контакта посадочного места.
        /// </remarks>
    public:
        class Pad {
            /// <summary>
            /// Номер контактной площадки (вывода) посадочного места.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("padNum", DataType = "int")] public int _padNum;
            int _padNum = 0;

            /// <summary>
            /// Имя объекта или ссылка на именованный объект.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
            QString _name;

            /// <summary>
            /// Задаёт угол в градусах c точностью до тысячных долей.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
            float _angle = 0.0F;

            /// <summary>
            /// Параметр контакта (вывода) посадочного места: перевёрнутость.
            /// Если флаг не установлен, площадка планарного контакта будет находиться на одной стороне с компонентом,
            /// иначе площадка будет расположена на противоположной стороне.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("flipped")] public Bool _flipped;
            Bool _flipped = static_cast<Bool>(0);
            virtual ~Pad() {
                delete _PadstackRef;
                delete _Org;
            }

            // ORIGINAL LINE: [XmlIgnore] public bool _flippedSpecified
            bool getFlippedSpecified() const;

            /// <summary>
            /// Ссылка на стек контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("PadstackRef")] public PadstackRef _PadstackRef;
            PadstackRef* _PadstackRef;

            /// <summary>
            /// Точка привязки объекта.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
            Org* _Org;
        };

        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Описание контактных площадок посадочного места.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Pads")][XmlArrayItem("Pad")] public List<Pad> _Pads;
        std::vector<Pad*> _Pads;
        bool ShouldSerialize_Pads();
        /// <summary>
        /// Надписи.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Texts")][XmlArrayItem("Text")] public List<Text> _Texts;
        std::vector<Text*> _Texts;
        bool ShouldSerialize_Texts();
        /// <summary>
        /// Детали посадочного места.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Details")][XmlArrayItem("Detail")] public List<Detail> _Details;
        std::vector<Detail*> _Details;
        bool ShouldSerialize_Details();
        /// <summary>
        /// Области металлизации (полигонов) в посадочных местах компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Coppers")][XmlArrayItem("Copper")] public List<Copper_Footprint> _Coppers;
        std::vector<Copper_Footprint*> _Coppers;
        bool ShouldSerialize_Coppers();
        /// <summary>
        /// Запреты размещения в посадочном месте.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("KeepoutsPlace")][XmlArrayItem("Keepout")] public List<Keepout_Place_Trace> _KeepoutsPlace;
        std::vector<Keepout_Place_Trace*> _KeepoutsPlace;
        bool ShouldSerialize_KeepoutsPlace();
        /// <summary>
        /// Запреты трассировки в посадочном месте.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("KeepoutsTrace")][XmlArrayItem("Keepout")] public List<Keepout_Place_Trace> _KeepoutsTrace;
        std::vector<Keepout_Place_Trace*> _KeepoutsTrace;
        bool ShouldSerialize_KeepoutsTrace();
        /// <summary>
        /// Монтажные отверстия.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Mntholes")][XmlArrayItem("Mnthole")] public List<Mnthole> _Mntholes;
        std::vector<Mnthole*> _Mntholes;
        bool ShouldSerialize_Mntholes();
        /// <summary>
        /// Ярлыки.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Labels")][XmlArrayItem("Label")] public List<Label_Footprint> _Labels;
        std::vector<Label_Footprint*> _Labels;
        bool ShouldSerialize_Labels();

        QString ToString();
    };

    /// <summary>
    /// Описание схемного компонента.
    /// </summary>
public:
    class Component {
        /// <summary>
        /// Описание контакта схемного компонента.
        /// </summary>
    public:
        class Pin_Component {
            /// <summary>
            /// Номер контакта компонента.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("pinNum", DataType = "int")] public int _pinNum;
            int _pinNum = 0;

            /// <summary>
            /// Имя объекта или ссылка на именованный объект.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
            QString _name;

            /// <summary>
            /// Схемотехническое имя контакта компонента.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("pinSymName")] public string _pinSymName;
            QString _pinSymName;

            /// <summary>
            /// Параметр контакта компонента: эквивалентность.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("pinEqual", DataType = "int")] public int _pinEqual;
            int _pinEqual = 0;

            /// <summary>
            /// Параметр контакта (вывода) компонента: номер вентиля контакта.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("gate", DataType = "int")] public int _gate;
            int _gate = 0;

            /// <summary>
            /// Параметр контакта (вывода) компонента: эквивалентность вентиля контакта.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("gateEqual", DataType = "int")] public int _gateEqual;
            int _gateEqual = 0;
        };

        /// <summary>
        /// Описание атрибута схемного компонента.
        /// </summary>
    public:
        class Attribute_Component {
            /// <summary>
            /// Имя объекта или ссылка на именованный объект.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
            QString _name;

            /// <summary>
            /// Значение атрибута.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("value")] public string _value;
            QString _value;
        };

        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Контакты схемного компонента.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Pins")][XmlArrayItem("Pin")] public List<Pin_Component> _Pins;
        std::vector<Pin_Component*> _Pins;
        bool ShouldSerialize_Pins();
        /// <summary>
        /// Атрибуты компонента.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Attributes")][XmlArrayItem("Attribute")] public List<Attribute_Component> _Attributes;
        std::vector<Attribute_Component*> _Attributes;
        bool ShouldSerialize_Attributes();

        QString ToString();
    };

    /// <summary>
    /// Описание упаковки (соответствие контактов компонента и выводов посадочного места).
    /// </summary>
public:
    class Package {
        /// <summary>
        /// Соответствие контакта схемного компонента и вывода посадочного места.
        /// </summary>
    public:
        class Pinpack {
            /// <summary>
            /// Номер контакта компонента.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("pinNum", DataType = "int")] public int _pinNum;
            int _pinNum = 0;

            /// <summary>
            /// Номер контактной площадки (вывода) посадочного места.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("padNum", DataType = "int")] public int _padNum;
            int _padNum = 0;

            /// <summary>
            /// Параметр правил выравнивания задержек: тип значений констант и допусков.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("valueType")] public valueType _valueType;
            valueType _valueType = static_cast<valueType>(0);

            /// <summary>
            /// Параметр контакта компонента в посадочном месте: задержка сигнала в посадочном месте.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("delay", DataType = "float")] public float _delay;
            float _delay = 0.0F;
        };

        /// <summary>
        /// Ссылка на схемный компонент.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlElement("ComponentRef")] public ComponentRef _ComponentRef;
        ComponentRef* _ComponentRef;

        /// <summary>
        /// Ссылка на посадочное место.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("FootprintRef")] public FootprintRef _FootprintRef;
        FootprintRef* _FootprintRef;

        /// <summary>
        /// Соответствие контакта схемного компонента и вывода посадочного места.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Pinpack")] public List<Pinpack> _Pinpacks;
        std::vector<Pinpack*> _Pinpacks;
        virtual ~Package() {
            delete _ComponentRef;
            delete _FootprintRef;
        }

        bool ShouldSerialize_Pinpacks();
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Стеки контактных площадок.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Padstacks")][XmlArrayItem("Padstack")] public List<Padstack> _Padstacks;
    std::vector<Padstack*> _Padstacks;
    bool ShouldSerialize_Padstacks();
    /// <summary>
    /// Типы (стеки) переходных отверстий.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Viastacks")][XmlArrayItem("Viastack")] public List<Viastack> _Viastacks;
    std::vector<Viastack*> _Viastacks;
    bool ShouldSerialize_Viastacks();
    /// <summary>
    /// Посадочные места.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Footprints")][XmlArrayItem("Footprint")] public List<Footprint> _Footprints;
    std::vector<Footprint*> _Footprints;
    bool ShouldSerialize_Footprints();
    /// <summary>
    /// Схемные компоненты.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Components")][XmlArrayItem("Component")] public List<Component> _Components;
    std::vector<Component*> _Components;
    bool ShouldSerialize_Components();
    /// <summary>
    /// Упаковки.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Packages")][XmlArrayItem("Package")] public List<Package> _Packages;
    std::vector<Package*> _Packages;
    bool ShouldSerialize_Packages();

    /************************************************************************
     * Здесь находятся функции для работы с элементами класса LocalLibrary. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/

    /************************************************************************/
};
// } // namespace TopoR_PCB_Classes

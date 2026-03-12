#pragma once

#include "Commons.h"
#include <any>
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе u"Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г."_s.
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Описание конструктива платы.
/// </summary>
class Constructive : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Описание контура платы и вырезов в плате.
    /// </summary>
public:
    class BoardOutline {
    public:
        class Shape_Contour {

            /// <summary>
            /// Толщина линии.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute(u"lineWidth"_s, DataType = u"float"_s)] public float _lineWidth;
            float _lineWidth = 0.0F;

            /// <summary>
            /// Незалитая фигура.
            /// </summary>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement(u"ArcCCW"_s, typeof(ArcCCW)), XmlElement(u"ArcCW"_s, typeof(ArcCW)), XmlElement(u"ArcByAngle"_s, typeof(ArcByAngle)), XmlElement(u"ArcByMiddle"_s, typeof(ArcByMiddle)), XmlElement(u"Circle"_s, typeof(Circle)), XmlElement(u"Line"_s, typeof(Line)), XmlElement(u"Polyline"_s, typeof(Polyline)), XmlElement(u"Rect"_s, typeof(Rect)), XmlElement(u"Contour"_s, typeof(Contour))] public Object _NonfilledFigure;

            std::variant<ArcCCW, ArcCW, ArcByAngle, ArcByMiddle, Circle, Line, Polyline, Rect, Contour> _NonfilledFigure;

            /*************************************************************************
             * Здесь находятся функции для работы с элементами класса Shape_Contour. *
             * Они не являются частью формата TopoR PCB.                             *
             * ***********************************************************************/
            void Shift(float x, float y);
            void UnitsConvert(dist_ in_units, dist_ out_units);
            /*************************************************************************/
        };

    public:
        class Shape_Voids {

            /// <summary>
            /// Толщина линии.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute(u"lineWidth"_s, DataType = u"float"_s)] public float _lineWidth;
            float _lineWidth = 0.0F;

            /// <summary>
            /// Описание залитой фигуры.
            /// </summary>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement(u"FilledCircle"_s, typeof(FilledCircle)), XmlElement(u"FilledRect"_s, typeof(FilledRect)), XmlElement(u"Polygon"_s, typeof(Polygon)), XmlElement(u"FilledContour"_s, typeof(FilledContour))] public Object _FilledFigure;

            std::variant<FilledCircle, FilledRect, Polygon, FilledContour> _FilledFigure;

            /**********************************************************************
             * Здесь находятся функции для работы с элементами класса Shape_Voids. *
             * Они не являются частью формата TopoR PCB.                           *
             * *********************************************************************/
            void Shift(float x, float y);
            void UnitsConvert(dist_ in_units, dist_ out_units);
            /***********************************************************************/
        };

        /// <summary>
        /// Описание контура платы.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlArray(u"Contour"_s)][XmlArrayItem(u"Shape"_s)] public List<Shape_Contour> _Contours;
        std::vector<Shape_Contour*> _Contours;
        bool ShouldSerialize_Contours();
        /// <summary>
        /// Вырезы в плате.
        /// </summary>

        // ORIGINAL LINE: [XmlArray(u"Voids"_s)][XmlArrayItem(u"Shape"_s)] public List<Shape_Voids> _Voids;
        std::vector<Shape_Voids*> _Voids;
        bool ShouldSerialize_Voids();
    };

    /// <summary>
    /// Описание монтажного отверстия на плате.
    /// </summary>
public:
    class MntholeInstance {
        /// <summary>
        /// Задаёт угол в градусах c точностью до тысячных долей.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"angle"_s, DataType = u"float"_s)] public float _angle;
        float _angle = 0.0F;

        /// <summary>
        /// Признак фиксации.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"fixed"_s)] public Bool _fixed;
        Bool _fixed = static_cast<Bool>(0);
        virtual ~MntholeInstance() {
            delete _PadstackRef;
            delete _NetRef;
            delete _Org;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _fixedSpecified
        bool getFixedSpecified() const;

        /// <summary>
        /// Ссылка на стек контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"PadstackRef"_s)] public PadstackRef _PadstackRef;
        PadstackRef* _PadstackRef;

        /// <summary>
        /// ссылка на цепь.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"NetRef"_s)] public NetRef _NetRef;
        NetRef* _NetRef;

        /// <summary>
        /// Точка привязки объекта.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"Org"_s)] public Org _Org;
        Org* _Org;

        void Shift(float x, float y);
        void UnitsConvert(dist_ in_units, dist_ out_units);
    };
    /// <summary>
    /// Описание запрета.
    /// </summary>
public:
    class Keepout_Сonstructive {
        /// <summary>
        /// Тип запрета.
        /// </summary>
    public:
        class Role {
            // <summary>
            /// Тип запрета: запрет трассировки.
            /// </summary>
        public:
            class Trace {
                /// <summary>
                /// Тип запрета трассировки.
                /// </summary>
            public:
                // ORIGINAL LINE: [XmlAttribute(u"role"_s)] public role _role;
                role _role = static_cast<role>(0);

                /// <summary>
                /// Ссылка на слои. См. также _LayersRefs
                /// </summary>
                /// <remarks>! При null необходимо смотреть _LayersRefs - там описан список ссылок типа LayerRef. </remarks>
                // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

                // ORIGINAL LINE: [XmlElement(u"AllLayers"_s, typeof(AllLayers)), XmlElement(u"AllLayersInner"_s, typeof(AllLayersInner)), XmlElement(u"AllLayersInnerSignal"_s, typeof(AllLayersInnerSignal)), XmlElement(u"AllLayersSignal"_s, typeof(AllLayersSignal)), XmlElement(u"AllLayersOuter"_s, typeof(AllLayersOuter)), XmlElement(u"LayerGroupRef"_s, typeof(LayerGroupRef))] public Object _LayersRef;
                std::any _LayersRef;
                /// <summary>
                /// Ссылка на слои. См. также _LayersRef
                /// </summary>
                /// <remarks>! При null необходимо смотреть _LayersRef - там описаны ссылки остальных типов. </remarks>

                // ORIGINAL LINE: [XmlElement(u"LayerRef"_s)] public List<LayerRef> _LayersRefs;
                std::vector<LayerRef*> _LayersRefs;
                bool ShouldSerialize_LayersRefs();
            };

            /// <summary>
            /// Тип запрета: запрет размещения.
            /// </summary>
        public:
            class Place {
                /// <summary>
                /// Сторона объекта.
                /// </summary>
            public:
                // ORIGINAL LINE: [XmlAttribute(u"side"_s)] public side _side;
                side _side = static_cast<side>(0);
            };
            /// <summary>
            /// Тип запрета: запрет трассировки.
            /// </summary>
        public:
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement(u"Trace"_s, typeof(Trace))] public Trace _Trace;
            Trace* _Trace;
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement(u"Place"_s, typeof(Place))] public Place _Place;
            Place* _Place;
            virtual ~Role() {
                delete _Trace;
                delete _Place;
            }
        };

    public:
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement(u"Role"_s, typeof(Role))] public Role _Role;
        Role* _Role;

        /// <summary>
        /// Описание фигуры.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement(u"ArcCCW"_s, typeof(ArcCCW)), XmlElement(u"ArcCW"_s, typeof(ArcCW)), XmlElement(u"ArcByAngle"_s, typeof(ArcByAngle)), XmlElement(u"ArcByMiddle"_s, typeof(ArcByMiddle)), XmlElement(u"Line"_s, typeof(Line)), XmlElement(u"Circle"_s, typeof(Circle)), XmlElement(u"Rect"_s, typeof(Rect)), XmlElement(u"FilledCircle"_s, typeof(FilledCircle)), XmlElement(u"FilledRect"_s, typeof(FilledRect)), XmlElement(u"Polygon"_s, typeof(Polygon)), XmlElement(u"Contour"_s, typeof(Contour)), XmlElement(u"FilledContour"_s, typeof(FilledContour)), XmlElement(u"Polyline"_s, typeof(Polyline))] public Object _FigureContPolyline;
        std::variant<ArcCCW,
            ArcCW,
            ArcByAngle,
            ArcByMiddle,
            Line,
            Circle,
            Rect,
            FilledCircle,
            FilledRect,
            Polygon,
            Contour,
            FilledContour,
            Polyline>
            _FigureContPolyline;

        /********************************************************************************
         * Здесь находятся функции для работы с элементами класса Keepout_Сonstructive. *
         * Они не являются частью формата TopoR PCB.                                    *
         * ******************************************************************************/
        virtual ~Keepout_Сonstructive() {
            delete _Role;
        }

        void Shift(float x, float y);
        void UnitsConvert(dist_ in_units, dist_ out_units);
        /********************************************************************************/
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute(u"version"_s)] public string _version;
    QString _version;

    /// <summary>
    /// Контур платы и вырезы в плате.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"BoardOutline"_s)] public BoardOutline _BoardOutline;
    BoardOutline* _BoardOutline;

    /// <summary>
    /// Монтажные отверстия на плате.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"Mntholes"_s), DefaultValue(null)][XmlArrayItem(u"MntholeInstance"_s)] public List<MntholeInstance> _Mntholes;
    std::vector<MntholeInstance*> _Mntholes;
    virtual ~Constructive() {
        delete _BoardOutline;
    }

    bool ShouldSerialize_Mntholes();
    /// <summary>
    /// Детали на механических слоях.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"MechLayerObjects"_s), DefaultValue(null)][XmlArrayItem(u"Detail"_s)] public List<Detail> _MechLayerObjects;
    std::vector<Detail*> _MechLayerObjects;
    bool ShouldSerialize_MechLayerObjects();
    /// <summary>
    /// Описание надписей.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"Texts"_s), DefaultValue(null)][XmlArrayItem(u"Text"_s)] public List<Text> _Texts;
    std::vector<Text*> _Texts;
    bool ShouldSerialize_Texts();
    /// <summary>
    /// Описание запретов.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"Keepouts"_s), DefaultValue(null)][XmlArrayItem(u"Keepout"_s)] public List<Keepout_Сonstructive> _Keepouts;
    std::vector<Keepout_Сonstructive*> _Keepouts;
    bool ShouldSerialize_Keepouts();

    /************************************************************************
     * Здесь находятся функции для работы с элементами класса Сonstructive. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/

    void Shift(float x, float y);

    void UnitsConvert(dist_ in_units, dist_ out_units);

    void Add(Constructive* a, bool boardOutline, bool mntholeInstances, bool details, bool texts, bool keepouts);
    /************************************************************************/
};
// } // namespace TopoR_PCB_Classes

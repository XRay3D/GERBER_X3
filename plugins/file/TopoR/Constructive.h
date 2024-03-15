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
            // ORIGINAL LINE: [XmlAttribute("lineWidth", DataType = "float")] public float _lineWidth;
            float _lineWidth = 0.0F;

            /// <summary>
            /// Незалитая фигура.
            /// </summary>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("ArcCCW", typeof(ArcCCW)), XmlElement("ArcCW", typeof(ArcCW)), XmlElement("ArcByAngle", typeof(ArcByAngle)), XmlElement("ArcByMiddle", typeof(ArcByMiddle)), XmlElement("Circle", typeof(Circle)), XmlElement("Line", typeof(Line)), XmlElement("Polyline", typeof(Polyline)), XmlElement("Rect", typeof(Rect)), XmlElement("Contour", typeof(Contour))] public Object _NonfilledFigure;

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
            // ORIGINAL LINE: [XmlAttribute("lineWidth", DataType = "float")] public float _lineWidth;
            float _lineWidth = 0.0F;

            /// <summary>
            /// Описание залитой фигуры.
            /// </summary>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("FilledCircle", typeof(FilledCircle)), XmlElement("FilledRect", typeof(FilledRect)), XmlElement("Polygon", typeof(Polygon)), XmlElement("FilledContour", typeof(FilledContour))] public Object _FilledFigure;

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
        // ORIGINAL LINE: [XmlArray("Contour")][XmlArrayItem("Shape")] public List<Shape_Contour> _Contours;
        std::vector<Shape_Contour*> _Contours;
        bool ShouldSerialize_Contours();
        /// <summary>
        /// Вырезы в плате.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Voids")][XmlArrayItem("Shape")] public List<Shape_Voids> _Voids;
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
        // ORIGINAL LINE: [XmlAttribute("angle", DataType = "float")] public float _angle;
        float _angle = 0.0F;

        /// <summary>
        /// Признак фиксации.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("fixed")] public Bool _fixed;
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

        // ORIGINAL LINE: [XmlElement("PadstackRef")] public PadstackRef _PadstackRef;
        PadstackRef* _PadstackRef;

        /// <summary>
        /// ссылка на цепь.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("NetRef")] public NetRef _NetRef;
        NetRef* _NetRef;

        /// <summary>
        /// Точка привязки объекта.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Org")] public Org _Org;
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
                // ORIGINAL LINE: [XmlAttribute("role")] public role _role;
                role _role = static_cast<role>(0);

                /// <summary>
                /// Ссылка на слои. См. также _LayersRefs
                /// </summary>
                /// <remarks>! При null необходимо смотреть _LayersRefs - там описан список ссылок типа LayerRef. </remarks>
                // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

                // ORIGINAL LINE: [XmlElement("AllLayers", typeof(AllLayers)), XmlElement("AllLayersInner", typeof(AllLayersInner)), XmlElement("AllLayersInnerSignal", typeof(AllLayersInnerSignal)), XmlElement("AllLayersSignal", typeof(AllLayersSignal)), XmlElement("AllLayersOuter", typeof(AllLayersOuter)), XmlElement("LayerGroupRef", typeof(LayerGroupRef))] public Object _LayersRef;
                std::any _LayersRef;
                /// <summary>
                /// Ссылка на слои. См. также _LayersRef
                /// </summary>
                /// <remarks>! При null необходимо смотреть _LayersRef - там описаны ссылки остальных типов. </remarks>

                // ORIGINAL LINE: [XmlElement("LayerRef")] public List<LayerRef> _LayersRefs;
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
                // ORIGINAL LINE: [XmlAttribute("side")] public side _side;
                side _side = static_cast<side>(0);
            };
            /// <summary>
            /// Тип запрета: запрет трассировки.
            /// </summary>
        public:
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("Trace", typeof(Trace))] public Trace _Trace;
            Trace* _Trace;
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("Place", typeof(Place))] public Place _Place;
            Place* _Place;
            virtual ~Role() {
                delete _Trace;
                delete _Place;
            }
        };

    public:
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("Role", typeof(Role))] public Role _Role;
        Role* _Role;

        /// <summary>
        /// Описание фигуры.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("ArcCCW", typeof(ArcCCW)), XmlElement("ArcCW", typeof(ArcCW)), XmlElement("ArcByAngle", typeof(ArcByAngle)), XmlElement("ArcByMiddle", typeof(ArcByMiddle)), XmlElement("Line", typeof(Line)), XmlElement("Circle", typeof(Circle)), XmlElement("Rect", typeof(Rect)), XmlElement("FilledCircle", typeof(FilledCircle)), XmlElement("FilledRect", typeof(FilledRect)), XmlElement("Polygon", typeof(Polygon)), XmlElement("Contour", typeof(Contour)), XmlElement("FilledContour", typeof(FilledContour)), XmlElement("Polyline", typeof(Polyline))] public Object _FigureContPolyline;
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
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Контур платы и вырезы в плате.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("BoardOutline")] public BoardOutline _BoardOutline;
    BoardOutline* _BoardOutline;

    /// <summary>
    /// Монтажные отверстия на плате.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Mntholes"), DefaultValue(null)][XmlArrayItem("MntholeInstance")] public List<MntholeInstance> _Mntholes;
    std::vector<MntholeInstance*> _Mntholes;
    virtual ~Constructive() {
        delete _BoardOutline;
    }

    bool ShouldSerialize_Mntholes();
    /// <summary>
    /// Детали на механических слоях.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("MechLayerObjects"), DefaultValue(null)][XmlArrayItem("Detail")] public List<Detail> _MechLayerObjects;
    std::vector<Detail*> _MechLayerObjects;
    bool ShouldSerialize_MechLayerObjects();
    /// <summary>
    /// Описание надписей.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Texts"), DefaultValue(null)][XmlArrayItem("Text")] public List<Text> _Texts;
    std::vector<Text*> _Texts;
    bool ShouldSerialize_Texts();
    /// <summary>
    /// Описание запретов.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("Keepouts"), DefaultValue(null)][XmlArrayItem("Keepout")] public List<Keepout_Сonstructive> _Keepouts;
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

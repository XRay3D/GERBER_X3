#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {
// Описание конструктива платы.
struct Constructive {
    // Описание контура платы и вырезов в плате.
    struct BoardOutline {
#if 0
        struct ShapeContour {
            // Толщина линии.
            // [Xml::Attribute("lineWidth", DataType = "double")] public double lineWidth_;
            Xml::Attr<double> lineWidth;
            // Незалитая фигура.
            // [Xml::Element(ArcCCW),
            //  Xml::Element(ArcCW),
            //  Xml::Element(ArcByAngle),
            //  Xml::Element(ArcByMiddle),
            //  Xml::Element(Circle),
            //  Xml::Element(Line),
            //  Xml::Element(Polyline),
            //  Xml::Element(Rect),
            //  Xml::Element(Contour)] public Object NonfilledFigure_;
            Xml::Variant<
                ArcCCW,
                ArcCW,
                ArcByAngle,
                ArcByMiddle,
                Circle,
                Line,
                Polyline,
                Rect,
                Contour>
                NonfilledFigure_;
            /*************************************************************************
             * Здесь находятся функции для работы с элементами класса Shape_Contour. *
             * Они не являются частью формата TopoR PCB.                             *
             * ***********************************************************************/

            //    void UnitsConvert(dist in_units, dist out_units);
            /*************************************************************************/
        };
        struct ShapeVoids {
            // Толщина линии.
            // [Xml::Attribute("lineWidth", DataType = "double")] public double lineWidth_;
            Xml::Attr<double> lineWidth;
            // Описание залитой фигуры.
            // [Xml::Element(FilledCircle),
            //  Xml::Element(FilledRect),
            //  Xml::Element(Polygon),
            //  Xml::Element(FilledContour)] public Object FilledFigure_;
            Xml::Variant<FilledCircle, FilledRect, Polygon, FilledContour> FilledFigure_;
            /**********************************************************************
             * Здесь находятся функции для работы с элементами класса Shape_Voids. *
             * Они не являются частью формата TopoR PCB.                           *
             * *********************************************************************/

            //    void UnitsConvert(dist in_units, dist out_units);
            /***********************************************************************/
        };
        // Описание контура платы.
        // [Xml::Array("Contour")][Xml::ArrayItem("Shape")] public List<Shape_Contour> Contours_;
        Xml::ArrayElem<ShapeContour> Contours;
        // Вырезы в плате.
        // [Xml::Array("Voids")][Xml::ArrayItem("Shape")] public List<Shape_Voids> Voids_;
        Xml::ArrayElem<ShapeVoids> Voids;
#else
        struct Shape {
            // Толщина линии.
            Xml::Attr<double> lineWidth;
            // Незалитая фигура.
            Xml::Variant<
                ArcCCW,      // Contour
                ArcCW,       // Contour
                ArcByAngle,  // Contour
                ArcByMiddle, // Contour
                Circle,      // Contour
                Line,        // Contour
                Polyline,    // Contour
                Rect,        // Contour
                Contour>     // Contour
                NonfilledFigure;
            // Описание залитой фигуры.
            Xml::Variant<
                FilledCircle,  // Voids
                FilledRect,    // Voids
                Polygon,       // Voids
                FilledContour> // Voids
                FilledFigure;
            /*************************************************************************
             * Здесь находятся функции для работы с элементами класса Shape_Contour. *
             * Они не являются частью формата TopoR PCB.                             *
             * ***********************************************************************/

            //    void UnitsConvert(dist in_units, dist out_units);
            /*************************************************************************/
        };
        // Описание контура платы.
        Xml::ArrayElem<Shape> Contour_;
        // Вырезы в плате.
        Xml::ArrayElem<Shape> Voids;
#endif
    };

    // Описание монтажного отверстия на плате.
    struct MntholeInstance {
        // Задаёт угол в градусах c точностью до тысячных долей.
        Xml::Attr<double> angle;
        // Признак фиксации.
        Xml::Attr<Bool> fixed;
        // Ссылка на стек контактных площадок.
        PadstackRef padstackRef;
        // ссылка на цепь.
        NetRef netRef;
        // Точка привязки объекта.
        Org org;

        //    void UnitsConvert(dist in_units, dist out_units);
    };

    // Описание запрета.
    struct Keepout {
        // Тип запрета.
        struct Role {
            // Тип запрета: запрет трассировки.
            struct Trace {
                // Тип запрета трассировки.
                Xml::Attr<role> role_;
                // Ссылка на слои. См. также LayersRefs_
                /// \note !При null необходимо смотреть LayersRefs_ - там описан список ссылок типа LayerRef.
                Xml::Variant<
                    AllLayers,
                    AllLayersInner,
                    AllLayersInnerSignal,
                    AllLayersSignal,
                    AllLayersOuter,
                    LayerGroupRef>
                    LayersRef;
                // Ссылка на слои. См. также LayersRef_
                /// \note !При null необходимо смотреть LayersRef_ - там описаны ссылки остальных типов.
                Xml::Array<LayerRef> LayersRefs;
            };
            // Тип запрета: запрет размещения.
            struct Place {
                // Сторона объекта.
                Xml::Attr<side> side_;
            };
            // Тип запрета: запрет трассировки.
            Trace trace;
            // Place place;
            Xml::Attr<side> Place;
        };

        //
        Role role_;
        // Описание фигуры.
        Xml::Variant<
            ArcCCW,
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
            FigureContPolyline;
        /********************************************************************************
         * Здесь находятся функции для работы с элементами класса Keepout_Сonstructive. *
         * Они не являются частью формата TopoR PCB.                                    *
         * ******************************************************************************/

        //    void UnitsConvert(dist in_units, dist out_units);
        /********************************************************************************/
    };
    // Версия раздела.
    Xml::Attr<QString> version;
    // Контур платы и вырезы в плате.
    BoardOutline boardOutline;
    // Монтажные отверстия на плате.
    Xml::ArrayElem<MntholeInstance> Mntholes;
    // Детали на механических слоях.
    Xml::ArrayElem<Detail> MechLayerObjects;
    // Описание надписей.
    Xml::ArrayElem<Text> Texts;
    // Описание запретов.
    Xml::ArrayElem<Keepout> Keepouts;
    /************************************************************************
     * Здесь находятся функции для работы с элементами класса Сonstructive. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/

    //    void UnitsConvert(dist in_units, dist out_units);
    //    void Add(Constructive a, bool boardOutline, bool mntholeInstances, bool details, bool texts, bool keepouts);
    /************************************************************************/
};

} // namespace TopoR

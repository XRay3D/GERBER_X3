#pragma once
#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Настройки отображения».
struct DisplayControl {
    // Настройка отображения: параметры текущего вида.
    struct View {
        // Параметр текущего вида: масштаб.
        Xml::Attr<double> scale;
        // Параметр текущего вида: прокрутка по горизонтали.
        Xml::Attr<double> scrollHorz;
        // Параметр текущего вида: прокрутка по вертикали.
        Xml::Attr<double> scrollVert;
    };
    // Устанавливает активный слой.
    struct ActiveLayer {
        /* Опечатка в спецификации?
            // Тип слоя.
            [Xml::Attribute("type")]
        Xml::Attr<LayerType> type;*/
        // Наименование слоя.
        Xml::Attr<QString> name;
    };
    // Настройка отображения: единицы измерения.
    struct Units {
        // Настройка отображения: единицы измерения.
        Xml::Attr<preference, NoOpt> preference_;
    };
    // Настройка отображения: общие цветовые настройки.
    struct Colors {
        // Настройка отображения: текущая цветовая схема.
        Xml::Attr<QString> colorScheme;
        // Настройка отображения: яркость выделенных объектов.
        Xml::Attr<int> hilightRate;
        // Настройка отображения: степень затемнения невыделенных объектов.
        Xml::Attr<int> darkRate;
        // Настройка отображения: цвет фона.
        Xml::Attr<QString> background;
        // Настройка отображения: цвет контура платы.
        Xml::Attr<QString> board;
        // Настройка отображения: цвет линий связей.
        Xml::Attr<QString> netLines;
        // Настройка отображения: цвет запрета размещения на обеих сторонах платы.
        Xml::Attr<QString> keepoutPlaceBoth;
        // Настройка отображения: цвет запрета трассировки на всех слоях.
        Xml::Attr<QString> keepoutWireAll;
        // Настройка отображения: цвет запрета размещения на верхней стороне платы.
        Xml::Attr<QString> keepoutPlaceTop;
        // Настройка отображения: цвет запрета размещения на нижней стороне платы.
        Xml::Attr<QString> keepoutPlaceBot;
        // Настройка отображения: цвет габаритов компонентов.
        Xml::Attr<QString> compsBound;
        // Настройка отображения: цвет позиционных обозначений компонентов.
        Xml::Attr<QString> compsName;
        // Настройка отображения: цвет имён контактов.
        Xml::Attr<QString> pinsName;
        // Настройка отображения: цвет имён цепей контактов.
        Xml::Attr<QString> pinsNet;
        // Настройка отображения: цвет сквозных контактных площадок.
        Xml::Attr<QString> clrThroughPads;
        // Настройка отображения: цвет сквозных переходных отверстий.
        Xml::Attr<QString> clrThroughVias;
        // Настройка отображения: цвет скрытых переходных отверстий.
        Xml::Attr<QString> clrBurriedVias;
        // Настройка отображения: цвет глухих переходных отверстий.
        Xml::Attr<QString> clrBlindVias;
        // Настройка отображения: цвет зафиксированных переходных отверстий.
        Xml::Attr<QString> clrFixedVias;
        // Настройка отображения: цвет нарушений DRC.
        Xml::Attr<QString> drcViolation;
        // Настройка отображения: цвет индикации уменьшения номинального зазора.
        Xml::Attr<QString> narrow;
        // Настройка отображения: цвет индикации уменьшения ширины проводника.
        Xml::Attr<QString> trimmed;
    };
    // Настройка отображения: настройки видимости объектов.
    struct Show {
        // Настройка отображения: показывать глухие переходные отверстия специальным цветом.
        Xml::Attr<Bool> blindVia;
        // Настройка отображения: показывать скрытые переходные отверстия специальным цветом
        Xml::Attr<Bool> burriedVia;
        // Настройка отображения: показывать фиксированные переходные отверстия специальным цветом.
        Xml::Attr<Bool> fixedVia;
        // Настройка отображения: показывать только активный слой.
        Xml::Attr<Bool> showActiveLayerOnly;
        // Настройка отображения: показывать контур платы.
        Xml::Attr<Bool> showBoardOutline;
        // Настройка отображения: показывать детали на нижних металлических слоях.
        Xml::Attr<Bool> showBotMechDetails;
        // Настройка отображения: показывать нижние механические слои.
        Xml::Attr<Bool> showBotMechLayers;
        // Настройка отображения: показывать контактные площадки на нижних металлических слоях.
        Xml::Attr<Bool> showBotMechPads;
        // Настройка отображения: показывать компоненты на нижней стороне.
        Xml::Attr<Bool> showCompBot;
        // Настройка отображения: показывать компоненты на верхней стороне.
        Xml::Attr<Bool> showCompTop;
        // Настройка отображения: показывать компоненты.
        Xml::Attr<Bool> showComponents;
        // Настройка отображения: показывать габариты компонентов.
        Xml::Attr<Bool> showCompsBound;
        // Настройка отображения: показывать позиционные обозначения компонентов.
        Xml::Attr<Bool> showCompsDes;
        // Настройка отображения: показывать области металлизации (полигоны).
        Xml::Attr<Bool> showCoppers;
        // Настройка отображения: показывать нарушение DRC.
        Xml::Attr<Bool> showDRCViolations;
        // Настройка отображения: показывать документирующие слои.
        Xml::Attr<Bool> showDocLayers;
        // Настройка отображения: показывать запреты.
        Xml::Attr<Bool> showKeepouts;
        // Настройка отображения: показывать ярлыки пользовательских атрибутов.
        Xml::Attr<Bool> showLabelOther;
        // Настройка отображения: показывать ярлыки атрибута PartName.
        Xml::Attr<Bool> showLabelPartName;
        // Настройка отображения: показывать ярлыки атрибута RefDes.
        Xml::Attr<Bool> showLabelRefDes;
        // Настройка отображения: показывать контактные площадки на металлических слоях.
        Xml::Attr<Bool> showMetalPads;
        // Настройка отображения: показывать монтажные отверстия.
        Xml::Attr<Bool> showMountingHoles;
        // Настройка отображения: показывать уменьшение номинального зазора.
        Xml::Attr<Bool> showNarrow;
        // Настройка отображения: показывать связи.
        Xml::Attr<Bool> showNetLines;
        // Настройка отображения: показывать имена контактов.
        Xml::Attr<Bool> showPinsName;
        // Настройка отображения: показывать имена цепей контактов.
        Xml::Attr<Bool> showPinsNet;
        // Настройка отображения: показывать запреты размещения.
        Xml::Attr<Bool> showPlaceKeepouts;
        // Настройка отображения: показывать запреты трассировки.
        Xml::Attr<Bool> showRouteKeepouts;
        // Настройка отображения: показывать области змеек.
        Xml::Attr<Bool> showSerpentArea;
        // Настройка отображения: показывать металлические слои.
        Xml::Attr<Bool> showSignalLayers;
        // Настройка отображения: показывать ярлыки (надписи).
        Xml::Attr<Bool> showTexts;
        // Настройка отображения: показывать проводники тонкими линиями.
        Xml::Attr<Bool> showThinWires;
        // Настройка отображения: показывать детали на верхних металлических слоях.
        Xml::Attr<Bool> showTopMechDetails;
        // Настройка отображения: показывать верхние механические слои.
        Xml::Attr<Bool> showTopMechLayers;
        // Настройка отображения: показывать КП на верхних металлических слоях.
        Xml::Attr<Bool> showTopMechPads;
        // Настройка отображения: показывать уменьшение ширины проводника.
        Xml::Attr<Bool> showTrimmed;
        // Настройка отображения: показывать переходы.
        Xml::Attr<Bool> showVias;
        // Настройка отображения: показывать нарушения.
        Xml::Attr<Bool> showViolations;
        // Настройка отображения: показывать проводники.
        Xml::Attr<Bool> showWires;
        // Настройка отображения: показывать сквозные контактные площадки специальным цветом.
        Xml::Attr<Bool> throughPad;
        // Настройка отображения: показывать сквозные переходные отверстия специальным цветом.
        Xml::Attr<Bool> throughVia;
        // Настройка отображения: текущая схема отображения.
        Xml::Attr<QString> displayScheme;
    };
    // Настройки сетки.
    struct Grid {
        // Настройка отображения сетки: шаг сетки.
        struct GridSpace {
            // шаг сетки по горизонтали и вертикали.
            Xml::Attr<double> x, y;
        } gridSpace;
        // Настройка отображения сетки: цвет сетки.
        Xml::Attr<QString> gridColor;
        // Настройка отображения сетки: тип сетки.
        Xml::Attr<gridKind, NoOpt> gridKind_;
        // Настройка отображения сетки: показывать сетку.
        Xml::Attr<Bool> gridShow;
        // Настройка ручного редактора: выравнивание на сетку.
        Xml::Attr<Bool> alignTogrid;
        // Настройка ручного редактирования: привязка к углу кратному 45˚.
        Xml::Attr<Bool> snapToAngle;
        Xml::Attr<Bool> saveProportion;
    };
    // Настройка отображения: настройки видимости слоя.
    struct LayerOptions {
        // Настройка отображения: цветовые настройки слоя.
        struct Colors {
            // Настройка отображения слоя: цвет деталей, проводников (основной цвет слоя).
            Xml::Attr<QString> details;
            // Настройка отображения слоя: цвет контактных площадок.
            Xml::Attr<QString> pads;
            // Настройка отображения слоя: цвет зафиксированных объектов.
            Xml::Attr<QString> fix;
        };
        // Настройка отображения слоя: настройки видимости.
        struct Show {
            // Флаг видимости.
            Xml::Attr<Bool> visible;
            // Настройка отображения слоя: видимость деталей.
            Xml::Attr<Bool> details;
            // Настройка отображения слоя: видимость контактных площадок.
            Xml::Attr<Bool> pads;
        };
        // Ссылка на слой.
        LayerRef layerRef;
        // Настройка отображения: цветовые настройки слоя.
        Colors colors;
        // Настройка отображения слоя: настройки видимости.
        Show show;
    };
    // Отображение цепей особым цветом.
    struct ColorNets {
        // Отображение цепей особым цветом: установить цвет для цепи / сигнала / группы цепей / группы сигналов.
        struct SetColor {
            // Отображение цепей особым цветом: задание цвета.
            Xml::Attr<QString> color;
            // Ссылка на цепь или сигнал
            Xml::Variant<
                NetRef,
                NetGroupRef,
                AllNets,
                SignalRef,
                DiffSignalRef,
                SignalGroupRef>
                Refs;
        };
        // Флаг применения правила.
        Xml::Attr<Bool> enabled;
        // Отображение цепей особым цветом: применять для проводников.
        Xml::Attr<Bool> colorizeWire;
        // Отображение цепей особым цветом: применять для контактных площадок.
        Xml::Attr<Bool> colorizePad;
        // Отображение цепей особым цветом: применять для областей металлизации.
        Xml::Attr<Bool> colorizeCopper;
        // Отображение цепей особым цветом: применять для переходов.
        Xml::Attr<Bool> colorizeVia;
        // Отображение цепей особым цветом: применять для связей.
        Xml::Attr<Bool> colorizeNetline;
        // Отображение цепей особым цветом: установить цвет для цепи / сигнала / группы цепей / группы сигналов.
        Xml::ArrayElem<SetColor> SetColors;
    };
    // Фильтр отображения связей.
    struct FilterNetlines {
        // Флаг применения правила.
        Xml::Attr<Bool, NoOpt> enabled;
        // Ссылки на цепь или сигнал
        Xml::ArrayElem<Xml::Variant<
            AllNets,
            DiffSignalRef,
            NetGroupRef,
            NetRef,
            SignalGroupRef,
            SignalRef>>
            Refs;
    };
    // Версия раздела.
    Xml::Attr<QString> version;
    // Настройка отображения: параметры текущего вида.
    View view;
    // Устанавливает активный слой.
    ActiveLayer activeLayer;
    // Настройка отображения: единицы измерения.
    Units units;
    // Настройка отображения: общие цветовые настройки.
    Colors colors;
    //  Настройка отображения: настройки видимости объектов.
    Show show;
    //  Настройки сетки.
    Grid grid;
    // Настройка отображения: настройки видимости слоёв.
    Xml::ArrayElem<LayerOptions> LayersVisualOptions;
    // Отображение цепей особым цветом.
    ColorNets colorNets;
    // Фильтр отображения связей.
    FilterNetlines filterNetlines;

    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DisplayControl. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/
};

} // namespace TopoR

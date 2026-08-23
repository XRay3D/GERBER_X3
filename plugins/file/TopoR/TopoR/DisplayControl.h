#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Настройки отображения».
struct DisplayControl {
    // Настройка отображения: параметры текущего вида.
    struct View {
        // Параметр текущего вида: масштаб.
        [[= XML::Attr]] double scale{};
        // Параметр текущего вида: прокрутка по горизонтали.
        [[= XML::Attr]] double scrollHorz{};
        // Параметр текущего вида: прокрутка по вертикали.
        [[= XML::Attr]] double scrollVert{};
    };
    // Устанавливает активный слой.
    struct ActiveLayer {
        /* Опечатка в спецификации?    .
            // Тип слоя.
            [XmlAttribute("type")]
            public layertype type;*/
        // Наименование слоя.
        [[= XML::Attr]] std::string name;
    };
    // Настройка отображения: единицы измерения.
    struct Units {
        // Настройка отображения: единицы измерения.
        [[= XML::Attr]] preference preference{};
    };
    // Настройка отображения: общие цветовые настройки.
    struct Colors {
        // Настройка отображения: текущая цветовая схема.
        [[= XML::Attr]] std::string colorScheme;
        // Настройка отображения: яркость выделенных объектов.
        [[= XML::Attr]] int hilightRate{};
        // Настройка отображения: степень затемнения невыделенных объектов.
        [[= XML::Attr]] int darkRate{};
        // Настройка отображения: цвет фона.
        [[= XML::Attr]] std::string background;
        // Настройка отображения: цвет контура платы.
        [[= XML::Attr]] std::string board;
        // Настройка отображения: цвет линий связей.
        [[= XML::Attr]] std::string netLines;
        // Настройка отображения: цвет запрета размещения на обеих сторонах платы.
        [[= XML::Attr]] std::string keepoutPlaceBoth;
        // Настройка отображения: цвет запрета трассировки на всех слоях.
        [[= XML::Attr]] std::string keepoutWireAll;
        // Настройка отображения: цвет запрета размещения на верхней стороне платы.
        [[= XML::Attr]] std::string keepoutPlaceTop;
        // Настройка отображения: цвет запрета размещения на нижней стороне платы.
        [[= XML::Attr]] std::string keepoutPlaceBot;
        // Настройка отображения: цвет габаритов компонентов.
        [[= XML::Attr]] std::string compsBound;
        // Настройка отображения: цвет позиционных обозначений компонентов.
        [[= XML::Attr]] std::string compsName;
        // Настройка отображения: цвет имён контактов.
        [[= XML::Attr]] std::string pinsName;
        // Настройка отображения: цвет имён цепей контактов.
        [[= XML::Attr]] std::string pinsNet;
        // Настройка отображения: цвет сквозных контактных площадок.
        [[= XML::Attr]] std::string clrThroughPads;
        // Настройка отображения: цвет сквозных переходных отверстий.
        [[= XML::Attr]] std::string clrThroughVias;
        // Настройка отображения: цвет скрытых переходных отверстий.
        [[= XML::Attr]] std::string clrBurriedVias;
        // Настройка отображения: цвет глухих переходных отверстий.
        [[= XML::Attr]] std::string clrBlindVias;
        // Настройка отображения: цвет зафиксированных переходных отверстий.
        [[= XML::Attr]] std::string clrFixedVias;
        // Настройка отображения: цвет нарушений DRC.
        [[= XML::Attr]] std::string drcViolation;
        // Настройка отображения: цвет индикации уменьшения номинального зазора.
        [[= XML::Attr]] std::string narrow;
        // Настройка отображения: цвет индикации уменьшения ширины проводника.
        [[= XML::Attr]] std::string trimmed;
    };
    // Настройка отображения: настройки видимости объектов.
    struct Show {
        // Настройка отображения: текущая схема отображения.
        [[= XML::Attr]] std::string displayScheme;
        // Настройка отображения: показывать контур платы.
        [[= XML::Attr]] Bool showBoardOutline{};
        // public bool showBoardOutlineSpecified
        bool getShowBoardOutlineSpecified() const;
        // Настройка отображения: показывать проводники.
        [[= XML::Attr]] Bool showWires{};
        // public bool showWiresSpecified
        bool getShowWiresSpecified() const;
        // Настройка отображения: показывать области металлизации (полигоны).
        [[= XML::Attr]] Bool showCoppers{};
        // public bool showCoppersSpecified
        bool getShowCoppersSpecified() const;
        // Настройка отображения: показывать ярлыки (надписи).
        [[= XML::Attr]] Bool showTexts{};
        // public bool showTextsSpecified
        bool getShowTextsSpecified() const;
        // Настройка отображения: показывать сквозные контактные площадки специальным цветом.
        [[= XML::Attr]] Bool throughPad{};
        // public bool throughPadSpecified
        bool getThroughPadSpecified() const;
        // Настройка отображения: показывать сквозные переходные отверстия специальным цветом.
        [[= XML::Attr]] Bool throughVia{};
        // public bool throughViaSpecified
        bool getThroughViaSpecified() const;
        // Настройка отображения: показывать скрытые переходные отверстия специальным цветом
        [[= XML::Attr]] Bool burriedVia{};
        // public bool burriedViaSpecified
        bool getBurriedViaSpecified() const;
        // Настройка отображения: показывать глухие переходные отверстия специальным цветом.
        [[= XML::Attr]] Bool blindVia{};
        // public bool blindViaSpecified
        bool getBlindViaSpecified() const;
        // Настройка отображения: показывать фиксированные переходные отверстия специальным цветом.
        [[= XML::Attr]] Bool fixedVia{};
        // public bool fixedViaSpecified
        bool getFixedViaSpecified() const;
        // Настройка отображения: показывать переходы.
        [[= XML::Attr]] Bool showVias{};
        // public bool showViasSpecified
        bool getShowViasSpecified() const;
        // Настройка отображения: показывать металлические слои.
        [[= XML::Attr]] Bool showSignalLayers{};
        // public bool showSignalLayersSpecified
        bool getShowSignalLayersSpecified() const;
        // Настройка отображения: показывать верхние механические слои.
        [[= XML::Attr]] Bool showTopMechLayers{};
        // public bool showTopMechLayersSpecified
        bool getShowTopMechLayersSpecified() const;
        // Настройка отображения: показывать нижние механические слои.
        [[= XML::Attr]] Bool showBotMechLayers{};
        // public bool showBotMechLayersSpecified
        bool getShowBotMechLayersSpecified() const;
        // Настройка отображения: показывать документирующие слои.
        [[= XML::Attr]] Bool showDocLayers{};
        // public bool showDocLayersSpecified
        bool getShowDocLayersSpecified() const;
        // Настройка отображения: показывать детали на верхних металлических слоях.
        [[= XML::Attr]] Bool showTopMechDetails{};
        // public bool showTopMechDetailsSpecified
        bool getShowTopMechDetailsSpecified() const;
        // Настройка отображения: показывать детали на нижних металлических слоях.
        [[= XML::Attr]] Bool showBotMechDetails{};
        // public bool showBotMechDetailsSpecified
        bool getShowBotMechDetailsSpecified() const;
        // Настройка отображения: показывать контактные площадки на металлических слоях.
        [[= XML::Attr]] Bool showMetalPads{};
        // public bool showMetalPadsSpecified
        bool getShowMetalPadsSpecified() const;
        // Настройка отображения: показывать КП на верхних металлических слоях.
        [[= XML::Attr]] Bool showTopMechPads{};
        // public bool showTopMechPadsSpecified
        bool getShowTopMechPadsSpecified() const;
        // Настройка отображения: показывать контактные площадки на нижних металлических слоях.
        [[= XML::Attr]] Bool showBotMechPads{};
        // public bool showBotMechPadsSpecified
        bool getShowBotMechPadsSpecified() const;
        // Настройка отображения: показывать связи.
        [[= XML::Attr]] Bool showNetLines{};
        // public bool showNetLinesSpecified
        bool getShowNetLinesSpecified() const;
        // Настройка отображения: показывать монтажные отверстия.
        [[= XML::Attr]] Bool showMountingHoles{};
        // public bool showMountingHolesSpecified
        bool getShowMountingHolesSpecified() const;
        // Настройка отображения: показывать проводники тонкими линиями.
        [[= XML::Attr]] Bool showThinWires{};
        // public bool showThinWiresSpecified
        bool getShowThinWiresSpecified() const;
        // Настройка отображения: показывать компоненты.
        [[= XML::Attr]] Bool showComponents{};
        // public bool showComponentsSpecified
        bool getShowComponentsSpecified() const;
        // Настройка отображения: показывать компоненты на верхней стороне.
        [[= XML::Attr]] Bool showCompTop{};
        // public bool showCompTopSpecified
        bool getShowCompTopSpecified() const;
        // Настройка отображения: показывать компоненты на нижней стороне.
        [[= XML::Attr]] Bool showCompBot{};
        // public bool showCompBotSpecified
        bool getShowCompBotSpecified() const;
        // Настройка отображения: показывать позиционные обозначения компонентов.
        [[= XML::Attr]] Bool showCompsDes{};
        // public bool showCompsDesSpecified
        bool getShowCompsDesSpecified() const;
        // Настройка отображения: показывать имена контактов.
        [[= XML::Attr]] Bool showPinsName{};
        // public bool showPinsNameSpecified
        bool getShowPinsNameSpecified() const;
        // Настройка отображения: показывать имена цепей контактов.
        [[= XML::Attr]] Bool showPinsNet{};
        // public bool showPinsNetSpecified
        bool getShowPinsNetSpecified() const;
        // Настройка отображения: показывать габариты компонентов.
        [[= XML::Attr]] Bool showCompsBound{};
        // public bool showCompsBoundSpecified
        bool getShowCompsBoundSpecified() const;
        // Настройка отображения: показывать ярлыки атрибута RefDes.
        [[= XML::Attr]] Bool showLabelRefDes{};
        // public bool showLabelRefDesSpecified
        bool getShowLabelRefDesSpecified() const;
        // Настройка отображения: показывать ярлыки атрибута PartName.
        [[= XML::Attr]] Bool showLabelPartName{};
        // public bool showLabelPartNameSpecified
        bool getShowLabelPartNameSpecified() const;
        // Настройка отображения: показывать ярлыки пользовательских атрибутов.
        [[= XML::Attr]] Bool showLabelOther{};
        // public bool showLabelOtherSpecified
        bool getShowLabelOtherSpecified() const;
        // Настройка отображения: показывать нарушения.
        [[= XML::Attr]] Bool showViolations{};
        // public bool showViolationsSpecified
        bool getShowViolationsSpecified() const;
        // Настройка отображения: показывать уменьшение номинального зазора.
        [[= XML::Attr]] Bool showNarrow{};
        // public bool showNarrowSpecified
        bool getShowNarrowSpecified() const;
        // Настройка отображения: показывать уменьшение ширины проводника.
        [[= XML::Attr]] Bool showTrimmed{};
        // public bool showTrimmedSpecified
        bool getShowTrimmedSpecified() const;
        // Настройка отображения: показывать нарушение DRC.
        [[= XML::Attr]] Bool showDRCViolations{};
        // public bool showDRCViolationsSpecified
        bool getShowDRCViolationsSpecified() const;
        // Настройка отображения: показывать запреты.
        [[= XML::Attr]] Bool showKeepouts{};
        // public bool showKeepoutsSpecified
        bool getShowKeepoutsSpecified() const;
        // Настройка отображения: показывать запреты трассировки.
        [[= XML::Attr]] Bool showRouteKeepouts{};
        // public bool showRouteKeepoutsSpecified
        bool getShowRouteKeepoutsSpecified() const;
        // Настройка отображения: показывать запреты размещения.
        [[= XML::Attr]] Bool showPlaceKeepouts{};
        // public bool showPlaceKeepoutsSpecified
        bool getShowPlaceKeepoutsSpecified() const;
        // Настройка отображения: показывать только активный слой.
        [[= XML::Attr]] Bool showActiveLayerOnly{};
        // public bool showActiveLayerOnlySpecified
        bool getShowActiveLayerOnlySpecified() const;
        // Настройка отображения: показывать области змеек.
        [[= XML::Attr]] Bool showSerpentArea{};
        // public bool showSerpentAreaSpecified
        bool getShowSerpentAreaSpecified() const;
    };
    // Настройки сетки.
    struct Grid {
        // Настройка отображения сетки: шаг сетки.
        struct GridSpace {
            // шаг сетки по горизонтали.
            [[= XML::Attr]] double x{};
            // шаг сетки по вертикали.
            [[= XML::Attr]] double y{};
        };
        // Настройка отображения сетки: цвет сетки.
        [[= XML::Attr]] std::string gridColor;
        // Настройка отображения сетки: тип сетки.
        [[= XML::Attr]] gridKind gridKind{};
        // Настройка отображения сетки: показывать сетку.
        [[= XML::Attr]] Bool gridShow{};
        //
        [[= XML::Attr]] Bool saveProportion{};
        // public bool gridShowSpecified
        bool getGridShowSpecified() const;
        // Настройка ручного редактора: выравнивание на сетку.
        [[= XML::Attr]] Bool alignTogrid{};
        // public bool alignToGridSpecified
        bool getAlignToGridSpecified() const;
        // Настройка ручного редактирования: привязка к углу кратному 45˚.
        [[= XML::Attr]] Bool snapToAngle{};
        // public bool snapToAngleSpecified
        bool getSnapToAngleSpecified() const;
        // Настройка отображения сетки: шаг сетки.
        /*[[= XML::Elem]]*/ GridSpace GridSpace;
    };
    // Настройка отображения: настройки видимости слоя.
    struct LayerOptions {
        // Настройка отображения: цветовые настройки слоя.
        struct Colors {
            // Настройка отображения слоя: цвет деталей, проводников (основной цвет слоя).
            [[= XML::Attr]] std::string details;
            // Настройка отображения слоя: цвет контактных площадок.
            [[= XML::Attr]] std::string pads;
            // Настройка отображения слоя: цвет зафиксированных объектов.
            [[= XML::Attr]] std::string fix;
        };
        // Настройка отображения слоя: настройки видимости.
        struct Show {
            // Флаг видимости.
            [[= XML::Attr]] Bool visible{};
            // public bool visibleSpecified
            bool getVisibleSpecified() const;
            // Настройка отображения слоя: видимость деталей.
            [[= XML::Attr]] Bool details{};
            // public bool detailsSpecified
            bool getDetailsSpecified() const;
            // Настройка отображения слоя: видимость контактных площадок.
            [[= XML::Attr]] Bool pads{};
            // public bool padsSpecified
            bool getPadsSpecified() const;
        };
        // Ссылка на слой.
        /*[[= XML::Elem]]*/ LayerRef LayerRef;
        // Настройка отображения: цветовые настройки слоя.
        /*[[= XML::Elem]]*/ Colors Colors;
        // Настройка отображения слоя: настройки видимости.
        /*[[= XML::Elem]]*/ Show Show;
    };
    // Отображение цепей особым цветом.
    struct ColorNets {
        // Отображение цепей особым цветом: установить цвет для цепи / сигнала / группы цепей / группы сигналов.
        struct SetColor {
            // Отображение цепей особым цветом: задание цвета.
            [[= XML::Attr]] std::string color;
            // Ссылка на цепь или сигнал
            // public Object Refs;
            /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ NetRef, NetGroupRef, AllNets, SignalRef, DiffSignalRef, SignalGroupRef> Refs;
        };
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Отображение цепей особым цветом: применять для проводников.
        [[= XML::Attr]] Bool colorizeWire{};
        // public bool colorizeWireSpecified
        bool getColorizeWireSpecified() const;
        // Отображение цепей особым цветом: применять для контактных площадок.
        [[= XML::Attr]] Bool colorizePad{};
        // public bool colorizePadSpecified
        bool getColorizePadSpecified() const;
        // Отображение цепей особым цветом: применять для областей металлизации.
        [[= XML::Attr]] Bool colorizeCopper{};
        // public bool colorizeCopperSpecified
        bool getColorizeCopperSpecified() const;
        // Отображение цепей особым цветом: применять для переходов.
        [[= XML::Attr]] Bool colorizeVia{};
        // public bool colorizeViaSpecified
        bool getColorizeViaSpecified() const;
        // Отображение цепей особым цветом: применять для связей.
        [[= XML::Attr]] Bool colorizeNetline{};
        // public bool colorizeNetlineSpecified
        bool getColorizeNetlineSpecified() const;
        // Отображение цепей особым цветом: установить цвет для цепи / сигнала / группы цепей / группы сигналов.
        //[[= XML::Elem("SetColor")]] // public List<SetColor> SetColors;
        [[= XML::Elem]] std::vector<SetColor> SetColors;
        bool ShouldSerialize_SetColors();
    };
    // Фильтр отображения связей.
    struct FilterNetlines {
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Ссылки на цепь или сигнал
        // public List<Object> Refs;
        [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ NetRef, NetGroupRef, AllNets, SignalRef, DiffSignalRef, SignalGroupRef>> Refs;
        bool ShouldSerialize_Refs();
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Настройка отображения: параметры текущего вида.
    /*[[= XML::Elem]]*/ View View;
    // Устанавливает активный слой.
    /*[[= XML::Elem]]*/ ActiveLayer ActiveLayer;
    // Настройка отображения: единицы измерения.
    [[= XML::ElemF]] Units Units;
    // Настройка отображения: общие цветовые настройки.
    /*[[= XML::Elem]]*/ Colors Colors;
    //  Настройка отображения: настройки видимости объектов.
    /*[[= XML::Elem]]*/ Show Show;
    //  Настройки сетки.
    /*[[= XML::Elem]]*/ Grid Grid;
    // Настройка отображения: настройки видимости слоёв.
    //[XmlArrayItem("LayerOptions")] public List<LayerOptions> LayersVisualOptions;
    [[= XML::Array]] std::vector<LayerOptions> LayersVisualOptions;
    bool ShouldSerialize_LayersVisualOptions();
    // Отображение цепей особым цветом.
    /*[[= XML::Elem]]*/ ColorNets ColorNets;
    // Фильтр отображения связей.
    [[= XML::ElemF]] FilterNetlines FilterNetlines;
    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DisplayControl. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/
    /**************************************************************************/
};
} // namespace TopoR

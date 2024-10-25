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
/// Раздел «Настройки отображения».
/// </summary>
class DisplayControl : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Настройка отображения: параметры текущего вида.
    /// </summary>
public:
    class View {
        /// <summary>
        /// Параметр текущего вида: масштаб.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("scale", DataType = "float")] public float _scale;
        float _scale = 0.0F;

        /// <summary>
        /// Параметр текущего вида: прокрутка по горизонтали.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("scrollHorz", DataType = "float")] public float _scrollHorz;
        float _scrollHorz = 0.0F;

        /// <summary>
        /// Параметр текущего вида: прокрутка по вертикали.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("scrollVert", DataType = "float")] public float _scrollVert;
        float _scrollVert = 0.0F;
    };

    /// <summary>
    /// Устанавливает активный слой.
    /// </summary>
public:
    class ActiveLayer {
        /* Опечатка в спецификации?    /// <summary>.
            /// Тип слоя.
            /// </summary>
            [XmlAttribute("type")]
            public layer_type _type;*/

        /// <summary>
        /// Наименование слоя.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;
    };

    /// <summary>
    /// Настройка отображения: единицы измерения.
    /// </summary>
public:
    class Units_DisplayControl {
        /// <summary>
        /// Настройка отображения: единицы измерения.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("preference")] public preference _preference;
        preference _preference = static_cast<preference>(0);
    };

    /// <summary>
    /// Настройка отображения: общие цветовые настройки.
    /// </summary>
public:
    class Colors_DisplayControl {
        /// <summary>
        /// Настройка отображения: текущая цветовая схема.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("colorScheme")] public string _colorScheme;
        QString _colorScheme;

        /// <summary>
        /// Настройка отображения: яркость выделенных объектов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("hilightRate", DataType = "int")] public int _hilightRate;
        int _hilightRate = 0;

        /// <summary>
        /// Настройка отображения: степень затемнения невыделенных объектов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("darkRate", DataType = "int")] public int _darkRate;
        int _darkRate = 0;

        /// <summary>
        /// Настройка отображения: цвет фона.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("background")] public string _background;
        QString _background;

        /// <summary>
        /// Настройка отображения: цвет контура платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("board")] public string _board;
        QString _board;

        /// <summary>
        /// Настройка отображения: цвет линий связей.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("netLines")] public string _netLines;
        QString _netLines;

        /// <summary>
        /// Настройка отображения: цвет запрета размещения на обеих сторонах платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("keepoutPlaceBoth")] public string _keepoutPlaceBoth;
        QString _keepoutPlaceBoth;

        /// <summary>
        /// Настройка отображения: цвет запрета трассировки на всех слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("keepoutWireAll")] public string _keepoutWireAll;
        QString _keepoutWireAll;

        /// <summary>
        /// Настройка отображения: цвет запрета размещения на верхней стороне платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("keepoutPlaceTop")] public string _keepoutPlaceTop;
        QString _keepoutPlaceTop;

        /// <summary>
        /// Настройка отображения: цвет запрета размещения на нижней стороне платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("keepoutPlaceBot")] public string _keepoutPlaceBot;
        QString _keepoutPlaceBot;

        /// <summary>
        /// Настройка отображения: цвет габаритов компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("compsBound")] public string _compsBound;
        QString _compsBound;

        /// <summary>
        /// Настройка отображения: цвет позиционных обозначений компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("compsName")] public string _compsName;
        QString _compsName;

        /// <summary>
        /// Настройка отображения: цвет имён контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("pinsName")] public string _pinsName;
        QString _pinsName;

        /// <summary>
        /// Настройка отображения: цвет имён цепей контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("pinsNet")] public string _pinsNet;
        QString _pinsNet;

        /// <summary>
        /// Настройка отображения: цвет сквозных контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrThroughPads")] public string _clrThroughPads;
        QString _clrThroughPads;

        /// <summary>
        /// Настройка отображения: цвет сквозных переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrThroughVias")] public string _clrThroughVias;
        QString _clrThroughVias;

        /// <summary>
        /// Настройка отображения: цвет скрытых переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrBurriedVias")] public string _clrBurriedVias;
        QString _clrBurriedVias;

        /// <summary>
        /// Настройка отображения: цвет глухих переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrBlindVias")] public string _clrBlindVias;
        QString _clrBlindVias;

        /// <summary>
        /// Настройка отображения: цвет зафиксированных переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrFixedVias")] public string _clrFixedVias;
        QString _clrFixedVias;

        /// <summary>
        /// Настройка отображения: цвет нарушений DRC.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("drcViolation")] public string _drcViolation;
        QString _drcViolation;

        /// <summary>
        /// Настройка отображения: цвет индикации уменьшения номинального зазора.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("narrow")] public string _narrow;
        QString _narrow;

        /// <summary>
        /// Настройка отображения: цвет индикации уменьшения ширины проводника.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("trimmed")] public string _trimmed;
        QString _trimmed;
    };

    /// <summary>
    /// Настройка отображения: настройки видимости объектов.
    /// </summary>
public:
    class Show_DisplayControl {
        /// <summary>
        /// Настройка отображения: текущая схема отображения.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("displayScheme")] public string _displayScheme;
        QString _displayScheme;

        /// <summary>
        /// Настройка отображения: показывать контур платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showBoardOutline")] public Bool _showBoardOutline;
        Bool _showBoardOutline = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBoardOutlineSpecified
        bool getShowBoardOutlineSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать проводники.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showWires")] public Bool _showWires;
        Bool _showWires = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showWiresSpecified
        bool getShowWiresSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать области металлизации (полигоны).
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showCoppers")] public Bool _showCoppers;
        Bool _showCoppers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCoppersSpecified
        bool getShowCoppersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки (надписи).
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showTexts")] public Bool _showTexts;
        Bool _showTexts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTextsSpecified
        bool getShowTextsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать сквозные контактные площадки специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("throughPad")] public Bool _throughPad;
        Bool _throughPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _throughPadSpecified
        bool getThroughPadSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать сквозные переходные отверстия специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("throughVia")] public Bool _throughVia;
        Bool _throughVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _throughViaSpecified
        bool getThroughViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать скрытые переходные отверстия специальным цветом
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("burriedVia")] public Bool _burriedVia;
        Bool _burriedVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _burriedViaSpecified
        bool getBurriedViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать глухие переходные отверстия специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("blindVia")] public Bool _blindVia;
        Bool _blindVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _blindViaSpecified
        bool getBlindViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать фиксированные переходные отверстия специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("fixedVia")] public Bool _fixedVia;
        Bool _fixedVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _fixedViaSpecified
        bool getFixedViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать переходы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showVias")] public Bool _showVias;
        Bool _showVias = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showViasSpecified
        bool getShowViasSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать металлические слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showSignalLayers")] public Bool _showSignalLayers;
        Bool _showSignalLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showSignalLayersSpecified
        bool getShowSignalLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать верхние механические слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showTopMechLayers")] public Bool _showTopMechLayers;
        Bool _showTopMechLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTopMechLayersSpecified
        bool getShowTopMechLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать нижние механические слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showBotMechLayers")] public Bool _showBotMechLayers;
        Bool _showBotMechLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBotMechLayersSpecified
        bool getShowBotMechLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать документирующие слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showDocLayers")] public Bool _showDocLayers;
        Bool _showDocLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showDocLayersSpecified
        bool getShowDocLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать детали на верхних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showTopMechDetails")] public Bool _showTopMechDetails;
        Bool _showTopMechDetails = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTopMechDetailsSpecified
        bool getShowTopMechDetailsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать детали на нижних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showBotMechDetails")] public Bool _showBotMechDetails;
        Bool _showBotMechDetails = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBotMechDetailsSpecified
        bool getShowBotMechDetailsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать контактные площадки на металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showMetalPads")] public Bool _showMetalPads;
        Bool _showMetalPads = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showMetalPadsSpecified
        bool getShowMetalPadsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать КП на верхних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showTopMechPads")] public Bool _showTopMechPads;
        Bool _showTopMechPads = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTopMechPadsSpecified
        bool getShowTopMechPadsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать контактные площадки на нижних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showBotMechPads")] public Bool _showBotMechPads;
        Bool _showBotMechPads = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBotMechPadsSpecified
        bool getShowBotMechPadsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать связи.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showNetLines")] public Bool _showNetLines;
        Bool _showNetLines = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showNetLinesSpecified
        bool getShowNetLinesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать монтажные отверстия.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showMountingHoles")] public Bool _showMountingHoles;
        Bool _showMountingHoles = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showMountingHolesSpecified
        bool getShowMountingHolesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать проводники тонкими линиями.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showThinWires")] public Bool _showThinWires;
        Bool _showThinWires = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showThinWiresSpecified
        bool getShowThinWiresSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать компоненты.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showComponents")] public Bool _showComponents;
        Bool _showComponents = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showComponentsSpecified
        bool getShowComponentsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать компоненты на верхней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showCompTop")] public Bool _showCompTop;
        Bool _showCompTop = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompTopSpecified
        bool getShowCompTopSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать компоненты на нижней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showCompBot")] public Bool _showCompBot;
        Bool _showCompBot = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompBotSpecified
        bool getShowCompBotSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать позиционные обозначения компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showCompsDes")] public Bool _showCompsDes;
        Bool _showCompsDes = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompsDesSpecified
        bool getShowCompsDesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать имена контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showPinsName")] public Bool _showPinsName;
        Bool _showPinsName = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showPinsNameSpecified
        bool getShowPinsNameSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать имена цепей контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showPinsNet")] public Bool _showPinsNet;
        Bool _showPinsNet = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showPinsNetSpecified
        bool getShowPinsNetSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать габариты компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showCompsBound")] public Bool _showCompsBound;
        Bool _showCompsBound = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompsBoundSpecified
        bool getShowCompsBoundSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки атрибута RefDes.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showLabelRefDes")] public Bool _showLabelRefDes;
        Bool _showLabelRefDes = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showLabelRefDesSpecified
        bool getShowLabelRefDesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки атрибута PartName.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showLabelPartName")] public Bool _showLabelPartName;
        Bool _showLabelPartName = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showLabelPartNameSpecified
        bool getShowLabelPartNameSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки пользовательских атрибутов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showLabelOther")] public Bool _showLabelOther;
        Bool _showLabelOther = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showLabelOtherSpecified
        bool getShowLabelOtherSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать нарушения.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showViolations")] public Bool _showViolations;
        Bool _showViolations = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showViolationsSpecified
        bool getShowViolationsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать уменьшение номинального зазора.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showNarrow")] public Bool _showNarrow;
        Bool _showNarrow = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showNarrowSpecified
        bool getShowNarrowSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать уменьшение ширины проводника.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showTrimmed")] public Bool _showTrimmed;
        Bool _showTrimmed = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTrimmedSpecified
        bool getShowTrimmedSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать нарушение DRC.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showDRCViolations")] public Bool _showDRCViolations;
        Bool _showDRCViolations = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showDRCViolationsSpecified
        bool getShowDRCViolationsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать запреты.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showKeepouts")] public Bool _showKeepouts;
        Bool _showKeepouts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showKeepoutsSpecified
        bool getShowKeepoutsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать запреты трассировки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showRouteKeepouts")] public Bool _showRouteKeepouts;
        Bool _showRouteKeepouts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showRouteKeepoutsSpecified
        bool getShowRouteKeepoutsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать запреты размещения.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showPlaceKeepouts")] public Bool _showPlaceKeepouts;
        Bool _showPlaceKeepouts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showPlaceKeepoutsSpecified
        bool getShowPlaceKeepoutsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать только активный слой.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showActiveLayerOnly")] public Bool _showActiveLayerOnly;
        Bool _showActiveLayerOnly = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showActiveLayerOnlySpecified
        bool getShowActiveLayerOnlySpecified() const;

        /// <summary>
        /// Настройка отображения: показывать области змеек.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("showSerpentArea")] public Bool _showSerpentArea;
        Bool _showSerpentArea = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showSerpentAreaSpecified
        bool getShowSerpentAreaSpecified() const;
    };

    /// <summary>
    /// Настройки сетки.
    /// </summary>
public:
    class Grid {
        /// <summary>
        /// Настройка отображения сетки: шаг сетки.
        /// </summary>
    public:
        class GridSpace {
            /// <summary>
            /// шаг сетки по горизонтали.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("x", DataType = "float")] public float _x;
            float _x = 0.0F;

            /// <summary>
            /// шаг сетки по вертикали.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("y", DataType = "float")] public float _y;
            float _y = 0.0F;
        };

        /// <summary>
        /// Настройка отображения сетки: цвет сетки.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("gridColor")] public string _gridColor;
        QString _gridColor;

        /// <summary>
        /// Настройка отображения сетки: тип сетки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("gridKind")] public gridKind _gridKind;
        gridKind _gridKind = static_cast<gridKind>(0);

        /// <summary>
        /// Настройка отображения сетки: показывать сетку.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("gridShow")] public Bool _gridShow;
        Bool _gridShow = static_cast<Bool>(0);
        virtual ~Grid() {
            delete _GridSpace;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _gridShowSpecified
        bool getGridShowSpecified() const;

        /// <summary>
        /// Настройка ручного редактора: выравнивание на сетку.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("alignToGrid")] public Bool _alignToGrid;
        Bool _alignToGrid = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _alignToGridSpecified
        bool getAlignToGridSpecified() const;

        /// <summary>
        /// Настройка ручного редактирования: привязка к углу кратному 45˚.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("snapToAngle")] public Bool _snapToAngle;
        Bool _snapToAngle = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _snapToAngleSpecified
        bool getSnapToAngleSpecified() const;

        /// <summary>
        /// Настройка отображения сетки: шаг сетки.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("GridSpace")] public GridSpace _GridSpace;
        GridSpace* _GridSpace;
    };

    /// <summary>
    /// Настройка отображения: настройки видимости слоя.
    /// </summary>
public:
    class LayerOptions {
        /// <summary>
        /// Настройка отображения: цветовые настройки слоя.
        /// </summary>
    public:
        class Colors_LayerOptions {
            /// <summary>
            /// Настройка отображения слоя: цвет деталей, проводников (основной цвет слоя).
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("details")] public string _details;
            QString _details;

            /// <summary>
            /// Настройка отображения слоя: цвет контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("pads")] public string _pads;
            QString _pads;

            /// <summary>
            /// Настройка отображения слоя: цвет зафиксированных объектов.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("fix")] public string _fix;
            QString _fix;
        };

        /// <summary>
        /// Настройка отображения слоя: настройки видимости.
        /// </summary>
    public:
        class Show_LayerOptions {
            /// <summary>
            /// Флаг видимости.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("visible")] public Bool _visible;
            Bool _visible = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _visibleSpecified
            bool getVisibleSpecified() const;

            /// <summary>
            /// Настройка отображения слоя: видимость деталей.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("details")] public Bool _details;
            Bool _details = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _detailsSpecified
            bool getDetailsSpecified() const;

            /// <summary>
            /// Настройка отображения слоя: видимость контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("pads")] public Bool _pads;
            Bool _pads = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _padsSpecified
            bool getPadsSpecified() const;
        };

        /// <summary>
        /// Ссылка на слой.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
        LayerRef* _LayerRef;

        /// <summary>
        /// Настройка отображения: цветовые настройки слоя.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Colors")] public Colors_LayerOptions _Colors;
        Colors_LayerOptions* _Colors;

        /// <summary>
        /// Настройка отображения слоя: настройки видимости.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Show")] public Show_LayerOptions _Show;
        Show_LayerOptions* _Show;
        virtual ~LayerOptions() {
            delete _LayerRef;
            delete _Colors;
            delete _Show;
        }
    };

    /// <summary>
    /// Отображение цепей особым цветом.
    /// </summary>
public:
    class ColorNets {
        /// <summary>
        /// Отображение цепей особым цветом: установить цвет для цепи / сигнала / группы цепей / группы сигналов.
        /// </summary>
    public:
        class SetColor {
            /// <summary>
            /// Отображение цепей особым цветом: задание цвета.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("color")] public string _color;
            QString _color;

            /// <summary>
            /// Ссылка на цепь или сигнал
            /// </summary>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement("NetRef", typeof(NetRef)), XmlElement("NetGroupRef", typeof(NetGroupRef)), XmlElement("AllNets", typeof(AllNets)), XmlElement("SignalRef", typeof(SignalRef)), XmlElement("DiffSignalRef", typeof(DiffSignalRef)), XmlElement("SignalGroupRef", typeof(SignalGroupRef)),] public Object _Refs;
            std::any _Refs;
        };

        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для проводников.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("colorizeWire")] public Bool _colorizeWire;
        Bool _colorizeWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeWireSpecified
        bool getColorizeWireSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("colorizePad")] public Bool _colorizePad;
        Bool _colorizePad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizePadSpecified
        bool getColorizePadSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для областей металлизации.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("colorizeCopper")] public Bool _colorizeCopper;
        Bool _colorizeCopper = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeCopperSpecified
        bool getColorizeCopperSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для переходов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("colorizeVia")] public Bool _colorizeVia;
        Bool _colorizeVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeViaSpecified
        bool getColorizeViaSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для связей.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("colorizeNetline")] public Bool _colorizeNetline;
        Bool _colorizeNetline = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeNetlineSpecified
        bool getColorizeNetlineSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: установить цвет для цепи / сигнала / группы цепей / группы сигналов.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("SetColor")] public List<SetColor> _SetColors;
        std::vector<SetColor*> _SetColors;
        bool ShouldSerialize_SetColors();
    };

    /// <summary>
    /// Фильтр отображения связей.
    /// </summary>
public:
    class FilterNetlines {
        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Ссылки на цепь или сигнал
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("NetRef", typeof(NetRef)), XmlElement("NetGroupRef", typeof(NetGroupRef)), XmlElement("AllNets", typeof(AllNets)), XmlElement("SignalRef", typeof(SignalRef)), XmlElement("DiffSignalRef", typeof(DiffSignalRef)), XmlElement("SignalGroupRef", typeof(SignalGroupRef)),] public List<Object> _Refs;
        std::vector<std::any> _Refs;
        bool ShouldSerialize_Refs();
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Настройка отображения: параметры текущего вида.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("View")] public View _View;
    View* _View;

    /// <summary>
    /// Устанавливает активный слой.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("ActiveLayer")] public ActiveLayer _ActiveLayer;
    ActiveLayer* _ActiveLayer;

    /// <summary>
    /// Настройка отображения: единицы измерения.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Units")] public Units_DisplayControl _Units;
    Units_DisplayControl* _Units;

    /// <summary>
    /// Настройка отображения: общие цветовые настройки.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Colors")] public Colors_DisplayControl _Colors;
    Colors_DisplayControl* _Colors;

    /// <summary>
    ///  Настройка отображения: настройки видимости объектов.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Show")] public Show_DisplayControl _Show;
    Show_DisplayControl* _Show;

    /// <summary>
    ///  Настройки сетки.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Grid")] public Grid _Grid;
    Grid* _Grid;

    /// <summary>
    /// Настройка отображения: настройки видимости слоёв.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("LayersVisualOptions")][XmlArrayItem("LayerOptions")] public List<LayerOptions> _LayersVisualOptions;
    std::vector<LayerOptions*> _LayersVisualOptions;
    virtual ~DisplayControl() {
        delete _View;
        delete _ActiveLayer;
        delete _Units;
        delete _Colors;
        delete _Show;
        delete _Grid;
        delete _ColorNets;
        delete _FilterNetlines;
    }

    bool ShouldSerialize_LayersVisualOptions();

    /// <summary>
    /// Отображение цепей особым цветом.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("ColorNets")] public ColorNets _ColorNets;
    ColorNets* _ColorNets;

    /// <summary>
    /// Фильтр отображения связей.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("FilterNetlines")] public FilterNetlines _FilterNetlines;
    FilterNetlines* _FilterNetlines;

    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DisplayControl. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/

    /**************************************************************************/
};
// } // namespace TopoR_PCB_Classes

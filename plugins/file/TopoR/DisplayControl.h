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
        // ORIGINAL LINE: [XmlAttribute(u"scale"_s, DataType = u"float"_s)] public float _scale;
        float _scale = 0.0F;

        /// <summary>
        /// Параметр текущего вида: прокрутка по горизонтали.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"scrollHorz"_s, DataType = u"float"_s)] public float _scrollHorz;
        float _scrollHorz = 0.0F;

        /// <summary>
        /// Параметр текущего вида: прокрутка по вертикали.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"scrollVert"_s, DataType = u"float"_s)] public float _scrollVert;
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
            [XmlAttribute(u"type"_s)]
            public layer_type _type;*/

        /// <summary>
        /// Наименование слоя.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"name"_s)] public string _name;
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
        // ORIGINAL LINE: [XmlAttribute(u"preference"_s)] public preference _preference;
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
        // ORIGINAL LINE: [XmlAttribute(u"colorScheme"_s)] public string _colorScheme;
        QString _colorScheme;

        /// <summary>
        /// Настройка отображения: яркость выделенных объектов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"hilightRate"_s, DataType = u"int"_s)] public int _hilightRate;
        int _hilightRate{};

        /// <summary>
        /// Настройка отображения: степень затемнения невыделенных объектов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"darkRate"_s, DataType = u"int"_s)] public int _darkRate;
        int _darkRate{};

        /// <summary>
        /// Настройка отображения: цвет фона.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"background"_s)] public string _background;
        QString _background;

        /// <summary>
        /// Настройка отображения: цвет контура платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"board"_s)] public string _board;
        QString _board;

        /// <summary>
        /// Настройка отображения: цвет линий связей.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"netLines"_s)] public string _netLines;
        QString _netLines;

        /// <summary>
        /// Настройка отображения: цвет запрета размещения на обеих сторонах платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"keepoutPlaceBoth"_s)] public string _keepoutPlaceBoth;
        QString _keepoutPlaceBoth;

        /// <summary>
        /// Настройка отображения: цвет запрета трассировки на всех слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"keepoutWireAll"_s)] public string _keepoutWireAll;
        QString _keepoutWireAll;

        /// <summary>
        /// Настройка отображения: цвет запрета размещения на верхней стороне платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"keepoutPlaceTop"_s)] public string _keepoutPlaceTop;
        QString _keepoutPlaceTop;

        /// <summary>
        /// Настройка отображения: цвет запрета размещения на нижней стороне платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"keepoutPlaceBot"_s)] public string _keepoutPlaceBot;
        QString _keepoutPlaceBot;

        /// <summary>
        /// Настройка отображения: цвет габаритов компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"compsBound"_s)] public string _compsBound;
        QString _compsBound;

        /// <summary>
        /// Настройка отображения: цвет позиционных обозначений компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"compsName"_s)] public string _compsName;
        QString _compsName;

        /// <summary>
        /// Настройка отображения: цвет имён контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"pinsName"_s)] public string _pinsName;
        QString _pinsName;

        /// <summary>
        /// Настройка отображения: цвет имён цепей контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"pinsNet"_s)] public string _pinsNet;
        QString _pinsNet;

        /// <summary>
        /// Настройка отображения: цвет сквозных контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"clrThroughPads"_s)] public string _clrThroughPads;
        QString _clrThroughPads;

        /// <summary>
        /// Настройка отображения: цвет сквозных переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"clrThroughVias"_s)] public string _clrThroughVias;
        QString _clrThroughVias;

        /// <summary>
        /// Настройка отображения: цвет скрытых переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"clrBurriedVias"_s)] public string _clrBurriedVias;
        QString _clrBurriedVias;

        /// <summary>
        /// Настройка отображения: цвет глухих переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"clrBlindVias"_s)] public string _clrBlindVias;
        QString _clrBlindVias;

        /// <summary>
        /// Настройка отображения: цвет зафиксированных переходных отверстий.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"clrFixedVias"_s)] public string _clrFixedVias;
        QString _clrFixedVias;

        /// <summary>
        /// Настройка отображения: цвет нарушений DRC.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"drcViolation"_s)] public string _drcViolation;
        QString _drcViolation;

        /// <summary>
        /// Настройка отображения: цвет индикации уменьшения номинального зазора.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"narrow"_s)] public string _narrow;
        QString _narrow;

        /// <summary>
        /// Настройка отображения: цвет индикации уменьшения ширины проводника.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"trimmed"_s)] public string _trimmed;
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
        // ORIGINAL LINE: [XmlAttribute(u"displayScheme"_s)] public string _displayScheme;
        QString _displayScheme;

        /// <summary>
        /// Настройка отображения: показывать контур платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showBoardOutline"_s)] public Bool _showBoardOutline;
        Bool _showBoardOutline = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBoardOutlineSpecified
        bool getShowBoardOutlineSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать проводники.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showWires"_s)] public Bool _showWires;
        Bool _showWires = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showWiresSpecified
        bool getShowWiresSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать области металлизации (полигоны).
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showCoppers"_s)] public Bool _showCoppers;
        Bool _showCoppers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCoppersSpecified
        bool getShowCoppersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки (надписи).
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showTexts"_s)] public Bool _showTexts;
        Bool _showTexts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTextsSpecified
        bool getShowTextsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать сквозные контактные площадки специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"throughPad"_s)] public Bool _throughPad;
        Bool _throughPad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _throughPadSpecified
        bool getThroughPadSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать сквозные переходные отверстия специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"throughVia"_s)] public Bool _throughVia;
        Bool _throughVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _throughViaSpecified
        bool getThroughViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать скрытые переходные отверстия специальным цветом
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"burriedVia"_s)] public Bool _burriedVia;
        Bool _burriedVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _burriedViaSpecified
        bool getBurriedViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать глухие переходные отверстия специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"blindVia"_s)] public Bool _blindVia;
        Bool _blindVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _blindViaSpecified
        bool getBlindViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать фиксированные переходные отверстия специальным цветом.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"fixedVia"_s)] public Bool _fixedVia;
        Bool _fixedVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _fixedViaSpecified
        bool getFixedViaSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать переходы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showVias"_s)] public Bool _showVias;
        Bool _showVias = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showViasSpecified
        bool getShowViasSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать металлические слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showSignalLayers"_s)] public Bool _showSignalLayers;
        Bool _showSignalLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showSignalLayersSpecified
        bool getShowSignalLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать верхние механические слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showTopMechLayers"_s)] public Bool _showTopMechLayers;
        Bool _showTopMechLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTopMechLayersSpecified
        bool getShowTopMechLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать нижние механические слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showBotMechLayers"_s)] public Bool _showBotMechLayers;
        Bool _showBotMechLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBotMechLayersSpecified
        bool getShowBotMechLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать документирующие слои.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showDocLayers"_s)] public Bool _showDocLayers;
        Bool _showDocLayers = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showDocLayersSpecified
        bool getShowDocLayersSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать детали на верхних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showTopMechDetails"_s)] public Bool _showTopMechDetails;
        Bool _showTopMechDetails = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTopMechDetailsSpecified
        bool getShowTopMechDetailsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать детали на нижних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showBotMechDetails"_s)] public Bool _showBotMechDetails;
        Bool _showBotMechDetails = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBotMechDetailsSpecified
        bool getShowBotMechDetailsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать контактные площадки на металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showMetalPads"_s)] public Bool _showMetalPads;
        Bool _showMetalPads = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showMetalPadsSpecified
        bool getShowMetalPadsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать КП на верхних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showTopMechPads"_s)] public Bool _showTopMechPads;
        Bool _showTopMechPads = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTopMechPadsSpecified
        bool getShowTopMechPadsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать контактные площадки на нижних металлических слоях.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showBotMechPads"_s)] public Bool _showBotMechPads;
        Bool _showBotMechPads = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showBotMechPadsSpecified
        bool getShowBotMechPadsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать связи.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showNetLines"_s)] public Bool _showNetLines;
        Bool _showNetLines = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showNetLinesSpecified
        bool getShowNetLinesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать монтажные отверстия.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showMountingHoles"_s)] public Bool _showMountingHoles;
        Bool _showMountingHoles = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showMountingHolesSpecified
        bool getShowMountingHolesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать проводники тонкими линиями.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showThinWires"_s)] public Bool _showThinWires;
        Bool _showThinWires = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showThinWiresSpecified
        bool getShowThinWiresSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать компоненты.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showComponents"_s)] public Bool _showComponents;
        Bool _showComponents = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showComponentsSpecified
        bool getShowComponentsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать компоненты на верхней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showCompTop"_s)] public Bool _showCompTop;
        Bool _showCompTop = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompTopSpecified
        bool getShowCompTopSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать компоненты на нижней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showCompBot"_s)] public Bool _showCompBot;
        Bool _showCompBot = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompBotSpecified
        bool getShowCompBotSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать позиционные обозначения компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showCompsDes"_s)] public Bool _showCompsDes;
        Bool _showCompsDes = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompsDesSpecified
        bool getShowCompsDesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать имена контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showPinsName"_s)] public Bool _showPinsName;
        Bool _showPinsName = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showPinsNameSpecified
        bool getShowPinsNameSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать имена цепей контактов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showPinsNet"_s)] public Bool _showPinsNet;
        Bool _showPinsNet = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showPinsNetSpecified
        bool getShowPinsNetSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать габариты компонентов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showCompsBound"_s)] public Bool _showCompsBound;
        Bool _showCompsBound = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showCompsBoundSpecified
        bool getShowCompsBoundSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки атрибута RefDes.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showLabelRefDes"_s)] public Bool _showLabelRefDes;
        Bool _showLabelRefDes = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showLabelRefDesSpecified
        bool getShowLabelRefDesSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки атрибута PartName.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showLabelPartName"_s)] public Bool _showLabelPartName;
        Bool _showLabelPartName = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showLabelPartNameSpecified
        bool getShowLabelPartNameSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать ярлыки пользовательских атрибутов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showLabelOther"_s)] public Bool _showLabelOther;
        Bool _showLabelOther = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showLabelOtherSpecified
        bool getShowLabelOtherSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать нарушения.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showViolations"_s)] public Bool _showViolations;
        Bool _showViolations = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showViolationsSpecified
        bool getShowViolationsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать уменьшение номинального зазора.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showNarrow"_s)] public Bool _showNarrow;
        Bool _showNarrow = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showNarrowSpecified
        bool getShowNarrowSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать уменьшение ширины проводника.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showTrimmed"_s)] public Bool _showTrimmed;
        Bool _showTrimmed = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showTrimmedSpecified
        bool getShowTrimmedSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать нарушение DRC.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showDRCViolations"_s)] public Bool _showDRCViolations;
        Bool _showDRCViolations = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showDRCViolationsSpecified
        bool getShowDRCViolationsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать запреты.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showKeepouts"_s)] public Bool _showKeepouts;
        Bool _showKeepouts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showKeepoutsSpecified
        bool getShowKeepoutsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать запреты трассировки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showRouteKeepouts"_s)] public Bool _showRouteKeepouts;
        Bool _showRouteKeepouts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showRouteKeepoutsSpecified
        bool getShowRouteKeepoutsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать запреты размещения.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showPlaceKeepouts"_s)] public Bool _showPlaceKeepouts;
        Bool _showPlaceKeepouts = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showPlaceKeepoutsSpecified
        bool getShowPlaceKeepoutsSpecified() const;

        /// <summary>
        /// Настройка отображения: показывать только активный слой.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showActiveLayerOnly"_s)] public Bool _showActiveLayerOnly;
        Bool _showActiveLayerOnly = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _showActiveLayerOnlySpecified
        bool getShowActiveLayerOnlySpecified() const;

        /// <summary>
        /// Настройка отображения: показывать области змеек.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"showSerpentArea"_s)] public Bool _showSerpentArea;
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
            // ORIGINAL LINE: [XmlAttribute(u"x"_s, DataType = u"float"_s)] public float _x;
            float _x = 0.0F;

            /// <summary>
            /// шаг сетки по вертикали.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute(u"y"_s, DataType = u"float"_s)] public float _y;
            float _y = 0.0F;
        };

        /// <summary>
        /// Настройка отображения сетки: цвет сетки.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"gridColor"_s)] public string _gridColor;
        QString _gridColor;

        /// <summary>
        /// Настройка отображения сетки: тип сетки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"gridKind"_s)] public gridKind _gridKind;
        gridKind _gridKind = static_cast<gridKind>(0);

        /// <summary>
        /// Настройка отображения сетки: показывать сетку.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"gridShow"_s)] public Bool _gridShow;
        Bool _gridShow = static_cast<Bool>(0);
        virtual ~Grid() {
            delete _GridSpace;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _gridShowSpecified
        bool getGridShowSpecified() const;

        /// <summary>
        /// Настройка ручного редактора: выравнивание на сетку.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"alignToGrid"_s)] public Bool _alignToGrid;
        Bool _alignToGrid = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _alignToGridSpecified
        bool getAlignToGridSpecified() const;

        /// <summary>
        /// Настройка ручного редактирования: привязка к углу кратному 45˚.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"snapToAngle"_s)] public Bool _snapToAngle;
        Bool _snapToAngle = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _snapToAngleSpecified
        bool getSnapToAngleSpecified() const;

        /// <summary>
        /// Настройка отображения сетки: шаг сетки.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"GridSpace"_s)] public GridSpace _GridSpace;
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
            // ORIGINAL LINE: [XmlAttribute(u"details"_s)] public string _details;
            QString _details;

            /// <summary>
            /// Настройка отображения слоя: цвет контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute(u"pads"_s)] public string _pads;
            QString _pads;

            /// <summary>
            /// Настройка отображения слоя: цвет зафиксированных объектов.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute(u"fix"_s)] public string _fix;
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
            // ORIGINAL LINE: [XmlAttribute(u"visible"_s)] public Bool _visible;
            Bool _visible = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _visibleSpecified
            bool getVisibleSpecified() const;

            /// <summary>
            /// Настройка отображения слоя: видимость деталей.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute(u"details"_s)] public Bool _details;
            Bool _details = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _detailsSpecified
            bool getDetailsSpecified() const;

            /// <summary>
            /// Настройка отображения слоя: видимость контактных площадок.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute(u"pads"_s)] public Bool _pads;
            Bool _pads = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _padsSpecified
            bool getPadsSpecified() const;
        };

        /// <summary>
        /// Ссылка на слой.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlElement(u"LayerRef"_s)] public LayerRef _LayerRef;
        LayerRef* _LayerRef;

        /// <summary>
        /// Настройка отображения: цветовые настройки слоя.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"Colors"_s)] public Colors_LayerOptions _Colors;
        Colors_LayerOptions* _Colors;

        /// <summary>
        /// Настройка отображения слоя: настройки видимости.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"Show"_s)] public Show_LayerOptions _Show;
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
            // ORIGINAL LINE: [XmlAttribute(u"color"_s)] public string _color;
            QString _color;

            /// <summary>
            /// Ссылка на цепь или сигнал
            /// </summary>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlElement(u"NetRef"_s, typeof(NetRef)), XmlElement(u"NetGroupRef"_s, typeof(NetGroupRef)), XmlElement(u"AllNets"_s, typeof(AllNets)), XmlElement(u"SignalRef"_s, typeof(SignalRef)), XmlElement(u"DiffSignalRef"_s, typeof(DiffSignalRef)), XmlElement(u"SignalGroupRef"_s, typeof(SignalGroupRef)),] public Object _Refs;
            std::any _Refs;
        };

        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"enabled"_s)] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для проводников.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"colorizeWire"_s)] public Bool _colorizeWire;
        Bool _colorizeWire = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeWireSpecified
        bool getColorizeWireSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"colorizePad"_s)] public Bool _colorizePad;
        Bool _colorizePad = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizePadSpecified
        bool getColorizePadSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для областей металлизации.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"colorizeCopper"_s)] public Bool _colorizeCopper;
        Bool _colorizeCopper = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeCopperSpecified
        bool getColorizeCopperSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для переходов.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"colorizeVia"_s)] public Bool _colorizeVia;
        Bool _colorizeVia = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeViaSpecified
        bool getColorizeViaSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: применять для связей.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"colorizeNetline"_s)] public Bool _colorizeNetline;
        Bool _colorizeNetline = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _colorizeNetlineSpecified
        bool getColorizeNetlineSpecified() const;

        /// <summary>
        /// Отображение цепей особым цветом: установить цвет для цепи / сигнала / группы цепей / группы сигналов.
        /// </summary>

        // ORIGINAL LINE: [XmlElement(u"SetColor"_s)] public List<SetColor> _SetColors;
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
        // ORIGINAL LINE: [XmlAttribute(u"enabled"_s)] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Ссылки на цепь или сигнал
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement(u"NetRef"_s, typeof(NetRef)), XmlElement(u"NetGroupRef"_s, typeof(NetGroupRef)), XmlElement(u"AllNets"_s, typeof(AllNets)), XmlElement(u"SignalRef"_s, typeof(SignalRef)), XmlElement(u"DiffSignalRef"_s, typeof(DiffSignalRef)), XmlElement(u"SignalGroupRef"_s, typeof(SignalGroupRef)),] public List<Object> _Refs;
        std::vector<std::any> _Refs;
        bool ShouldSerialize_Refs();
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute(u"version"_s)] public string _version;
    QString _version;

    /// <summary>
    /// Настройка отображения: параметры текущего вида.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"View"_s)] public View _View;
    View* _View;

    /// <summary>
    /// Устанавливает активный слой.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"ActiveLayer"_s)] public ActiveLayer _ActiveLayer;
    ActiveLayer* _ActiveLayer;

    /// <summary>
    /// Настройка отображения: единицы измерения.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Units"_s)] public Units_DisplayControl _Units;
    Units_DisplayControl* _Units;

    /// <summary>
    /// Настройка отображения: общие цветовые настройки.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Colors"_s)] public Colors_DisplayControl _Colors;
    Colors_DisplayControl* _Colors;

    /// <summary>
    /// Настройка отображения: настройки видимости объектов.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Show"_s)] public Show_DisplayControl _Show;
    Show_DisplayControl* _Show;

    /// <summary>
    /// Настройки сетки.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Grid"_s)] public Grid _Grid;
    Grid* _Grid;

    /// <summary>
    /// Настройка отображения: настройки видимости слоёв.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"LayersVisualOptions"_s)][XmlArrayItem(u"LayerOptions"_s)] public List<LayerOptions> _LayersVisualOptions;
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

    // ORIGINAL LINE: [XmlElement(u"ColorNets"_s)] public ColorNets _ColorNets;
    ColorNets* _ColorNets;

    /// <summary>
    /// Фильтр отображения связей.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"FilterNetlines"_s)] public FilterNetlines _FilterNetlines;
    FilterNetlines* _FilterNetlines;

    /**************************************************************************
     * Здесь находятся функции для работы с элементами класса DisplayControl. *
     * Они не являются частью формата TopoR PCB.                              *
     * ************************************************************************/

    /**************************************************************************/
};
// } // namespace TopoR_PCB_Classes

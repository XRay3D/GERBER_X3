#pragma once

#include "Commons.h"
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе u"Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г."_s.
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Раздел «Настройки дизайна».
/// </summary>
class Settings : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Настройки автоматической трассировки.
    /// </summary>
public:
    class Autoroute {
        /// <summary>
        /// Настройка автоматической трассировки: режим трассировки.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"mode"_s)] public mode_Autoroute _mode;
        mode_Autoroute _mode = static_cast<mode_Autoroute>(0);

        /// <summary>
        /// Параметр автоматической трассировки: использование функциональной эквивалентности.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"autoEqu"_s)] public autoEqu _autoEqu;
        autoEqu _autoEqu = static_cast<autoEqu>(0);

        /// <summary>
        /// Параметр автоматической трассировки: форма проводников.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"wireShape"_s)] public wireShape _wireShape;
        wireShape _wireShape = static_cast<wireShape>(0);

        /// <summary>
        /// Параметр автоматической трассировки: создавать «капельки».
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"teardrops"_s)] public Bool _teardrops;
        Bool _teardrops = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _teardropsSpecified
        bool getTeardropsSpecified() const;

        /// <summary>
        /// Параметр автоматической трассировки: ослабленный контроль зазоров.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"weakCheck"_s)] public Bool _weakCheck;
        Bool _weakCheck = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _weakCheckSpecified
        bool getWeakCheckSpecified() const;

        /// <summary>
        /// Параметр автоматической трассировки: использовать имеющуюся разводку в качестве начального варианта.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"takeCurLayout"_s)] public Bool _takeCurLayout;
        Bool _takeCurLayout = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _takeCurLayoutSpecified
        bool getTakeCurLayoutSpecified() const;

        /// <summary>
        /// Настройка автоматической трассировки: соединять планарные контакты напрямую.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"directConnectSMD"_s)] public Bool _directConnectSMD;
        Bool _directConnectSMD = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _directConnectSMDSpecified
        bool getDirectConnectSMDSpecified() const;

        /// <summary>
        /// Настройка автоматической трассировки: не дотягивать проводник до точки привязки полигонального контакта.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"dontStretchWireToPolypin"_s)] public Bool _dontStretchWireToPolypin;
        Bool _dontStretchWireToPolypin = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _dontStretchWireToPolypinSpecified
        bool getDontStretchWireToPolypinSpecified() const;
    };

    /// <summary>
    /// Настройки автоматических процедур.
    /// </summary>
public:
    class Autoproc {
        /// <summary>
        /// Настройка автоматической перекладки проводников.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"refine"_s)] public refine _refine;
        refine _refine = static_cast<refine>(0);

        /// <summary>
        /// Настройка автоматической подвижки.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"automove"_s)] public automove _automove;
        automove _automove = static_cast<automove>(0);
    };

    /// <summary>
    /// Настройки автоматического размещения компонентов.
    /// </summary>
public:
    class Placement {
        /// <summary>
        /// Настройки автоматического размещения компонентов: область размещения. Область прямоугольная, задаётся двумя вершинами(верхняя левая и правая нижняя).
        /// </summary>
    public:
        class PlacementArea {
            /// <summary>
            /// Координаты точек, вершин
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlElement(u"Dot"_s)] public List<Dot> _Dots;
            std::vector<Dot*> _Dots;
            bool ShouldSerialize_Dots();
        };

        /// <summary>
        /// Настройки автоматического размещения компонентов: область размещения. Область прямоугольная, задаётся двумя вершинами(верхняя левая и правая нижняя).
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlElement(u"PlacementArea"_s)] public PlacementArea _PlacementArea;
        PlacementArea* _PlacementArea;
        virtual ~Placement() {
            delete _PlacementArea;
        }
    };

    /// <summary>
    /// Настройки ориентации ярлыков.
    /// </summary>
public:
    class Labels_Settings {
        /// <summary>
        /// Настройка ориентации ярлыков: вращать ярлык при вращении компонента.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"rotateWithComp"_s)] public Bool _rotateWithComp;
        Bool _rotateWithComp = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _rotateWithCompSpecified
        bool getRotateWithCompSpecified() const;

        /// <summary>
        /// Настройка редактирования ярлыков: использовать правила ориентации.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"useOrientRules"_s)] public Bool _useOrientRules;
        Bool _useOrientRules = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _useOrientRulesSpecified
        bool getUseOrientRulesSpecified() const;

        /// <summary>
        /// Настройка ориентации ярлыков: поворот для ярлыков горизонтальной ориентации на верхней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"topHorzRotate"_s)] public Bool _topHorzRotate;
        Bool _topHorzRotate = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _topHorzRotateSpecified
        bool getTopHorzRotateSpecified() const;

        /// <summary>
        /// Настройка ориентации ярлыков: поворот для ярлыков вертикальной ориентации на верхней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"topVertRotate"_s)] public Bool _topVertRotate;
        Bool _topVertRotate = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _topVertRotateSpecified
        bool getTopVertRotateSpecified() const;

        /// <summary>
        /// Настройка ориентации ярлыков: поворот для ярлыков горизонтальной ориентации на нижней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"bottomHorzRotate"_s)] public Bool _bottomHorzRotate;
        Bool _bottomHorzRotate = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _bottomHorzRotateSpecified
        bool getBottomHorzRotateSpecified() const;

        /// <summary>
        /// Настройка ориентации ярлыков: поворот для ярлыков вертикальной ориентации на нижней стороне.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute(u"bottomVertRotate"_s)] public Bool _bottomVertRotate;
        Bool _bottomVertRotate = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _bottomVertRotateSpecified
        bool getBottomVertRotateSpecified() const;
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute(u"version"_s)] public string _version;
    QString _version;

    /// <summary>
    /// Настройки автоматической трассировки.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Autoroute"_s)] public Autoroute _Autoroute;
    Autoroute* _Autoroute;

    /// <summary>
    /// Настройки автоматических процедур.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Autoproc"_s)] public Autoproc _Autoproc;
    Autoproc* _Autoproc;

    /// <summary>
    /// Настройки автоматического размещения компонентов.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Placement"_s)] public Placement _Placement;
    Placement* _Placement;

    /// <summary>
    /// Настройки ориентации ярлыков.
    /// </summary>

    // ORIGINAL LINE: [XmlElement(u"Labels"_s)] public Labels_Settings _Labels;
    Labels_Settings* _Labels;

    /********************************************************************
     * Здесь находятся функции для работы с элементами класса Settings. *
     * Они не являются частью формата TopoR PCB.                        *
     * ******************************************************************/

    /********************************************************************/
    virtual ~Settings() {
        delete _Autoroute;
        delete _Autoproc;
        delete _Placement;
        delete _Labels;
    }
};
// } // namespace TopoR_PCB_Classes

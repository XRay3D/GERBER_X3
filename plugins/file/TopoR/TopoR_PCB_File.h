#pragma once

#include "ComponentsOnBoard.h"
#include "Connectivity.h"
#include "Constructive.h"
#include "DialogSettings.h"
#include "DisplayControl.h"
#include "Groups.h"
#include "Header.h"
#include "HiSpeedRules.h"
#include "Layers.h"
#include "LocalLibrary.h"
#include "NetList.h"
#include "Rules.h"
#include "Settings.h"
#include "TextStyles.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Корневой тег. Включает все разделы файла.
/// </summary>

// ORIGINAL LINE: [XmlRoot("TopoR_PCB_File ")] public class TopoR_PCB_File
class TopoR_PCB_File : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Раздел «Заголовок файла».
    /// </summary>
public:
    QS_OBJECT(Header, _Header) // ORIGINAL LINE: [XmlElement("Header")] public Header _Header;

    /// <summary>
    /// Раздел «Слои». (Обязательный раздел)
    /// </summary>
    QS_OBJECT(Layers, _Layers); // ORIGINAL LINE: [XmlElement("Layers")] public Layers _Layers;

    //    /// <summary>
    //    /// Раздел «Стили надписей».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("TextStyles")] public TextStyles _TextStyles;
    //    QS_OBJECT(TextStyles, _TextStyles);

    //    /// <summary>
    //    /// Раздел «Библиотечные элементы». (Обязательный раздел)
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("LocalLibrary")] public LocalLibrary _LocalLibrary;
    //    QS_OBJECT(LocalLibrary, _LocalLibrary);

    //    /// <summary>
    //    /// Раздел «Конструктив платы».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("Constructive")] public Constructive _Constructive;
    //    QS_OBJECT(Constructive, _Constructive);

    //    /// <summary>
    //    /// Раздел «Компоненты на плате». (Обязательный раздел).
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("ComponentsOnBoard")] public ComponentsOnBoard _ComponentsOnBoard;
    //    QS_OBJECT(ComponentsOnBoard, _ComponentsOnBoard);

    //    /// <summary>
    //    /// Раздел «Текущий список соединений».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("NetList")] public NetList _NetList;
    //    QS_OBJECT(NetList, _NetList);

    //    /// <summary>
    //    /// Раздел «Группировка объектов».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("Groups")] public Groups _Groups;
    //    QS_OBJECT(Groups, _Groups);

    //    /// <summary>
    //    /// Раздел «Правила для высокоскоростных устройств».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("HiSpeedRules")] public HiSpeedRules _HiSpeedRules;
    //    QS_OBJECT(HiSpeedRules, _HiSpeedRules);

    //    /// <summary>
    //    /// Раздел «Правила».
    //    /// </summary>
    //    /// <remarks>! Порядок следования правил в каждой секции определяет приоритет правил. Чем выше приоритет у правила, тем ниже оно описано.</remarks>

    //    // ORIGINAL LINE: [XmlElement("Rules")] public Rules _Rules;
    //    QS_OBJECT(Rules, _Rules);

    //    /// <summary>
    //    /// Раздел «Соединения на плате».
    //    /// </summary>
    //    /// <remarks>В этом разделе описывается конкретная реализация соединений: печатные проводники, межслойные переходы и области металлизации.</remarks>

    //    // ORIGINAL LINE: [XmlElement("Connectivity")] public Connectivity _Connectivity;
    //    QS_OBJECT(Connectivity, _Connectivity);

    //    /// <summary>
    //    /// Раздел «Настройки дизайна».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("Settings")] public Settings _Settings;
    //    QS_OBJECT(Settings, _Settings);

    //    /// <summary>
    //    /// Раздел «Настройки отображения».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("DisplayControl")] public DisplayControl _DisplayControl;
    //    QS_OBJECT(DisplayControl, _DisplayControl);

    //    /// <summary>
    //    /// Раздел «Настройки диалогов».
    //    /// </summary>

    //    // ORIGINAL LINE: [XmlElement("DialogSettings")] public DialogSettings _DialogSettings;
    //    QS_OBJECT(DialogSettings, _DialogSettings);
};
// } // namespace TopoR_PCB_Classes

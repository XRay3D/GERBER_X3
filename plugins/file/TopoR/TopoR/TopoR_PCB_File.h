#pragma once

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

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

namespace TopoR {

// Корневой тег TopoR_PCB_File. Включает все разделы файла.
struct TopoR_PCB_File {
    // Раздел «Заголовок файла».
    Header header;
    // Раздел «Слои». (Обязательный раздел)
    Layers layers;
    // Раздел «Стили надписей».
    TextStyles textStyles;
    // Раздел «Библиотечные элементы». (Обязательный раздел)
    LocalLibrary localLibrary;
    // Раздел «Конструктив платы».
    Constructive constructive;
    // Раздел «Компоненты на плате». (Обязательный раздел).
    ComponentsOnBoard componentsOnBoard;
    // Раздел «Текущий список соединений».
    NetList netList;
    // Раздел «Группировка объектов».
    Groups groups;
    // Раздел «Правила для высокоскоростных устройств».
    HiSpeedRules hiSpeedRules;
    // Раздел «ПРАВИЛА»
    /// \note !Порядок следования правил в каждой секции определяет приоритет правил. Чем выше приоритет у правила, тем ниже оно описано.
    Rules rules;
    // Раздел «Соединения на плате».
    /// \note В этом разделе описывается конкретная реализация соединений: печатные проводники, межслойные переходы и области металлизации.
    Connectivity connectivity;
    // Раздел «Настройки дизайна».
    Settings settings;
    // Раздел «Настройки отображения».
    DisplayControl displayControl;
    // Раздел «Настройки диалогов».
    DialogSettings dialogSettings;
};

} // namespace TopoR

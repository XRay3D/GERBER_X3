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
namespace TopoR {
// Корневой тег. Включает все разделы файла.
struct[[= XML::Root]] TopoR_PCB_File {
    // Раздел «Заголовок файла».
    Header Header;
    // Раздел «Слои». (Обязательный раздел)
    Layers Layers;
    // Раздел «Стили надписей».
    TextStyles TextStyles;
    // Раздел «Библиотечные элементы». (Обязательный раздел)
    LocalLibrary LocalLibrary;
    // Раздел «Конструктив платы».
    Constructive Constructive;
    // Раздел «Компоненты на плате». (Обязательный раздел).
    ComponentsOnBoard ComponentsOnBoard;
    // Раздел «Текущий список соединений».
    NetList NetList;
    // Раздел «Группировка объектов».
    Groups Groups;
    // Раздел «Правила для высокоскоростных устройств».
    HiSpeedRules HiSpeedRules;
    // Раздел «Правила».
    // ! Порядок следования правил в каждой секции определяет приоритет правил. Чем выше приоритет у правила, тем ниже оно описано.
    Rules Rules;
    // Раздел «Соединения на плате».
    // В этом разделе описывается конкретная реализация соединений: печатные проводники, межслойные переходы и области металлизации.
    Connectivity Connectivity;
    // Раздел «Настройки дизайна».
    Settings Settings;
    // Раздел «Настройки отображения».
    DisplayControl DisplayControl;
    // Раздел «Настройки диалогов».
    DialogSettings DialogSettings;
};
} // namespace TopoR

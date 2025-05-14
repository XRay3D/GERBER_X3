#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Заголовок файла».
struct Header {
    // Название формата файла. Xml::Element
    QString Format;
    // Версия формата. Xml::Element
    QString Version;
    // Название программы, создавшей файл. Xml::Element
    QString Program;
    // Дата и время создания файла (в произвольной форме). Xml::Element
    QString Date;
    // Формат импортированного файла, из которого был получен дизайн. Xml::Element
    QString OriginalFormat;
    // Импортированный файл. Путь к файлу задаётся относительно каталога содержащего файл проекта. Xml::Element
    QString OriginalFile;
    // Единицы измерения для всего файла. Xml::Element
    struct Units {
        // Единицы измерения длины для всего файла. Значение по умолчанию – mm (миллиметр).
        Xml::Attr<dist, NoOpt> dist_;
        // Единицы измерения времени для всего файла. Значение по умолчанию – ps (пикосекунда).
        Xml::Attr<time, NoOpt> time_;
    } units;
};

} // namespace TopoR

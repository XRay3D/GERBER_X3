#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
struct Header {                              // Раздел «Заголовок файла».
    struct Units {                           // Единицы измерений для всего файла.
        [[= XML::AttrF]] dist dist{}; // Единицы измерения длины для всего файла. Значение по умолчанию – mm (миллиметр).
        [[= XML::AttrF]] time time{}; // Единицы измерения времени для всего файла. Значение по умолчанию – ps (пикосекунда).
    };
    /*[[= XML::Elem]]*/ std::string Format;         // Название формата файла.
    /*[[= XML::Elem]]*/ std::string Version;        // Версия формата.
    /*[[= XML::Elem]]*/ std::string Program;        // Название программы, создавшей файл.
    /*[[= XML::Elem]]*/ std::string Date;           // Дата и время создания файла (в произвольной форме).
    /*[[= XML::Elem]]*/ std::string OriginalFormat; // Формат импортированного файла, из которого был получен дизайн.
    /*[[= XML::Elem]]*/ std::string OriginalFile;   // Импортированный файл. Путь к файлу задаётся относительно каталога содержащего файл проекта.
    /*[[= XML::ElemF]]*/ Units Units;                   // Единицы измерения для всего файла.
};
} // namespace TopoR

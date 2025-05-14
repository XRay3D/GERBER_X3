#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Стили надписей».
struct TextStyles {
    // Описание стиля надписей.
    struct TextStyle {
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Параметр стиля надписей: название шрифта.
        Xml::Attr<QString> fontName;
        // Параметр стиля надписей: высота символов в текущих единицах.
        Xml::Attr<double> height;
        // Параметр стиля надписей: жирность шрифта.
        Xml::Attr<Bool> bold;
        // Параметр стиля надписей: курсив.
        Xml::Attr<Bool> italic;
    };

    // Версия раздела.
    Xml::Attr<QString> version;
    // Стили надписей.
    Xml::Array<TextStyle> textStyles;

    /**********************************************************************
     * Здесь находятся функции для работы с элементами класса TextStyles. *
     * Они не являются частью формата TopoR PCB.                          *
     * ********************************************************************/

    const TextStyle* getTextStyle(const QString& name) const;
    /**********************************************************************/
};


} // namespace TopoR

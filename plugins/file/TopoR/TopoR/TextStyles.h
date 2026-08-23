#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Стили надписей».
struct TextStyles {
    // Описание стиля надписей.
    struct TextStyle {
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name;
        // Параметр стиля надписей: название шрифта.
        [[= XML::Attr]] std::string fontName;
        // Параметр стиля надписей: высота символов в текущих единицах.
        [[= XML::Attr]] double height{};
        // Параметр стиля надписей: жирность шрифта.
        [[= XML::Attr]] Bool bold{};
        // Параметр стиля надписей: курсив.
        [[= XML::Attr]] Bool italic{};
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Стили надписей.
    [[= XML::Elem]] std::vector<TextStyle> TextStyles;
    // bool ShouldSerialize_TextStyles();

    std::optional<const TextStyle&> getTextStyle(std::string_view name) const {
        auto ps = r::find(TextStyles, name, &TextStyle::name);
        if(ps != TextStyles.end()) return *ps.base();
        return {};
    }

    /**********************************************************************
     * Здесь находятся функции для работы с элементами класса TextStyles. *
     * Они не являются частью формата TopoR PCB.                          *
     * ********************************************************************/
    /**********************************************************************/
};
} // namespace TopoR

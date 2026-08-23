#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Текущий список соединений».
struct NetList {
    // Раздел «Текущий список соединени
    struct Net {
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name;
        // Ссылка на контакт или вывод посадочного места (объект класса PinRef или PadRef).
        // public List<Object> refs;
        [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ PinRef, PadRef>> refs;
        bool ShouldSerialize_refs();
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Описания цепей.
    [[= XML::Elem]] std::vector<Net> Nets;
    bool ShouldSerialize_Nets();
    /*******************************************************************
     * Здесь находятся функции для работы с элементами класса NetList. *
     * Они не являются частью формата TopoR PCB.                       *
     * *****************************************************************/
    // Переименование ссылок на компонент, если его имя изменилось
    // <param name="oldname">старое имя компонента</param>   // <param name="newname">новое имя компонента</param>
    void Rename_compName(const std::string& oldname, const std::string& newname);
    /*******************************************************************/
};
} // namespace TopoR

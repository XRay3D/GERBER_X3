#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Текущий список соединений».
struct NetList {
    // Раздел «Текущий список соединени
    struct Net {
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Ссылка на контакт или вывод посадочного места (объект класса PinRef или PadRef).
        Xml::Array<Xml::Variant<PinRef, PadRef>> refs;
    };
    // Версия раздела.
    Xml::Attr<QString> version;
    // Описания цепей.
    Xml::Array<Net> Nets;
    /*******************************************************************
     * Здесь находятся функции для работы с элементами класса Nets. *
     * Они не являются частью формата TopoR PCB.                       *
     * *****************************************************************/
    // Переименование ссылок на компонент, если его имя изменилось
    /// \param oldname \brief старое имя компонента
    /// \param newname \brief новое имя компонента
    void Rename_compName(const QString& oldname, const QString& newname);
    /*******************************************************************/
};

} // namespace TopoR

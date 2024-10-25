#pragma once

#include "Commons.h"
#include <any>
#include <string>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Раздел «Текущий список соединений».
/// </summary>
class NetList : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Раздел «Текущий список соединени
    /// </summary>
public:
    class Net {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Ссылка на контакт или вывод посадочного места (объект класса PinRef или PadRef).
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("PinRef", typeof(PinRef)), XmlElement("PadRef", typeof(PadRef))] public List<Object> _refs;+

        std::vector<std::variant<PinRef, PadRef>> _refs;
        bool ShouldSerialize_refs();
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Описания цепей.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("Net")] public List<Net> _Nets;
    std::vector<Net*> _Nets;
    bool ShouldSerialize_Nets();

    /*******************************************************************
     * Здесь находятся функции для работы с элементами класса NetList. *
     * Они не являются частью формата TopoR PCB.                       *
     * *****************************************************************/

    /// <summary>
    /// Переименование ссылок на компонент, если его имя изменилось
    /// </summary>
    /// <param name="oldname">старое имя компонента</param>
    /// <param name="newname">новое имя компонента</param>
    void Rename_compName(const QString& oldname, const QString& newname);
    /*******************************************************************/
};
// } // namespace TopoR_PCB_Classes

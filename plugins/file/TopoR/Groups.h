#pragma once

#include "Commons.h"
#include <QString>
#include <string>
#include <variant>
#include <vector>

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе u"Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г."_s.
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */

// namespace TopoR_PCB_Classes {
/// <summary>
/// Раздел «Группировка объектов».
/// </summary>
class Groups : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Описание групп слоёв.
    /// </summary>
public:
    class LayerGroup {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"name"_s)] public string _name;
        QString _name;

        /// <summary>
        /// Ссылка на слой или ссылка на группу слоёв.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement(u"LayerRef"_s, typeof(LayerRef)), XmlElement(u"LayerGroupRef"_s, typeof(LayerGroupRef))] public List<Object> _LayerRefs;
        std::vector<std::variant<LayerRef, LayerGroupRef>> _LayerRefs;
        bool ShouldSerialize_LayerRefs();
        /**********************************************************************
         * Здесь находятся функции для работы с элементами класса LayerGroup. *
         * Они не являются частью формата TopoR PCB.                          *
         * *******************************************************************/
        QString ToString();
        /*********************************************************************/
    };

    /// <summary>
    /// Описание группы цепей.
    /// </summary>
public:
    class NetGroup {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"name"_s)] public string _name;
        QString _name;

        /// <summary>
        /// Ссылка на цепь или ссылка на группу цепей.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement(u"NetRef"_s, typeof(NetRef)), XmlElement(u"NetGroupRef"_s, typeof(NetGroupRef))] public List<Object> _NetRefs;
        std::vector<std::variant<NetRef, NetGroupRef>> _NetRefs;
        bool ShouldSerialize_NetRefs();
    };

    /// <summary>
    /// Описание группы компонентов.
    /// </summary>
public:
    class CompGroup {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute(u"name"_s)] public string _name;
        QString _name;

        /// <summary>
        /// Ссылка на компонент на плате или ссылка на группу компонентов.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement(u"CompInstanceRef"_s, typeof(CompInstanceRef)), XmlElement(u"CompGroupRef"_s, typeof(CompGroupRef))] public List<Object> _CompRefs;
        std::vector<std::variant<CompInstanceRef, CompGroupRef>> _CompRefs;
        bool ShouldSerialize_CompRefs();
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute(u"version"_s)] public string _version;
    QString _version;

    /// <summary>
    /// Группы слоёв.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"LayerGroups"_s)][XmlArrayItem(u"LayerGroup"_s)] public List<LayerGroup> _LayerGroups;
    std::vector<LayerGroup*> _LayerGroups;
    bool ShouldSerialize_LayerGroups();
    /// <summary>
    /// Группы цепей.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"NetGroups"_s)][XmlArrayItem(u"NetGroup"_s)] public List<NetGroup> _NetGroups;
    std::vector<NetGroup*> _NetGroups;
    bool ShouldSerialize_NetGroups();
    /// <summary>
    /// Группы компонентов.
    /// </summary>

    // ORIGINAL LINE: [XmlArray(u"CompGroups"_s)][XmlArrayItem(u"CompGroup"_s)] public List<CompGroup> _CompGroups;
    std::vector<CompGroup*> _CompGroups;
    bool ShouldSerialize_CompGroups();

    /******************************************************************
     * Здесь находятся функции для работы с элементами класса Groups. *
     * Они не являются частью формата TopoR PCB.                      *
     * ****************************************************************/

    /// <summary>
    /// Переименование ссылок на компонент, если его имя изменилось
    /// </summary>
    /// <param name=u"oldname"_s>старое имя компонента</param>
    /// <param name=u"newname"_s>новое имя компонента</param>
    void Rename_compName(const QString& oldname, const QString& newname);
    /******************************************************************/
};
// } // namespace TopoR_PCB_Classes

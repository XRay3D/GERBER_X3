#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Группировка объектов».
struct Groups {
    // Описание групп слоёв.
    struct LayerGroup {
        // Имя объекта или ссылка на именованный объект.
        // public string name;
        [[= XML::Attr]] std::string name;
        // Ссылка на слой или ссылка на группу слоёв.
        // public List<Object> LayerRefs;
        [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ LayerRef, LayerGroupRef>> LayerRefs;
        bool ShouldSerializeLayerRefs();
        /**********************************************************************
         * Здесь находятся функции для работы с элементами класса LayerGroup. *
         * Они не являются частью формата TopoR PCB.                          *
         * *******************************************************************/
        std::string ToString();
        /*********************************************************************/
    };
    // Описание группы цепей.
    struct NetGroup {
        // Имя объекта или ссылка на именованный объект.
        // public string name;
        [[= XML::Attr]] std::string name;
        // Ссылка на цепь или ссылка на группу цепей.
        // public List<Object> NetRefs;
        [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ NetRef, NetGroupRef>> NetRefs;
        bool ShouldSerialize_NetRefs();
    };
    // Описание группы компонентов.
    struct CompGroup {
        // Имя объекта или ссылка на именованный объект.
        // public string name;
        [[= XML::Attr]] std::string name;
        // Ссылка на компонент на плате или ссылка на группу компонентов.
        // public List<Object> CompRefs;
        [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ CompInstanceRef, CompGroupRef>> CompRefs;
        bool ShouldSerialize_CompRefs();
    };
    // Версия раздела.
    // public string version;
    [[= XML::Attr]] std::string version;
    // Группы слоёв.
    //[XmlArrayItem("LayerGroup")] public List<LayerGroup> LayerGroups;
    [[= XML::Array]] std::vector<LayerGroup> LayerGroups;
    bool ShouldSerialize_LayerGroups();
    // Группы цепей.
    //[XmlArrayItem("NetGroup")] public List<NetGroup> NetGroups;
    [[= XML::Array]] std::vector<NetGroup> NetGroups;
    bool ShouldSerialize_NetGroups();
    // Группы компонентов.
    //[XmlArrayItem("CompGroup")] public List<CompGroup> CompGroups;
    [[= XML::Array]] std::vector<CompGroup> CompGroups;
    bool ShouldSerialize_CompGroups();
    /******************************************************************
     * Здесь находятся функции для работы с элементами класса Groups. *
     * Они не являются частью формата TopoR PCB.                      *
     * ****************************************************************/
    // Переименование ссылок на компонент, если его имя изменилось
    void Rename_compName(const std::string& oldname, const std::string& newname);
    /******************************************************************/
};
} // namespace TopoR

#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Правила».
// ! Порядок следования правил в каждой секции определяет приоритет правил. Чем выше приоритет у правила, тем ниже оно описано.
struct Rules {
    // Описание правила ширины проводников.
    struct WidthOfWires {
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Параметр правила ширины проводников: минимальная ширина проводника.
        [[= XML::Attr]] double widthMin{};
        // Параметр правила ширины проводников: номинальная ширина проводника.
        [[= XML::Attr]] double widthNom{};
        // Ссылка на слои. См. также LayersRefs
        // ! При null необходимо смотреть LayersRefs - там описан список ссылок типа LayerRef.
        // public Object LayersRef;
        /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ AllLayers, AllLayersInner, AllLayersInnerSignal, AllLayersSignal, AllLayersOuter, LayerGroupRef> LayersRef;
        // Ссылка на слои. См. также LayersRef
        // ! При null необходимо смотреть LayersRef - там описаны ссылки остальных типов.
        //[[= XML::Elem("LayerRef")]] // public List<LayerRef> LayersRefs;
        [[= XML::Elem]] std::vector<LayerRef> LayersRefs;
        bool ShouldSerialize_LayersRefs();
        // Объекты воздействия правила.
        //[XmlArrayItem("NetRef"), XmlArrayItem("NetGroupRef"), XmlArrayItem("AllNets")] public List<Object> ObjectsAffected;
        [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ NetRef, NetGroupRef, AllNets>> ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };
    // Описание правила зазоров между цепями.
    struct ClearanceNetToNet {
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Параметр правила зазоров между цепями: минимальный зазор.
        [[= XML::Attr]] double clrnMin{};
        // Параметр правила зазоров между цепями: номинальный зазор.
        [[= XML::Attr]] double clrnNom{};
        // Ссылка на слои. См. также LayersRefs
        // ! При null необходимо смотреть LayersRefs - там описан список ссылок типа LayerRef.
        // public Object LayersRef;
        /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ AllLayers, AllLayersInner, AllLayersInnerSignal, AllLayersSignal, AllLayersOuter, LayerGroupRef> LayersRef;
        // Ссылка на слои. См. также LayersRef
        // ! При null необходимо смотреть LayersRef - там описаны ссылки остальных типов.
        //[[= XML::Elem("LayerRef")]] // public List<LayerRef> LayersRefs;
        [[= XML::Elem]] std::vector<LayerRef> LayersRefs;
        bool ShouldSerialize_LayersRefs();
        // Объекты воздействия правила.
        //[XmlArrayItem("NetRef"), XmlArrayItem("NetGroupRef"), XmlArrayItem("AllNets"), XmlArrayItem("SignalRef"), XmlArrayItem("DiffSignalRef"), XmlArrayItem("SignalGroupRef")] public List<Object> ObjectsAffected;
        [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ NetRef, NetGroupRef, AllNets, SignalRef, DiffSignalRef, SignalGroupRef>> ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };
    // Описание правила зазоров между компонентами.
    struct ClearanceCompToComp {
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Параметр правила зазоров между цепями: минимальный зазор.
        [[= XML::Attr]] double clrn{};
        // Объекты воздействия правила.
        //[XmlArrayItem("ComponentRef"), XmlArrayItem("CompGroupRef"), XmlArrayItem("AllComps")] public List<Object> ObjectsAffected;
        [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ ComponentRef, CompGroupRef, AllComps>> ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };
    // Описание зазоров до края платы.
    struct RulesClearancesToBoard {
        // Устанавливает зазор от проводников до края платы.
        [[= XML::Attr]] double clrn{};
        // Устанавливает зазор от компонентов до края платы.
        [[= XML::Attr]] double comps{};
        [[= XML::Attr]] double wires{};
    };
    // Описание правила назначения цепям стеков переходных отверстий.
    struct ViastacksOfNets {
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Объекты воздействия правила.
        //[XmlArrayItem("NetRef"), XmlArrayItem("NetGroupRef"), XmlArrayItem("AllNets"), XmlArrayItem("SignalRef"), XmlArrayItem("DiffSignalRef"), XmlArrayItem("SignalGroupRef")] public List<Object> ObjectsAffected;
        [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ NetRef, NetGroupRef, AllNets, SignalRef, DiffSignalRef, SignalGroupRef>> ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
        // Назначенные типы переходных отверстий.
        //[XmlArrayItem("AllViastacks"), XmlArrayItem("AllViastacksThrough"), XmlArrayItem("AllViastacksNotThrough"), XmlArrayItem("ViastackRef")] public List<Object> Viastacks;
        [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ AllViastacks, AllViastacksThrough, AllViastacksNotThrough, ViastackRef>> Viastacks;
        bool ShouldSerialize_Viastacks();
    };
    // Описание правила назначения цепям опорных слоёв.
    struct PlaneLayerNets {
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Ссылка на слои. См. также LayersRefs
        // ! При null необходимо смотреть LayersRefs - там описан список ссылок типа LayerRef.
        // public Object LayersRef;
        /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ AllLayers, AllLayersInner, AllLayersInnerSignal, AllLayersSignal, AllLayersOuter, LayerGroupRef> LayersRef;
        // Ссылка на слои. См. также LayersRef
        // ! При null необходимо смотреть LayersRef - там описаны ссылки остальных типов.
        //[[= XML::Elem("LayerRef")]] // public List<LayerRef> LayersRefs;
        [[= XML::Elem("LayerRef")]] std::vector<LayerRef> LayersRefs;
        bool ShouldSerialize_LayersRefs();
        // Объекты воздействия правила.
        //[XmlArrayItem("NetRef")] public List<NetRef> ObjectsAffected;
        [[= XML::Array]] std::vector<NetRef> ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };
    // Описание правила назначения цепям сигнальных слоёв.
    struct SignalLayerNets {
        // Флаг применения правила.
        [[= XML::Attr]] Bool enabled{};
        // public bool enabledSpecified
        bool getEnabledSpecified() const;
        // Ссылка на слои. См. также LayersRefs
        // ! При null необходимо смотреть LayersRefs - там описан список ссылок типа LayerRef.
        // public Object LayersRef;
        /*[[= XML::Elem]]*/ std::variant</*XML::Null,*/ AllLayers, AllLayersInner, AllLayersInnerSignal, AllLayersSignal, AllLayersOuter, LayerGroupRef> LayersRef;
        // Ссылка на слои. См. также LayersRef
        // ! При null необходимо смотреть LayersRef - там описаны ссылки остальных типов.
        //[[= XML::Elem("LayerRef")]] // public List<LayerRef> LayersRefs;
        [[= XML::Elem("LayerRef")]] std::vector<LayerRef> LayersRefs;
        bool ShouldSerialize_LayersRefs();
        // Объекты воздействия правила.
        //[XmlArrayItem("NetRef"), XmlArrayItem("NetGroupRef")] public List<Object> ObjectsAffected;
        [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ NetRef, NetGroupRef>> ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };
    // Свойства цепи.
    struct NetProperty {
        // Свойство цепи: гибкая фиксация.
        [[= XML::Attr]] Bool flexfix{};
        // public bool flexfixSpecified
        bool getFlexfixSpecified() const;
        // Свойство цепи: флаг трассировки для автоматического трассировщика.
        [[= XML::Attr]] Bool route{};
        // public bool routeSpecified
        bool getRouteSpecified() const;
        // Ссылка на цепь.
        //[[= XML::Elem("NetRef")]] // public List<NetRef> NetRefs;
        [[= XML::Elem("NetRef")]] std::vector<NetRef> NetRefs;
        bool ShouldSerialize_NetRefs();
    };
    // Настройки подключения к углам прямоугольных контактных площадок.
    struct PadConnectSettings {
        // Настройка подключения к углам прямоугольных контактных площадок: режим подключения.
        [[= XML::Attr("mode")]] PadConnectSettingsMode mode{};
        // Ссылки на стеки контактных площадок.
        //[[= XML::Elem("PadstackRef")]] // public List<PadstackRef> PadstackRefs;
        [[= XML::Elem("PadstackRef")]] std::vector<PadstackRef> PadstackRefs;
        bool ShouldSerializePadstackRefs();
        // Ссылки на контакты.
        //[[= XML::Elem("PinRef")]] // public List<PinRef> PinRefs;
        [[= XML::Elem("PinRef")]] std::vector<PinRef> PinRefs;
        bool ShouldSerialize_PinRefs();
        // Ссылки на выводы посадочных мест.
        //[[= XML::Elem("PadRef")]] // public List<PadRef> PadRefs;
        [[= XML::Elem("PadRef")]] std::vector<PadRef> PadRefs;
        bool ShouldSerialize_PadRefs();
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Правила ширин проводников.
    //[XmlArrayItem("WidthOfWires")] public List<WidthOfWires> RulesWidthOfWires;
    [[= XML::Array]] std::vector<WidthOfWires> RulesWidthOfWires;
    bool ShouldSerialize_RulesWidthOfWires();
    // Правила зазоров между цепями.
    //[XmlArrayItem("ClearanceNetToNet")] public List<ClearanceNetToNet> RulesClearancesNetToNet;
    [[= XML::Array]] std::vector<ClearanceNetToNet> RulesClearancesNetToNet;
    bool ShouldSerialize_RulesClearancesNetToNet();
    // Правила зазоров между компонентами.
    //[XmlArrayItem("ClearanceCompToComp")] public List<ClearanceCompToComp> RulesClearancesCompToComp;
    [[= XML::Array]] std::vector<ClearanceCompToComp> RulesClearancesCompToComp;
    bool ShouldSerialize_RulesClearancesCompToComp();
    // Правило зазоров до края платы.
    [[= XML::Elem("RulesClearancesToBoard")]] RulesClearancesToBoard RulesClearancesToBoard;
    // Правила назначения цепям стеков переходных отверстий.
    //[XmlArrayItem("ViastacksOfNets")] public List<ViastacksOfNets> RulesViastacksOfNets;
    [[= XML::Array]] std::vector<ViastacksOfNets> RulesViastacksOfNets;
    bool ShouldSerialize_RulesViastacksOfNets();
    // Правила назначения цепям опорных слоёв.
    //[XmlArrayItem("PlaneLayerNets")] public List<PlaneLayerNets> RulesPlaneLayersNets;
    [[= XML::Array]] std::vector<PlaneLayerNets> RulesPlaneLayersNets;
    bool ShouldSerialize_RulesPlaneLayersNets();
    // Правила назначения цепям сигнальных слоёв.
    //[XmlArrayItem("SignalLayerNets")] public List<SignalLayerNets> RulesSignalLayersNets;
    [[= XML::Array]] std::vector<SignalLayerNets> RulesSignalLayersNets;
    bool ShouldSerialize_RulesSignalLayersNets();
    // Свойства цепей
    //[XmlArrayItem("NetProperty")] public List<NetProperty> NetProperties;
    [[= XML::Array]] std::vector<NetProperty> NetProperties;
    bool ShouldSerialize_NetProperties();
    // Настройки подключения к углам прямоугольных контактных площадок.
    [[= XML::Elem("PadConnectSettings")]] PadConnectSettings PadConnectSettings;
    /*****************************************************************
     * Здесь находятся функции для работы с элементами класса Rules. *
     * Они не являются частью формата TopoR PCB.                     *
     * ***************************************************************/
    // Переименование ссылок на компонент, если его имя изменилось
    void Rename_compName(const std::string& oldname, const std::string& newname);
};
} // namespace TopoR

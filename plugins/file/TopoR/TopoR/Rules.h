#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Правила».
struct Rules {
    /// \note !Порядок следования правил в каждой секции определяет приоритет правил. Чем выше приоритет у правила, тем ниже оно описано.
    using VariantAllLayers = Xml::Variant<AllLayers, AllLayersInner, AllLayersInnerSignal, AllLayersSignal, AllLayersOuter, LayerGroupRef>;
    // Описание правила ширины проводников.
    struct WidthOfWires {
        // Флаг применения правила.
        Xml::Attr<Bool> enabled;
        // Параметр правила ширины проводников: минимальная ширина проводника.
        Xml::Attr<double> widthMin;
        // Параметр правила ширины проводников: номинальная ширина проводника.
        Xml::Attr<double> widthNom;
        // Ссылка на слои. См. также LayersRefs
        /// \note !При null необходимо смотреть LayersRefs - там описан список ссылок типа LayerRef.
        VariantAllLayers LayersRef;
        // Ссылка на слои. См. также LayersRef
        /// \note !При null необходимо смотреть LayersRef - там описаны ссылки остальных типов.
        Xml::ArrayElem<LayerRef> LayersRefs;
        // Объекты воздействия правила.
        Xml::ArrayElem<Xml::Variant<NetRef, NetGroupRef, AllNets>> ObjectsAffected;
    };
    // Описание правила зазоров между цепями.
    struct ClearanceNetToNet {
        // Флаг применения правила.
        Xml::Attr<Bool> enabled;
        // Параметр правила зазоров между цепями: минимальный зазор.
        Xml::Attr<double> clrnMin;
        // Параметр правила зазоров между цепями: номинальный зазор.
        Xml::Attr<double> clrnNom;
        // Ссылка на слои. См. также LayersRefs_
        /// \note !При null необходимо смотреть LayersRefs_ - там описан список ссылок типа LayerRef.
        VariantAllLayers LayersRef;
        // Ссылка на слои. См. также LayersRef_
        /// \note !При null необходимо смотреть LayersRef_ - там описаны ссылки остальных типов.
        Xml::Array<LayerRef> LayersRefs;
        // Объекты воздействия правила.
        Xml::ArrayElem<Xml::Variant<NetRef, NetGroupRef, AllNets, SignalRef, DiffSignalRef, SignalGroupRef>> ObjectsAffected;
    };
    // Описание правила зазоров между компонентами.
    struct ClearanceCompToComp {
        // Флаг применения правила.
        Xml::Attr<Bool> enabled;
        // Параметр правила зазоров между цепями: минимальный зазор.
        Xml::Attr<double> clrn;
        // Объекты воздействия правила.
        Xml::ArrayElem<Xml::Variant<ComponentRef, CompGroupRef, AllComps>> ObjectsAffected;
    };
    // Описание зазоров до края платы.
    struct RulesClearancesToBoard {
        // Устанавливает зазор от проводников до края платы.
        Xml::Attr<double> wires;
        // Устанавливает зазор от компонентов до края платы.
        Xml::Attr<double> comps;
    };
    // Описание правила назначения цепям стеков переходных отверстий.
    struct ViastacksOfNets {
        // Флаг применения правила.
        Xml::Attr<Bool> enabled;
        // Объекты воздействия правила.
        Xml::ArrayElem<Xml::Variant<NetRef, NetGroupRef, AllNets, SignalRef, DiffSignalRef, SignalGroupRef>> ObjectsAffected;
        // Назначенные типы переходных отверстий.
        Xml::ArrayElem<Xml::Variant<AllViastacks, AllViastacksThrough, AllViastacksNotThrough, ViastackRef>> Viastacks;
    };
    // Описание правила назначения цепям опорных слоёв.
    struct PlaneLayerNets {
        // Флаг применения правила.
        Xml::Attr<Bool> enabled;
        // Ссылка на слои. См. также LayersRefs_
        /// \note !При null необходимо смотреть LayersRefs_ - там описан список ссылок типа LayerRef.
        VariantAllLayers LayerRef_;
        // Ссылка на слои. См. также LayersRef_
        /// \note !При null необходимо смотреть LayersRef_ - там описаны ссылки остальных типов.
        Xml::Array<LayerRef> LayersRefs;
        // Объекты воздействия правила.
        Xml::ArrayElem<NetRef> ObjectsAffected;
    };
    // Описание правила назначения цепям сигнальных слоёв.
    struct SignalLayerNets {
        // Флаг применения правила.
        Xml::Attr<Bool> enabled;
        // Ссылка на слои. См. также LayersRefs_
        /// \note !При null необходимо смотреть LayersRefs_ - там описан список ссылок типа LayerRef.
        VariantAllLayers LayersRef;
        // Ссылка на слои. См. также LayersRef_
        /// \note !При null необходимо смотреть LayersRef_ - там описаны ссылки остальных типов.
        Xml::Array<LayerRef> LayersRefs;
        // Объекты воздействия правила.
        Xml::ArrayElem<Xml::Variant<NetRef, NetGroupRef>> ObjectsAffected;
    };
    // Свойства цепи.
    struct NetProperty {
        // Свойство цепи: гибкая фиксация.
        Xml::Attr<Bool, NoOpt> flexfix;
        // Свойство цепи: флаг трассировки для автоматического трассировщика.
        Xml::Attr<Bool> route;
        // Ссылка на цепь.
        Xml::Array<NetRef> NetRefs;
    };
    // Настройки подключения к углам прямоугольных контактных площадок.
    struct PadConnectSettings {
        // Настройка подключения к углам прямоугольных контактных площадок: режим подключения.
        Xml::Attr<mode_PadConnectSettings, NoOpt> mode;
        // Ссылки на стеки контактных площадок.
        Xml::Array<PadstackRef> PadstackRefs;
        // Ссылки на контакты.
        Xml::ArrayElem<PinRef> PinRefs; // NOTE ???  Xml::Array
        // Ссылки на выводы посадочных мест.
        Xml::ArrayElem<PadRef> PadRefs; // NOTE ???  Xml::Array
    };
    // Версия раздела.
    Xml::Attr<QString> version;
    // Правила ширин проводников.
    Xml::ArrayElem<WidthOfWires> RulesWidthOfWires;
    // Правила зазоров между цепями.
    Xml::ArrayElem<ClearanceNetToNet> RulesClearancesNetToNet;
    // Правила зазоров между компонентами.
    Xml::ArrayElem<ClearanceCompToComp> RulesClearancesCompToComp;
    // Правило зазоров до края платы.
    RulesClearancesToBoard rulesClearancesToBoard;
    // Правила назначения цепям стеков переходных отверстий.
    Xml::ArrayElem<ViastacksOfNets> RulesViastacksOfNets;
    // Правила назначения цепям опорных слоёв.
    Xml::ArrayElem<PlaneLayerNets> RulesPlaneLayersNets;
    // Правила назначения цепям сигнальных слоёв.
    Xml::ArrayElem<SignalLayerNets> RulesSignalLayersNets;
    // Свойства цепей
    Xml::ArrayElem<NetProperty> NetProperties;
    // Настройки подключения к углам прямоугольных контактных площадок.
    PadConnectSettings padConnectSettings;

    /*****************************************************************
     * Здесь находятся функции для работы с элементами класса Rules. *
     * Они не являются частью формата TopoR PCB.                     *
     * ***************************************************************/
    // Переименование ссылок на компонент, если его имя изменилось
    /// \param '1 \brief старое имя компонента
    /// \param '1 \brief новое имя компонента
    void Rename_compName(const QString& oldname, const QString& newname);
};

} // namespace TopoR

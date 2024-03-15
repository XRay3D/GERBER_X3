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
/// Раздел «Правила».
/// </summary>
/// <remarks>! Порядок следования правил в каждой секции определяет приоритет правил. Чем выше приоритет у правила, тем ниже оно описано.</remarks>
class Rules : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Описание правила ширины проводников.
    /// </summary>
public:
    class WidthOfWires {
        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Параметр правила ширины проводников: минимальная ширина проводника.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("widthMin", DataType = "float")] public float _widthMin;
        float _widthMin = 0.0F;

        /// <summary>
        /// Параметр правила ширины проводников: номинальная ширина проводника.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("widthNom", DataType = "float")] public float _widthNom;
        float _widthNom = 0.0F;

        /// <summary>
        /// Ссылка на слои. См. также _LayersRefs
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRefs - там описан список ссылок типа LayerRef. </remarks>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("AllLayers", typeof(AllLayers)), XmlElement("AllLayersInner", typeof(AllLayersInner)), XmlElement("AllLayersInnerSignal", typeof(AllLayersInnerSignal)), XmlElement("AllLayersSignal", typeof(AllLayersSignal)), XmlElement("AllLayersOuter", typeof(AllLayersOuter)), XmlElement("LayerGroupRef", typeof(LayerGroupRef))] public Object _LayersRef;
        std::any _LayersRef;
        /// <summary>
        /// Ссылка на слои. См. также _LayersRef
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRef - там описаны ссылки остальных типов. </remarks>

        // ORIGINAL LINE: [XmlElement("LayerRef")] public List<LayerRef> _LayersRefs;
        std::vector<LayerRef*> _LayersRefs;
        bool ShouldSerialize_LayersRefs();
        /// <summary>
        /// Объекты воздействия правила.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("NetRef", typeof(NetRef)), XmlArrayItem("NetGroupRef", typeof(NetGroupRef)), XmlArrayItem("AllNets", typeof(AllNets))] public List<Object> _ObjectsAffected;
        std::vector<std::any> _ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };

    /// <summary>
    /// Описание правила зазоров между цепями.
    /// </summary>
public:
    class ClearanceNetToNet {
        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Параметр правила зазоров между цепями: минимальный зазор.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrnMin", DataType = "float")] public float _clrnMin;
        float _clrnMin = 0.0F;

        /// <summary>
        /// Параметр правила зазоров между цепями: номинальный зазор.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrnNom", DataType = "float")] public float _clrnNom;
        float _clrnNom = 0.0F;

        /// <summary>
        /// Ссылка на слои. См. также _LayersRefs
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRefs - там описан список ссылок типа LayerRef. </remarks>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("AllLayers", typeof(AllLayers)), XmlElement("AllLayersInner", typeof(AllLayersInner)), XmlElement("AllLayersInnerSignal", typeof(AllLayersInnerSignal)), XmlElement("AllLayersSignal", typeof(AllLayersSignal)), XmlElement("AllLayersOuter", typeof(AllLayersOuter)), XmlElement("LayerGroupRef", typeof(LayerGroupRef))] public Object _LayersRef;
        std::any _LayersRef;
        /// <summary>
        /// Ссылка на слои. См. также _LayersRef
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRef - там описаны ссылки остальных типов. </remarks>

        // ORIGINAL LINE: [XmlElement("LayerRef")] public List<LayerRef> _LayersRefs;
        std::vector<LayerRef*> _LayersRefs;
        bool ShouldSerialize_LayersRefs();
        /// <summary>
        /// Объекты воздействия правила.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("NetRef", typeof(NetRef)), XmlArrayItem("NetGroupRef", typeof(NetGroupRef)), XmlArrayItem("AllNets", typeof(AllNets)), XmlArrayItem("SignalRef", typeof(SignalRef)), XmlArrayItem("DiffSignalRef", typeof(DiffSignalRef)), XmlArrayItem("SignalGroupRef", typeof(SignalGroupRef))] public List<Object> _ObjectsAffected;
        std::vector<std::any> _ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };

    /// <summary>
    /// Описание правила зазоров между компонентами.
    /// </summary>
public:
    class ClearanceCompToComp {
        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Параметр правила зазоров между цепями: минимальный зазор.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("clrn", DataType = "float")] public float _clrn;
        float _clrn = 0.0F;

        /// <summary>
        /// Объекты воздействия правила.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("ComponentRef", typeof(ComponentRef)), XmlArrayItem("CompGroupRef", typeof(CompGroupRef)), XmlArrayItem("AllComps", typeof(AllComps))] public List<Object> _ObjectsAffected;
        std::vector<std::any> _ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };

    /// <summary>
    /// Описание зазоров до края платы.
    /// </summary>
public:
    class RulesClearancesToBoard {
        /// <summary>
        /// Устанавливает зазор от проводников до края платы.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("wires", DataType = "float")] public float _clrn;
        float _clrn = 0.0F;

        /// <summary>
        /// Устанавливает зазор от компонентов до края платы.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("comps", DataType = "float")] public float _comps;
        float _comps = 0.0F;
    };

    /// <summary>
    /// Описание правила назначения цепям стеков переходных отверстий.
    /// </summary>
public:
    class ViastacksOfNets {
        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Объекты воздействия правила.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("NetRef", typeof(NetRef)), XmlArrayItem("NetGroupRef", typeof(NetGroupRef)), XmlArrayItem("AllNets", typeof(AllNets)), XmlArrayItem("SignalRef", typeof(SignalRef)), XmlArrayItem("DiffSignalRef", typeof(DiffSignalRef)), XmlArrayItem("SignalGroupRef", typeof(SignalGroupRef))] public List<Object> _ObjectsAffected;
        std::vector<std::any> _ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
        /// <summary>
        /// Назначенные типы переходных отверстий.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("Viastacks")][XmlArrayItem("AllViastacks", typeof(AllViastacks)), XmlArrayItem("AllViastacksThrough", typeof(AllViastacksThrough)), XmlArrayItem("AllViastacksNotThrough", typeof(AllViastacksNotThrough)), XmlArrayItem("ViastackRef", typeof(ViastackRef))] public List<Object> _Viastacks;
        std::vector<std::any> _Viastacks;
        bool ShouldSerialize_Viastacks();
    };

    /// <summary>
    /// Описание правила назначения цепям опорных слоёв.
    /// </summary>
public:
    class PlaneLayerNets {
        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Ссылка на слои. См. также _LayersRefs
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRefs - там описан список ссылок типа LayerRef. </remarks>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("AllLayers", typeof(AllLayers)), XmlElement("AllLayersInner", typeof(AllLayersInner)), XmlElement("AllLayersInnerSignal", typeof(AllLayersInnerSignal)), XmlElement("AllLayersSignal", typeof(AllLayersSignal)), XmlElement("AllLayersOuter", typeof(AllLayersOuter)), XmlElement("LayerGroupRef", typeof(LayerGroupRef))] public Object _LayersRef;
        std::any _LayersRef;
        /// <summary>
        /// Ссылка на слои. См. также _LayersRef
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRef - там описаны ссылки остальных типов. </remarks>

        // ORIGINAL LINE: [XmlElement("LayerRef")] public List<LayerRef> _LayersRefs;
        std::vector<LayerRef*> _LayersRefs;
        bool ShouldSerialize_LayersRefs();
        /// <summary>
        /// Объекты воздействия правила.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("NetRef")] public List<NetRef> _ObjectsAffected;
        std::vector<NetRef*> _ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };

    /// <summary>
    /// Описание правила назначения цепям сигнальных слоёв.
    /// </summary>
public:
    class SignalLayerNets {
        /// <summary>
        /// Флаг применения правила.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
        Bool _enabled = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
        bool getEnabledSpecified() const;

        /// <summary>
        /// Ссылка на слои. См. также _LayersRefs
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRefs - там описан список ссылок типа LayerRef. </remarks>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("AllLayers", typeof(AllLayers)), XmlElement("AllLayersInner", typeof(AllLayersInner)), XmlElement("AllLayersInnerSignal", typeof(AllLayersInnerSignal)), XmlElement("AllLayersSignal", typeof(AllLayersSignal)), XmlElement("AllLayersOuter", typeof(AllLayersOuter)), XmlElement("LayerGroupRef", typeof(LayerGroupRef))] public Object _LayersRef;
        std::any _LayersRef;
        /// <summary>
        /// Ссылка на слои. См. также _LayersRef
        /// </summary>
        /// <remarks>! При null необходимо смотреть _LayersRef - там описаны ссылки остальных типов. </remarks>

        // ORIGINAL LINE: [XmlElement("LayerRef")] public List<LayerRef> _LayersRefs;
        std::vector<LayerRef*> _LayersRefs;
        bool ShouldSerialize_LayersRefs();
        /// <summary>
        /// Объекты воздействия правила.
        /// </summary>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("NetRef", typeof(NetRef)), XmlArrayItem("NetGroupRef", typeof(NetGroupRef))] public List<Object> _ObjectsAffected;
        std::vector<std::any> _ObjectsAffected;
        bool ShouldSerialize_ObjectsAffected();
    };

    /// <summary>
    /// Свойства цепи.
    /// </summary>
public:
    class NetProperty {
        /// <summary>
        /// Свойство цепи: гибкая фиксация.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("flexfix")] public Bool _flexfix;
        Bool _flexfix = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _flexfixSpecified
        bool getFlexfixSpecified() const;

        /// <summary>
        /// Свойство цепи: флаг трассировки для автоматического трассировщика.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("route")] public Bool _route;
        Bool _route = static_cast<Bool>(0);

        // ORIGINAL LINE: [XmlIgnore] public bool _routeSpecified
        bool getRouteSpecified() const;

        /// <summary>
        /// Ссылка на цепь.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("NetRef")] public List<NetRef> _NetRefs;
        std::vector<NetRef*> _NetRefs;
        bool ShouldSerialize_NetRefs();
    };

    /// <summary>
    /// Настройки подключения к углам прямоугольных контактных площадок.
    /// </summary>
public:
    class PadConnectSettings {
        /// <summary>
        /// Настройка подключения к углам прямоугольных контактных площадок: режим подключения.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("mode")] public mode_PadConnectSettings _mode;
        mode_PadConnectSettings _mode = static_cast<mode_PadConnectSettings>(0);

        /// <summary>
        /// Ссылки на стеки контактных площадок.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("PadstackRef")] public List<PadstackRef> _PadstackRefs;
        std::vector<PadstackRef*> _PadstackRefs;
        bool ShouldSerialize_PadstackRefs();
        /// <summary>
        /// Ссылки на контакты.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("PinRef")] public List<PinRef> _PinRefs;
        std::vector<PinRef*> _PinRefs;
        bool ShouldSerialize_PinRefs();
        /// <summary>
        /// Ссылки на выводы посадочных мест.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("PadRef")] public List<PadRef> _PadRefs;
        std::vector<PadRef*> _PadRefs;
        bool ShouldSerialize_PadRefs();
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Правила ширин проводников.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("RulesWidthOfWires")][XmlArrayItem("WidthOfWires")] public List<WidthOfWires> _RulesWidthOfWires;
    std::vector<WidthOfWires*> _RulesWidthOfWires;
    virtual ~Rules() {
        delete _RulesClearancesToBoard;
        delete _PadConnectSettings;
    }

    bool ShouldSerialize_RulesWidthOfWires();
    /// <summary>
    /// Правила зазоров между цепями.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("RulesClearancesNetToNet")][XmlArrayItem("ClearanceNetToNet")] public List<ClearanceNetToNet> _RulesClearancesNetToNet;
    std::vector<ClearanceNetToNet*> _RulesClearancesNetToNet;
    bool ShouldSerialize_RulesClearancesNetToNet();
    /// <summary>
    /// Правила зазоров между компонентами.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("RulesClearancesCompToComp")][XmlArrayItem("ClearanceCompToComp")] public List<ClearanceCompToComp> _RulesClearancesCompToComp;
    std::vector<ClearanceCompToComp*> _RulesClearancesCompToComp;
    bool ShouldSerialize_RulesClearancesCompToComp();
    /// <summary>
    /// Правило зазоров до края платы.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("RulesClearancesToBoard")] public RulesClearancesToBoard _RulesClearancesToBoard;
    RulesClearancesToBoard* _RulesClearancesToBoard;

    /// <summary>
    /// Правила назначения цепям стеков переходных отверстий.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("RulesViastacksOfNets")][XmlArrayItem("ViastacksOfNets")] public List<ViastacksOfNets> _RulesViastacksOfNets;
    std::vector<ViastacksOfNets*> _RulesViastacksOfNets;
    bool ShouldSerialize_RulesViastacksOfNets();
    /// <summary>
    /// Правила назначения цепям опорных слоёв.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("RulesPlaneLayersNets")][XmlArrayItem("PlaneLayerNets")] public List<PlaneLayerNets> _RulesPlaneLayersNets;
    std::vector<PlaneLayerNets*> _RulesPlaneLayersNets;
    bool ShouldSerialize_RulesPlaneLayersNets();
    /// <summary>
    /// Правила назначения цепям сигнальных слоёв.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("RulesSignalLayersNets")][XmlArrayItem("SignalLayerNets")] public List<SignalLayerNets> _RulesSignalLayersNets;
    std::vector<SignalLayerNets*> _RulesSignalLayersNets;
    bool ShouldSerialize_RulesSignalLayersNets();
    /// <summary>
    /// Свойства цепей
    /// </summary>

    // ORIGINAL LINE: [XmlArray("NetProperties")][XmlArrayItem("NetProperty")] public List<NetProperty> _NetProperties;
    std::vector<NetProperty*> _NetProperties;
    bool ShouldSerialize_NetProperties();
    /// <summary>
    /// Настройки подключения к углам прямоугольных контактных площадок.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("PadConnectSettings")] public PadConnectSettings _PadConnectSettings;
    PadConnectSettings* _PadConnectSettings;

    /*****************************************************************
     * Здесь находятся функции для работы с элементами класса Rules. *
     * Они не являются частью формата TopoR PCB.                     *
     * ***************************************************************/

    /// <summary>
    /// Переименование ссылок на компонент, если его имя изменилось
    /// </summary>
    /// <param name="oldname">старое имя компонента</param>
    /// <param name="newname">новое имя компонента</param>
    void Rename_compName(const QString& oldname, const QString& newname);
};
// } // namespace TopoR_PCB_Classes

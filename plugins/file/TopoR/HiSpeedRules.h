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
/// Раздел «Правила для высокоскоростных устройств».
/// </summary>
class HiSpeedRules : public QSerializer {
    Q_GADGET
    QS_SERIALIZABLE
    /// <summary>
    /// Волновое сопротивление и правила разводки сигналов по слоям.
    /// </summary>
public:
    class Impedance {
    public:
        class LayerRule_Impendance {
            /// <summary>
            /// Ширина проводника.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("width", DataType = "float")] public float _width;
            float _width = 0.0F;

            /// <summary>
            /// Ссылка на слой.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
            LayerRef* _LayerRef;
            virtual ~LayerRule_Impendance() {
                delete _LayerRef;
            }
        };

        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Параметр правила разводки дифференциальной пары: значение волнового сопротивления в Омах.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("Z0", DataType = "float")] public float _z0;
        float _z0 = 0.0F;

        /// <summary>
        /// Правило разводки сигнала для слоя.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("LayerRule")] public List<LayerRule_Impendance> _LayerImpedanceRules;
        std::vector<LayerRule_Impendance*> _LayerImpedanceRules;
        bool ShouldSerialize_LayerImpedanceRules();
    };

    /// <summary>
    /// Волновое сопротивление и правила разводки сигналов по слоям для дифференциальных сигналов.
    /// </summary>
public:
    class ImpedanceDiff {
    public:
        class LayerRule_ImpendanceDiff {
            /// <summary>
            /// Ширина проводника.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("width", DataType = "float")] public float _width;
            float _width = 0.0F;

            /// <summary>
            /// Параметр правила разводки дифференциальных пар: зазор между проводниками пары.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("gap", DataType = "float")] public float _gap;
            float _gap = 0.0F;

            /// <summary>
            /// Ссылка на слой.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("LayerRef")] public LayerRef _LayerRef;
            LayerRef* _LayerRef;
            virtual ~LayerRule_ImpendanceDiff() {
                delete _LayerRef;
            }
        };

        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Параметр правила разводки дифференциальной пары: значение волнового сопротивления в Омах.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("Z0", DataType = "float")] public float _z0;
        float _z0 = 0.0F;

        /// <summary>
        /// Правило разводки дифференциальной пары для слоя.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("LayerRule")] public List<LayerRule_ImpendanceDiff> _LayerImpedanceDiffRules;
        std::vector<LayerRule_ImpendanceDiff*> _LayerImpedanceDiffRules;
        bool ShouldSerialize_LayerImpedanceDiffRules();
    };

    /// <summary>
    /// Описание сигнального кластера цепей.
    /// </summary>
public:
    class SignalCluster {
        /// <summary>
        /// Описание заданной связи.
        /// </summary>
    public:
        class PinPair {
            /// <summary>
            /// Ссылка на контакт источника сигнала.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlElement("PinRef")] public List<PinRef> _PinRefs;
            std::vector<PinRef*> _PinRefs;
            bool ShouldSerialize_PinRefs();
        };

        /// <summary>
        /// Описание сигнала.
        /// </summary>
    public:
        class Signal {
            /// <summary>
            /// Имя объекта или ссылка на именованный объект.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
            QString _name;

            /// <summary>
            /// Ссылка на контакт источника сигнала.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("ReceiverPinRef")] public ReceiverPinRef _ReceiverPinRef;
            ReceiverPinRef* _ReceiverPinRef;

            /// <summary>
            /// Пассивные компоненты на пути следования сигнала.
            /// </summary>

            // ORIGINAL LINE: [XmlArray("Components")][XmlArrayItem("CompInstanceRef")] public List<CompInstanceRef> _Components;
            std::vector<CompInstanceRef*> _Components;
            /*   public bool ShouldSerialize_Components()
               {
                   return _Components?.Count > 0;
               }*/
            virtual ~Signal() {
                delete _ReceiverPinRef;
            }
        };

        /// <summary>
        /// Ссылка на волновое сопротивление.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlElement("ImpedanceRef")] public ImpedanceRef _ImpedanceRef;
        ImpedanceRef* _ImpedanceRef;

        /// <summary>
        /// Ссылка на контакт источника сигнала.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("SourcePinRef")] public SourcePinRef _SourcePinRef;
        SourcePinRef* _SourcePinRef;

        /// <summary>
        /// Цепи сигнального кластера.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("Nets")][XmlArrayItem("NetRef")] public List<NetRef> _Nets;
        std::vector<NetRef*> _Nets;
        /*     public bool ShouldSerialize_Nets()
             {
                 return _Nets?.Count > 0;
             }
        */
        /// <summary>
        /// Описание заданных связей сигнального кластера.
        /// </summary>

        // ORIGINAL LINE: [XmlArray("PinPairs")][XmlArrayItem("PinPair")] public List<PinPair> _PinPairs;
        std::vector<PinPair*> _PinPairs;
        /*   public bool ShouldSerialize_PinPairs()
           {
               return _PinPairs?.Count > 0;
           }
        */
        /// <summary>
        /// Ссылки на сигналы.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("Signal")] public List<Signal> _Signals;
        std::vector<Signal*> _Signals;
        /*    public bool ShouldSerialize_Signals()
            {
                return _Signals?.Count > 0;
            }
        */
        virtual ~SignalCluster() {
            delete _ImpedanceRef;
            delete _SourcePinRef;
        }
    };

    /// <summary>
    /// Описание дифференциального сигнала (дифференциальной пары).
    /// </summary>
public:
    class DiffSignal {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Параметр дифференциальной пары: допустимый разброс длины между проводниками пары.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("mismatch", DataType = "float")] public float _mismatch;
        float _mismatch = 0.0F;

        /// <summary>
        /// Ссылка на волновое сопротивление.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("ImpedanceRef")] public ImpedanceRef _ImpedanceRef;
        ImpedanceRef* _ImpedanceRef;

        /// <summary>
        /// Ссылки на сигналы.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("SignalRef")] public List<SignalRef> _SignalRefs;
        std::vector<SignalRef*> _SignalRefs;
        virtual ~DiffSignal() {
            delete _ImpedanceRef;
        }

        bool ShouldSerialize_SignalRefs();
    };

    /// <summary>
    /// Описание группы сигналов.
    /// </summary>
public:
    class SignalGroup {
        /// <summary>
        /// Имя объекта или ссылка на именованный объект.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("name")] public string _name;
        QString _name;

        /// <summary>
        /// Ссылки на сигнал, диф.сигнал, или группу сигналов
        /// </summary>
        /// <value>SignalRef, DiffSignalRef, SignalGroupRef</value>
        // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

        // ORIGINAL LINE: [XmlElement("SignalRef", typeof(SignalRef)), XmlElement("DiffSignalRef", typeof(DiffSignalRef)), XmlElement("SignalGroupRef", typeof(SignalGroupRef))] public List<Object> _References;
        std::vector<std::any> _References;
        bool ShouldSerialize_References();
    };

    /// <summary>
    /// Описание правил выравнивания задержек.
    /// </summary>
public:
    class RulesDelay {
        /// <summary>
        /// Описание правила выравнивания задержек для группы цепей или группы дифференциальных пар.
        /// </summary>
    public:
        class DelayEqual {
            /// <summary>
            /// Флаг применения правила.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
            Bool _enabled = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
            bool getEnabledSpecified() const;

            /// <summary>
            /// Параметр правил выравнивания задержек: тип значений констант и допусков.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("valueType")] public valueType _valueType;
            valueType _valueType = static_cast<valueType>(0);

            /// <summary>
            /// Параметр правила выравнивания задержек внутри группы цепей: допуск.
            /// </summary>
            /// <remarks>! Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).</remarks>

            // ORIGINAL LINE: [XmlAttribute("tolerance", DataType = "float")] public float _tolerance;
            float _tolerance = 0.0F;

            /// <summary>
            /// Объекты воздействия правила.
            /// </summary>

            // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("SignalGroupRef")] public List<SignalGroupRef> _ObjectsAffected;
            std::vector<SignalGroupRef*> _ObjectsAffected;
            bool ShouldSerialize_ObjectsAffected();
        };

        /// <summary>
        /// Описание правила задания абсолютного значения задержки.
        /// </summary>
    public:
        class DelayConstant {
            /// <summary>
            /// Флаг применения правила.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
            Bool _enabled = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
            bool getEnabledSpecified() const;

            /// <summary>
            /// Параметр правил выравнивания задержек: тип значений констант и допусков.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("valueType")] public valueType _valueType;
            valueType _valueType = static_cast<valueType>(0);

            /// <summary>
            /// Значение константы в правилах выравнивания задержек.
            /// </summary>
            /// <remarks>! Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).</remarks>

            // ORIGINAL LINE: [XmlAttribute("constant", DataType = "float")] public float _constant;
            float _constant = 0.0F;

            /// <summary>
            /// Параметр правила выравнивания задержек: нижний допуск.
            /// </summary>
            /// <remarks>! Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).</remarks>

            // ORIGINAL LINE: [XmlAttribute("toleranceUnder", DataType = "float")] public float _toleranceUnder;
            float _toleranceUnder = 0.0F;

            /// <summary>
            /// Параметр правила выравнивания задержек: верхний допуск.
            /// </summary>
            /// <remarks>! Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).</remarks>

            // ORIGINAL LINE: [XmlAttribute("toleranceOver", DataType = "float")] public float _toleranceOver;
            float _toleranceOver = 0.0F;

            /// <summary>
            /// Объекты воздействия правила.
            /// </summary>
            // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

            // ORIGINAL LINE: [XmlArray("ObjectsAffected")][XmlArrayItem("SignalRef", typeof(SignalRef)), XmlArrayItem("DiffSignalRef", typeof(DiffSignalRef)), XmlArrayItem("SignalGroupRef", typeof(SignalGroupRef))] public List<Object> _ObjectsAffected;
            std::vector<std::any> _ObjectsAffected;
            bool ShouldSerialize_ObjectsAffected();
        };

        /// <summary>
        /// Описание правила взаимного выравнивания задержек.
        /// </summary>
        /// <remarks>! Правило несимметрично относительно ObjectLeft и ObjectRight</remarks>
    public:
        class DelayRelation {
            /// <summary>
            /// Флаг применения правила.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
            Bool _enabled = static_cast<Bool>(0);
            virtual ~DelayRelation() {
                delete _ObjectLeft;
                delete _ObjectRight;
            }

            // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
            bool getEnabledSpecified() const;

            /// <summary>
            /// Параметр правил выравнивания задержек: тип значений констант и допусков.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("valueType")] public valueType _valueType;
            valueType _valueType = static_cast<valueType>(0);

            /// <summary>
            /// Значение константы в правилах выравнивания задержек.
            /// </summary>
            /// <remarks>! Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).</remarks>

            // ORIGINAL LINE: [XmlAttribute("constant", DataType = "float")] public float _constant;
            float _constant = 0.0F;

            /// <summary>
            /// Параметр правила выравнивания задержек: нижний допуск.
            /// </summary>
            /// <remarks>! Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).</remarks>

            // ORIGINAL LINE: [XmlAttribute("toleranceUnder", DataType = "float")] public float _toleranceUnder;
            float _toleranceUnder = 0.0F;

            /// <summary>
            /// Параметр правила выравнивания задержек: верхний допуск.
            /// </summary>
            /// <remarks>! Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).</remarks>

            // ORIGINAL LINE: [XmlAttribute("toleranceOver", DataType = "float")] public float _toleranceOver;
            float _toleranceOver = 0.0F;

            /// <summary>
            /// Первый объект воздействия правила взаимного выравнивания задержек.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("ObjectLeft")] public ObjectSignal _ObjectLeft;
            ObjectSignal* _ObjectLeft;

            /// <summary>
            /// Второй объект воздействия правила взаимного выравнивания задержек.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("ObjectRight")] public ObjectSignal _ObjectRight;
            ObjectSignal* _ObjectRight;
        };

        /// <summary>
        /// Правила выравнивания задержек для группы цепей или группы дифференциальных пар.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlElement("DelayEqual")] public List<DelayEqual> _DelayEquals;
        std::vector<DelayEqual*> _DelayEquals;
        bool ShouldSerialize_DelayEquals();
        /// <summary>
        /// Правила задания абсолютного значения задержки.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("DelayConstant")] public List<DelayConstant> _DelayConstants;
        std::vector<DelayConstant*> _DelayConstants;
        bool ShouldSerialize_DelayConstants();
        /// <summary>
        /// Правила взаимного выравнивания задержек.
        /// </summary>
        /// <remarks>! Правила несимметричны относительно ObjectLeft и ObjectRight</remarks>

        // ORIGINAL LINE: [XmlElement("DelayRelation")] public List<DelayRelation> _DelayRelations;
        std::vector<DelayRelation*> _DelayRelations;
        bool ShouldSerialize_DelayRelations();
    };

    /// <summary>
    /// Настройки поиска сигналов.
    /// </summary>
public:
    class SignalSearchSettings {
        /// <summary>
        /// Правило именования цепей дифференциальных сигналов.
        /// </summary>
    public:
        class RuleDiffSignalNetsNames {
            /// <summary>
            /// Флаг применения правила.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("enabled")] public Bool _enabled;
            Bool _enabled = static_cast<Bool>(0);

            // ORIGINAL LINE: [XmlIgnore] public bool _enabledSpecified
            bool getEnabledSpecified() const;

            /// <summary>
            /// Параметр правила именования цепей дифференциальных сигналов: подстрока, определяющая цепь позитивного сигнала.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("posStr")] public string _posStr;
            QString _posStr;

            /// <summary>
            /// Параметр правила именования цепей дифференциальных сигналов: подстрока, определяющая цепь негативного сигнала.
            /// </summary>

            // ORIGINAL LINE: [XmlAttribute("negStr")] public string _negStr;
            QString _negStr;
        };

        /// <summary>
        /// Список цепей, исключённых из поиска сигналов.
        /// </summary>
    public:
        class ExcludedNets {
            /// <summary>
            /// Минимальное количество контактов в силовой цепи. Параметр используется для автоматического определения силовых цепей.
            /// </summary>
        public:
            // ORIGINAL LINE: [XmlAttribute("minPinsNumber", DataType = "int")] public int _minPinsNumber;
            int _minPinsNumber = 0;

            /// <summary>
            /// Cсылки на цепи.
            /// </summary>

            // ORIGINAL LINE: [XmlElement("NetRef")] public List<NetRef> _NetRefs;
            std::vector<NetRef*> _NetRefs;
            bool ShouldSerialize_NetRefs();
        };

        /// <summary>
        /// Максимальное число цепей в сигнальном кластере. Параметр используется при автоматическом определении цепей сигнального кластера.
        /// </summary>
    public:
        // ORIGINAL LINE: [XmlAttribute("maxNetsInCluster", DataType = "int")] public int _maxNetsInCluster;
        int _maxNetsInCluster = 0;

        /// <summary>
        /// Автоматически задавать связи.
        /// </summary>

        // ORIGINAL LINE: [XmlAttribute("createPinPairs")] public Bool _createPinPairs;
        Bool _createPinPairs = static_cast<Bool>(0);
        virtual ~SignalSearchSettings() {
            delete _ExcludedNets;
        }

        // ORIGINAL LINE: [XmlIgnore] public bool _createPinPairsSpecified
        bool getCreatePinPairsSpecified() const;

        /// <summary>
        /// Правила именования цепей дифференциальных сигналов.
        /// </summary>
        /// <remarks>! Порядок следования правил в этой секции определяет приоритет правил. Правила следуют в порядке убывания приоритета.</remarks>

        // ORIGINAL LINE: [XmlArray("RulesDiffSignalNetsNames")][XmlArrayItem("RuleDiffSignalNetsNames")] public List<RuleDiffSignalNetsNames> _RulesDiffSignalNetsNames;
        std::vector<RuleDiffSignalNetsNames*> _RulesDiffSignalNetsNames;
        bool ShouldSerialize_RulesDiffSignalNetsNames();
        /// <summary>
        /// Список цепей, исключённых из поиска сигналов.
        /// </summary>

        // ORIGINAL LINE: [XmlElement("ExcludedNets")] public ExcludedNets _ExcludedNets;
        ExcludedNets* _ExcludedNets;
    };

    /// <summary>
    /// Версия раздела.
    /// </summary>
public:
    // ORIGINAL LINE: [XmlAttribute("version")] public string _version;
    QString _version;

    /// <summary>
    /// Волновые сопротивления и правила разводки сигналов.
    /// </summary>
    // C# TO C++ CONVERTER TODO TASK: There is no C++ equivalent to the C# 'typeof' operator:

    // ORIGINAL LINE: [XmlArray("RulesImpedances")][XmlArrayItem("Impedance", typeof(Impedance)), XmlArrayItem("ImpedanceDiff", typeof(ImpedanceDiff))] public List<Object> _RulesImpedances;
    std::vector<std::any> _RulesImpedances;
    virtual ~HiSpeedRules() {
        delete _RulesDelay;
        delete _SignalSearchSettings;
    }

    bool ShouldSerialize_RulesImpedances();
    /// <summary>
    /// Сигнальные кластеры цепей.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("SignalClusters")][XmlArrayItem("SignalCluster")] public List<SignalCluster> _SignalClusters;
    std::vector<SignalCluster*> _SignalClusters;
    bool ShouldSerialize_SignalClusters();
    /// <summary>
    /// Дифференциальные сигналы.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("DiffSignals")][XmlArrayItem("DiffSignal")] public List<DiffSignal> _DiffSignals;
    std::vector<DiffSignal*> _DiffSignals;
    bool ShouldSerialize_DiffSignals();
    /// <summary>
    /// Группы сигналов.
    /// </summary>

    // ORIGINAL LINE: [XmlArray("SignalGroups")][XmlArrayItem("SignalGroup")] public List<SignalGroup> _SignalGroups;
    std::vector<SignalGroup*> _SignalGroups;
    bool ShouldSerialize_SignalGroups();
    /// <summary>
    /// Правила выравнивания задержек.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("RulesDelay")] public RulesDelay _RulesDelay;
    RulesDelay* _RulesDelay;

    /// <summary>
    /// Настройки автоматического поиска сигналов.
    /// </summary>

    // ORIGINAL LINE: [XmlElement("SignalSearchSettings")] public SignalSearchSettings _SignalSearchSettings;
    SignalSearchSettings* _SignalSearchSettings;

    /************************************************************************
     * Здесь находятся функции для работы с элементами класса HiSpeedRules. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/

    /// <summary>
    /// Переименование ссылок на компонент, если его имя изменилось
    /// </summary>
    /// <param name="oldname">старое имя компонента</param>
    /// <param name="newname">новое имя компонента</param>
    void Rename_compName(const QString& oldname, const QString& newname);
    /***********************************************************************/
};
// } // namespace TopoR_PCB_Classes

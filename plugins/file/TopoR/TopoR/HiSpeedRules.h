#pragma once
#include "Commons.h"
/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 */
namespace TopoR {
// Раздел «Правила для высокоскоростных устройств».
struct HiSpeedRules {
    // Волновое сопротивление и правила разводки сигналов по слоям.
    struct Impedance {
        struct LayerRule {
            // Ширина проводника.
            [[= XML::Attr]] double width{};        //("width")
                                                   // Ссылка на слой.
            /*[[= XML::Elem]]*/ LayerRef LayerRef; //("LayerRef")
        };
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name; //("name")
        // Параметр правила разводки дифференциальной пары: значение волнового сопротивления в Омах.
        [[= XML::Attr]] double Z0{}; //("Z0")
        // Правило разводки сигнала для слоя.
        ///*[[= XML::Elem]]*/ // public List<LayerRule_Impendance> LayerImpedanceRules;//("LayerRule")
        [[= XML::Elem]] std::vector<LayerRule> LayerImpedanceRules; //("LayerRule")
        bool ShouldSerialize_LayerImpedanceRules();
    };
    // Волновое сопротивление и правила разводки сигналов по слоям для дифференциальных сигналов.
    struct ImpedanceDiff {
        struct LayerRule {
            // Ширина проводника.
            [[= XML::Attr]] double width{}; //("width")
            // Параметр правила разводки дифференциальных пар: зазор между проводниками пары.
            [[= XML::Attr]] double gap{};          //("gap")
                                                   // Ссылка на слой.
            /*[[= XML::Elem]]*/ LayerRef LayerRef; //("LayerRef")
        };
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name; //("name")
        // Параметр правила разводки дифференциальной пары: значение волнового сопротивления в Омах.
        [[= XML::Attr]] double Z0{}; //("Z0")
        // Правило разводки дифференциальной пары для слоя.
        ///*[[= XML::Elem]]*/ // public List<LayerRule_ImpendanceDiff> LayerImpedanceDiffRules;//("LayerRule")
        [[= XML::Elem]] std::vector<LayerRule> LayerImpedanceDiffRules; //("LayerRule")
        bool ShouldSerialize_LayerImpedanceDiffRules();
    };
    // Описание сигнального кластера цепей.
    struct SignalCluster {
        // Описание заданной связи.
        struct PinPair {
            // Ссылка на контакт источника сигнала.
            ///*[[= XML::Elem]]*/ // public List<PinRef> PinRefs;//("PinRef")
            [[= XML::Elem]] std::vector<PinRef> PinRefs; //("PinRef")
            bool ShouldSerialize_PinRefs();
        };
        // Описание сигнала.
        struct Signal {
            // Имя объекта или ссылка на именованный объект.
            [[= XML::Attr]] std::string name;                  //("name")
                                                               // Ссылка на контакт источника сигнала.
            /*[[= XML::Elem]]*/ ReceiverPinRef ReceiverPinRef; //("ReceiverPinRef")
            // Пассивные компоненты на пути следования сигнала.
            //[XmlArrayItem("CompInstanceRef")] public List<CompInstanceRef> Components;
            [[= XML::Array]] std::vector<CompInstanceRef> Components;
            /*   public bool ShouldSerialize_Components()
               {
                   return Components?.Count > 0;
               }*/
        };
        // Ссылка на волновое сопротивление.
        /*[[= XML::Elem]]*/ ImpedanceRef ImpedanceRef; //("ImpedanceRef")
                                                       // Ссылка на контакт источника сигнала.
        /*[[= XML::Elem]]*/ SourcePinRef SourcePinRef; //("SourcePinRef")
        // Цепи сигнального кластера.
        //[XmlArrayItem("NetRef")] public List<NetRef> Nets;
        [[= XML::Array]] std::vector<NetRef> Nets;
        /*     public bool ShouldSerialize_Nets()
             {
                 return Nets?.Count > 0;
             }
        */
        // Описание заданных связей сигнального кластера.
        //[XmlArrayItem("PinPair")] public List<PinPair> PinPairs;
        [[= XML::Array]] std::vector<PinPair> PinPairs;
        /*   public bool ShouldSerialize_PinPairs()
           {
               return PinPairs?.Count > 0;
           }
        */
        // Ссылки на сигналы.
        ///*[[= XML::Elem]]*/ // public List<Signal> Signals;//("Signal")
        [[= XML::Elem]] std::vector<Signal> Signals; //("Signal")
        /*    public bool ShouldSerialize_Signals()
            {
                return Signals?.Count > 0;
            }
        */
    };
    // Описание дифференциального сигнала (дифференциальной пары).
    struct DiffSignal {
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name; //("name")
        // Параметр дифференциальной пары: допустимый разброс длины между проводниками пары.
        [[= XML::Attr]] double mismatch{};             //("mismatch")
                                                       // Ссылка на волновое сопротивление.
        /*[[= XML::Elem]]*/ ImpedanceRef ImpedanceRef; //("ImpedanceRef")
        // Ссылки на сигналы.
        ///*[[= XML::Elem]]*/ // public List<SignalRef> SignalRefs;//("SignalRef")
        [[= XML::Elem]] std::vector<SignalRef> SignalRefs; //("SignalRef")
        bool ShouldSerialize_SignalRefs();
    };
    // Описание группы сигналов.
    struct SignalGroup {
        // Имя объекта или ссылка на именованный объект.
        [[= XML::Attr]] std::string name; //("name")
        // Ссылки на сигнал, диф.сигнал, или группу сигналов
        // <value>SignalRef, DiffSignalRef, SignalGroupRef</value>
        // public List<Object> References;
        [[= XML::Elem]] std::vector<std::variant</*XML::Null,*/ SignalRef, DiffSignalRef, SignalGroupRef>> References;
        bool ShouldSerialize_References();
    };
    // Описание правил выравнивания задержек.
    struct RulesDelay {
        // Описание правила выравнивания задержек для группы цепей или группы дифференциальных пар.
        struct DelayEqual {
            // Флаг применения правила.
            [[= XML::Attr]] Bool enabled{}; //("enabled")
            bool getEnabledSpecified() const;
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            [[= XML::Attr]] ValueType valueType{};
            // Параметр правила выравнивания задержек внутри группы цепей: допуск.
            // ! Единицы измерения значения зависят от параметра ValueType и единиц заданных для всего файла(см.Units).
            [[= XML::Attr]] double tolerance{}; //("tolerance")
            // Объекты воздействия правила.
            //[XmlArrayItem("SignalGroupRef")] public List<SignalGroupRef> ObjectsAffected;
            [[= XML::Array]] std::vector<SignalGroupRef> ObjectsAffected;
            bool ShouldSerialize_ObjectsAffected();
        };
        // Описание правила задания абсолютного значения задержки.
        struct DelayConstant {
            // Флаг применения правила.
            [[= XML::Attr]] Bool enabled{}; //("enabled")
            bool getEnabledSpecified() const;
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            [[= XML::Attr]] ValueType valueType{};
            // Значение константы в правилах выравнивания задержек.
            // ! Единицы измерения значения зависят от параметра ValueType и единиц заданных для всего файла(см.Units).
            [[= XML::Attr]] double constant{}; //("constant")
            // Параметр правила выравнивания задержек: нижний допуск.
            // ! Единицы измерения значения зависят от параметра ValueType и единиц заданных для всего файла(см.Units).
            [[= XML::Attr]] double toleranceUnder{}; //("toleranceUnder")
            // Параметр правила выравнивания задержек: верхний допуск.
            // ! Единицы измерения значения зависят от параметра ValueType и единиц заданных для всего файла(см.Units).
            [[= XML::Attr]] double toleranceOver{}; //("toleranceOver")
            // Объекты воздействия правила.
            //[XmlArrayItem("SignalRef", typeof(SignalRef)), XmlArrayItem("DiffSignalRef", typeof(DiffSignalRef)), XmlArrayItem("SignalGroupRef", typeof(SignalGroupRef))] public List<Object> ObjectsAffected;
            [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ SignalRef, DiffSignalRef, SignalGroupRef>> ObjectsAffected;
            bool ShouldSerialize_ObjectsAffected();
        };
        // Описание правила взаимного выравнивания задержек.
        // ! Правило несимметрично относительно ObjectLeft и ObjectRight
        struct DelayRelation {
            // Флаг применения правила.
            [[= XML::Attr]] Bool enabled{}; //("enabled")
            bool getEnabledSpecified() const;
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            [[= XML::Attr]] ValueType valueType{};
            // Значение константы в правилах выравнивания задержек.
            // ! Единицы измерения значения зависят от параметра ValueType и единиц заданных для всего файла(см.Units).
            [[= XML::Attr]] double constant{}; //("constant")
            // Параметр правила выравнивания задержек: нижний допуск.
            // ! Единицы измерения значения зависят от параметра ValueType и единиц заданных для всего файла(см.Units).
            [[= XML::Attr]] double toleranceUnder{}; //("toleranceUnder")
            // Параметр правила выравнивания задержек: верхний допуск.
            // ! Единицы измерения значения зависят от параметра ValueType и единиц заданных для всего файла(см.Units).
            [[= XML::Attr]] double toleranceOver{};       //("toleranceOver")
                                                          // Первый объект воздействия правила взаимного выравнивания задержек.
            /*[[= XML::Elem]]*/ ObjectSignal ObjectLeft;  //("ObjectLeft")
                                                          // Второй объект воздействия правила взаимного выравнивания задержек.
            /*[[= XML::Elem]]*/ ObjectSignal ObjectRight; //("ObjectRight")
        };
        // Правила выравнивания задержек для группы цепей или группы дифференциальных пар.
        ///*[[= XML::Elem]]*/ // public List<DelayEqual> DelayEquals;//("DelayEqual")
        [[= XML::Elem]] std::vector<DelayEqual> DelayEquals; //("DelayEqual")
        bool ShouldSerialize_DelayEquals();
        // Правила задания абсолютного значения задержки.
        ///*[[= XML::Elem]]*/ // public List<DelayConstant> DelayConstants;//("DelayConstant")
        [[= XML::Elem]] std::vector<DelayConstant> DelayConstants; //("DelayConstant")
        bool ShouldSerialize_DelayConstants();
        // Правила взаимного выравнивания задержек.
        // ! Правила несимметричны относительно ObjectLeft и ObjectRight
        ///*[[= XML::Elem]]*/ // public List<DelayRelation> DelayRelations;//("DelayRelation")
        [[= XML::Elem]] std::vector<DelayRelation> DelayRelations; //("DelayRelation")
        bool ShouldSerialize_DelayRelations();
    };
    // Настройки поиска сигналов.
    struct SignalSearchSettings {
        // Правило именования цепей дифференциальных сигналов.
        struct RuleDiffSignalNetsNames {
            // Флаг применения правила.
            [[= XML::Attr]] Bool enabled{};
            bool getEnabledSpecified() const;
            // Параметр правила именования цепей дифференциальных сигналов: подстрока, определяющая цепь позитивного сигнала.
            [[= XML::Attr]] std::string posStr;
            // Параметр правила именования цепей дифференциальных сигналов: подстрока, определяющая цепь негативного сигнала.
            [[= XML::Attr]] std::string negStr;
        };
        // Список цепей, исключённых из поиска сигналов.
        struct ExcludedNets {
            // Минимальное количество контактов в силовой цепи. Параметр используется для автоматического определения силовых цепей.
            [[= XML::Attr]] int minPinsNumber{};
            // Cсылки на цепи.
            ///*[[= XML::Elem]]*/ // public List<NetRef> NetRefs;//("NetRef")
            [[= XML::Elem]] std::vector<NetRef> NetRefs;
            bool ShouldSerialize_NetRefs();
        };
        // Максимальное число цепей в сигнальном кластере. Параметр используется при автоматическом определении цепей сигнального кластера.
        [[= XML::Attr]] int maxNetsInCluster{};
        // Автоматически задавать связи.
        [[= XML::Attr]] Bool createPinPairs{};
        // public bool createPinPairsSpecified
        bool getCreatePinPairsSpecified() const;
        // Правила именования цепей дифференциальных сигналов.
        // ! Порядок следования правил в этой секции определяет приоритет правил. Правила следуют в порядке убывания приоритета.
        //[XmlArrayItem("RuleDiffSignalNetsNames")] public List<RuleDiffSignalNetsNames> RulesDiffSignalNetsNames;
        [[= XML::Array]] std::vector<RuleDiffSignalNetsNames> RulesDiffSignalNetsNames;
        bool ShouldSerialize_RulesDiffSignalNetsNames();
        // Список цепей, исключённых из поиска сигналов.
        /*[[= XML::Elem]]*/ ExcludedNets ExcludedNets;
    };
    // Версия раздела.
    [[= XML::Attr]] std::string version;
    // Волновые сопротивления и правила разводки сигналов.
    //[XmlArrayItem("Impedance"), XmlArrayItem("ImpedanceDiff")] public List<Object> RulesImpedances;
    [[= XML::Array]] std::vector<std::variant</*XML::Null,*/ Impedance, ImpedanceDiff>> RulesImpedances;
    bool ShouldSerialize_RulesImpedances();
    // Сигнальные кластеры цепей.
    //[XmlArrayItem("SignalCluster")] public List<SignalCluster> SignalClusters;
    [[= XML::Array]] std::vector<SignalCluster> SignalClusters;
    bool ShouldSerialize_SignalClusters();
    // Дифференциальные сигналы.
    //[XmlArrayItem("DiffSignal")] public List<DiffSignal> DiffSignals;
    [[= XML::Array]] std::vector<DiffSignal> DiffSignals;
    bool ShouldSerialize_DiffSignals();
    // Группы сигналов.
    //[XmlArrayItem("SignalGroup")] public List<SignalGroup> SignalGroups;
    [[= XML::Array]] std::vector<SignalGroup> SignalGroups;
    bool ShouldSerialize_SignalGroups();
    // Правила выравнивания задержек.
    /*[[= XML::Elem]]*/ RulesDelay RulesDelay;
    // Настройки автоматического поиска сигналов.
    /*[[= XML::Elem]]*/ SignalSearchSettings SignalSearchSettings;
    /************************************************************************
     * Здесь находятся функции для работы с элементами класса HiSpeedRules. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/
    // Переименование ссылок на компонент, если его имя изменилось
    void Rename_compName(const std::string& oldname, const std::string& newname);
    /***********************************************************************/
};
} // namespace TopoR

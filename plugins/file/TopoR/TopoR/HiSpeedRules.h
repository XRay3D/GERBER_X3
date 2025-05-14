#pragma once

#include "Commons.h"

/* Мною, Константином aka KilkennyCat, 05 июля 2020 года создано сиё
 * на основе "Описание формата TopoR PCB версия 1.2.0 Апрель 2017 г.".
 * k@kilkennycat.pro
 * http://kilkennycat.ru  http://kilkennycat.pro
 * Мною, Дамиром aka x-ray, 08.02.2025 года сие перекидано на кресты.
 */

namespace TopoR {

// Раздел «Правила для высокоскоростных устройств».
struct HiSpeedRules {
    // Волновое сопротивление и правила разводки сигналов по слоям.
    struct Impedance {
        struct LayerRule {
            // Ширина проводника.
            Xml::Attr<double> width;
            // Ссылка на слой.
            LayerRef layerRef;
        };
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Параметр правила разводки дифференциальной пары: значение волнового сопротивления в Омах.
        Xml::Attr<double> Z0;
        // Правило разводки сигнала для слоя.
        Xml::Array<LayerRule> LayerImpedanceRules;
    };
    // Волновое сопротивление и правила разводки сигналов по слоям для дифференциальных сигналов.
    struct ImpedanceDiff {
        struct LayerRule {
            // Ширина проводника.
            Xml::Attr<double> width;
            // Параметр правила разводки дифференциальных пар: зазор между проводниками пары.
            Xml::Attr<double> gap;
            // Ссылка на слой.
            LayerRef layerRef;
        };
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Параметр правила разводки дифференциальной пары: значение волнового сопротивления в Омах.
        Xml::Attr<double> Z0;
        // Правило разводки дифференциальной пары для слоя.
        Xml::Array<LayerRule> LayerImpedanceDiffRules;
    };
    // Описание сигнального кластера цепей.
    struct SignalCluster {
        // Описание заданной связи.
        struct PinPair {
            // Ссылка на контакт источника сигнала.
            Xml::Array<PinRef> PinRefs;
        };
        // Описание сигнала.
        struct Signal {
            // Имя объекта или ссылка на именованный объект.
            Xml::Attr<QString> name;
            // Ссылка на контакт источника сигнала.
            ReceiverPinRef receiverPinRef;
            // Пассивные компоненты на пути следования сигнала.
            Xml::ArrayElem<CompInstanceRef> Components;
        };
        // Ссылка на волновое сопротивление.
        // [Xml::Element("ImpedanceRef")] public ImpedanceRef impedanceRef;
        ImpedanceRef impedanceRef;
        // Ссылка на контакт источника сигнала.
        SourcePinRef sourcePinRef;
        // Цепи сигнального кластера.
        Xml::ArrayElem<NetRef> Nets;
        // Описание заданных связей сигнального кластера.
        Xml::ArrayElem<PinPair> PinPairs;
        // Ссылки на сигналы.
        Xml::Array<Signal> Signals;
    };
    // Описание дифференциального сигнала (дифференциальной пары).
    struct DiffSignal {
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Параметр дифференциальной пары: допустимый разброс длины между проводниками пары.
        Xml::Attr<double> mismatch;
        // Ссылка на волновое сопротивление.
        ImpedanceRef impedanceRef;
        // Ссылки на сигналы.
        Xml::Array<SignalRef> SignalRefs;
    };
    // Описание группы сигналов.
    struct SignalGroup {
        // Имя объекта или ссылка на именованный объект.
        Xml::Attr<QString> name;
        // Ссылки на сигнал, диф.сигнал, или группу сигналов
        Xml::Array<Xml::Variant<SignalRef, DiffSignalRef, SignalGroupRef>> References;
    };
    // Описание правил выравнивания задержек.
    struct RulesDelay {
        // Описание правила выравнивания задержек для группы цепей или группы дифференциальных пар.
        struct DelayEqual {
            // Флаг применения правила.
            Xml::Attr<Bool> enabled;
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            Xml::Attr<valueType> valueType_;
            // Параметр правила выравнивания задержек внутри группы цепей: допуск.
            /// \note !Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).
            Xml::Attr<double> tolerance;
            // Объекты воздействия правила.
            Xml::ArrayElem<SignalGroupRef> ObjectsAffected;
            bool isEmpty() const { return ObjectsAffected.empty(); } // FIXME  bugfix for generate empty DelayEqual // to skip serialization
        };
        // Описание правила задания абсолютного значения задержки.
        struct DelayConstant {
            // Флаг применения правила.
            Xml::Attr<Bool> enabled;
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            Xml::Attr<valueType> valueType_;
            // Значение константы в правилах выравнивания задержек.
            /// \note !Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).
            Xml::Attr<double> constant;
            // Параметр правила выравнивания задержек: нижний допуск.
            /// \note !Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).
            Xml::Attr<double> toleranceUnder;
            // Параметр правила выравнивания задержек: верхний допуск.
            /// \note !Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).
            Xml::Attr<double> toleranceOver;
            // Объекты воздействия правила.
            Xml::ArrayElem<Xml::Variant<SignalRef, DiffSignalRef, SignalGroupRef>> ObjectsAffected;
        };
        // Описание правила взаимного выравнивания задержек.
        /// \note !Правило несимметрично относительно ObjectLeft и ObjectRight
        struct DelayRelation {
            // Флаг применения правила.
            Xml::Attr<Bool> enabled;
            // Параметр правил выравнивания задержек: тип значений констант и допусков.
            Xml::Attr<valueType> valueType_;
            // Значение константы в правилах выравнивания задержек.
            /// \note !Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).
            Xml::Attr<double> constant;
            // Параметр правила выравнивания задержек: нижний допуск.
            /// \note !Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).
            Xml::Attr<double> toleranceUnder;
            // Параметр правила выравнивания задержек: верхний допуск.
            /// \note !Единицы измерения значения зависят от параметра valueType и единиц заданных для всего файла(см.Units).
            Xml::Attr<double> toleranceOver;
            // Первый объект воздействия правила взаимного выравнивания задержек.
            Xml::NamedTag<ObjectSignal, "ObjectLeft"> objectLeft;
            Xml::NamedTag<ObjectSignal, "ObjectRight"> objectRight;
        };
        // Правила выравнивания задержек для группы цепей или группы дифференциальных пар.
        Xml::Array<DelayEqual> delayEquals;
        // Правила задания абсолютного значения задержки.
        Xml::Array<DelayConstant> delayConstants;
        // Правила взаимного выравнивания задержек.
        /// \note !Правила несимметричны относительно ObjectLeft и ObjectRight
        Xml::Array<DelayRelation> delayRelations;

        bool isEmpty() const { return delayEquals.empty() && delayConstants.empty() && delayRelations.empty(); } // to skip serialization
    };
    // Настройки поиска сигналов.
    struct SignalSearchSettings {
        // Правило именования цепей дифференциальных сигналов.
        struct RuleDiffSignalNetsNames {
            // Флаг применения правила.
            Xml::Attr<Bool> enabled;
            // Параметр правила именования цепей дифференциальных сигналов: подстрока, определяющая цепь позитивного сигнала.
            Xml::Attr<QString> posStr;
            // Параметр правила именования цепей дифференциальных сигналов: подстрока, определяющая цепь негативного сигнала.
            Xml::Attr<QString> negStr;
            operator bool() const { return +enabled; }
        };
        // Список цепей, исключённых из поиска сигналов.
        struct ExcludedNets {
            // Минимальное количество контактов в силовой цепи. Параметр используется для автоматического определения силовых цепей.
            Xml::Attr<int> minPinsNumber;
            // Cсылки на цепи.
            Xml::Array<NetRef> NetRefs;
        };
        // Максимальное число цепей в сигнальном кластере. Параметр используется при автоматическом определении цепей сигнального кластера.
        Xml::Attr<int> maxNetsInCluster;
        // Автоматически задавать связи.
        Xml::Attr<Bool> createPinPairs;
        // Правила именования цепей дифференциальных сигналов.
        /// \note !Порядок следования правил в этой секции определяет приоритет правил. Правила следуют в порядке убывания приоритета.
        Xml::ArrayElem<RuleDiffSignalNetsNames> RulesDiffSignalNetsNames;
        // Список цепей, исключённых из поиска сигналов.
        ExcludedNets excludedNets;
    };
    // Версия раздела.
    Xml::Attr<QString> version;
    // Волновые сопротивления и правила разводки сигналов.
    Xml::ArrayElem<Xml::Variant<Impedance, ImpedanceDiff>> RulesImpedances;
    // Сигнальные кластеры цепей.
    Xml::ArrayElem<SignalCluster> SignalClusters;
    // Дифференциальные сигналы.
    Xml::ArrayElem<DiffSignal> DiffSignals;
    // Группы сигналов.
    Xml::ArrayElem<SignalGroup> SignalGroups;
    // Правила выравнивания задержек.
    RulesDelay rulesDelay;
    // Настройки автоматического поиска сигналов.
    SignalSearchSettings signalSearchSettings;
    /************************************************************************
     * Здесь находятся функции для работы с элементами класса HiSpeedRules. *
     * Они не являются частью формата TopoR PCB.                            *
     * **********************************************************************/
    // Переименование ссылок на компонент, если его имя изменилось
    /// \param '1 \brief старое имя компонента
    /// \param '1 \brief новое имя компонента
    void Rename_compName(const QString& oldname, const QString& newname);
    /***********************************************************************/
};

} // namespace TopoR

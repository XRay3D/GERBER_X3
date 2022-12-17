#include "HiSpeedRules.h"
#include <ranges>

// namespace TopoR_PCB_Classes {

bool HiSpeedRules::Impedance::ShouldSerialize_LayerImpedanceRules() {
    return _LayerImpedanceRules.empty() ? false : _LayerImpedanceRules.size() > 0;
}

bool HiSpeedRules::ImpedanceDiff::ShouldSerialize_LayerImpedanceDiffRules() {
    return _LayerImpedanceDiffRules.empty() ? false : _LayerImpedanceDiffRules.size() > 0;
}

bool HiSpeedRules::SignalCluster::PinPair::ShouldSerialize_PinRefs() {
    return _PinRefs.empty() ? false : _PinRefs.size() > 0;
}

bool HiSpeedRules::DiffSignal::ShouldSerialize_SignalRefs() {
    return _SignalRefs.empty() ? false : _SignalRefs.size() > 0;
}

bool HiSpeedRules::SignalGroup::ShouldSerialize_References() {
    return _References.empty() ? false : _References.size() > 0;
}

bool HiSpeedRules::RulesDelay::DelayEqual::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool HiSpeedRules::RulesDelay::DelayEqual::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool HiSpeedRules::RulesDelay::DelayConstant::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool HiSpeedRules::RulesDelay::DelayConstant::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool HiSpeedRules::RulesDelay::DelayRelation::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool HiSpeedRules::RulesDelay::ShouldSerialize_DelayEquals() {
    return _DelayEquals.empty() ? false : _DelayEquals.size() > 0;
}

bool HiSpeedRules::RulesDelay::ShouldSerialize_DelayConstants() {
    return _DelayConstants.empty() ? false : _DelayConstants.size() > 0;
}

bool HiSpeedRules::RulesDelay::ShouldSerialize_DelayRelations() {
    return _DelayRelations.empty() ? false : _DelayRelations.size() > 0;
}

bool HiSpeedRules::SignalSearchSettings::RuleDiffSignalNetsNames::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool HiSpeedRules::SignalSearchSettings::ExcludedNets::ShouldSerialize_NetRefs() {
    return _NetRefs.empty() ? false : _NetRefs.size() > 0;
}

bool HiSpeedRules::SignalSearchSettings::getCreatePinPairsSpecified() const {
    return _createPinPairs != Bool::off;
}

bool HiSpeedRules::SignalSearchSettings::ShouldSerialize_RulesDiffSignalNetsNames() {
    return _RulesDiffSignalNetsNames.empty() ? false : _RulesDiffSignalNetsNames.size() > 0;
}

bool HiSpeedRules::ShouldSerialize_RulesImpedances() {
    return _RulesImpedances.empty() ? false : _RulesImpedances.size() > 0;
}

bool HiSpeedRules::ShouldSerialize_SignalClusters() {
    return _SignalClusters.empty() ? false : _SignalClusters.size() > 0;
}

bool HiSpeedRules::ShouldSerialize_DiffSignals() {
    return _DiffSignals.empty() ? false : _DiffSignals.size() > 0;
}

bool HiSpeedRules::ShouldSerialize_SignalGroups() {
    return _SignalGroups.empty() ? false : _SignalGroups.size() > 0;
}

void HiSpeedRules::Rename_compName(const QString& oldname, const QString& newname) {
    for (auto a : _SignalClusters) {
        if (a->_SourcePinRef->_compName == oldname) {
            a->_SourcePinRef->_compName = newname;
        }
        for (auto b : a->_Signals) {
            if (b->_ReceiverPinRef->_compName == oldname) {
                b->_ReceiverPinRef->_compName = newname;
            }
            for (auto c : b->_Components | std::views::filter([&](CompInstanceRef* r) { return r->_ReferenceName == oldname; })) {
                c->_ReferenceName = newname;
            }
        }
        for (auto b : a->_PinPairs) {
            for (auto c : b->_PinRefs | std::views::filter([&](PinRef* r) { return r->_compName == oldname; })) {
                c->_compName = newname;
            }
        }
    }
}
// } // namespace TopoR_PCB_Classes

#include "HiSpeedRules.h"
namespace TopoR {
bool HiSpeedRules::Impedance::ShouldSerialize_LayerImpedanceRules() {
    return {}; //    return LayerImpedanceRules.size();
}
bool HiSpeedRules::ImpedanceDiff::ShouldSerialize_LayerImpedanceDiffRules() {
    return {}; //    return LayerImpedanceDiffRules.size();
}
bool HiSpeedRules::SignalCluster::PinPair::ShouldSerialize_PinRefs() {
    return {}; //    return PinRefs.size();
}
bool HiSpeedRules::DiffSignal::ShouldSerialize_SignalRefs() {
    return {}; //    return SignalRefs.size();
}
bool HiSpeedRules::SignalGroup::ShouldSerialize_References() {
    return {}; //    return References.size();
}

bool HiSpeedRules::RulesDelay::DelayEqual::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}

bool HiSpeedRules::RulesDelay::DelayConstant::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}

bool HiSpeedRules::RulesDelay::ShouldSerialize_DelayEquals() {
    return {}; //    return DelayEquals.size();
}
bool HiSpeedRules::RulesDelay::ShouldSerialize_DelayConstants() {
    return {}; //    return DelayConstants.size();
}
bool HiSpeedRules::RulesDelay::ShouldSerialize_DelayRelations() {
    return {}; //    return DelayRelations.size();
}

bool HiSpeedRules::SignalSearchSettings::ExcludedNets::ShouldSerialize_NetRefs() {
    return {}; //    return NetRefs.size();
}

bool HiSpeedRules::SignalSearchSettings::ShouldSerialize_RulesDiffSignalNetsNames() {
    return {}; //    return RulesDiffSignalNetsNames.size();
}
bool HiSpeedRules::ShouldSerialize_RulesImpedances() {
    return {}; //    return RulesImpedances.size();
}
bool HiSpeedRules::ShouldSerialize_SignalClusters() {
    return {}; //    return SignalClusters.size();
}
bool HiSpeedRules::ShouldSerialize_DiffSignals() {
    return {}; //    return DiffSignals.size();
}
bool HiSpeedRules::ShouldSerialize_SignalGroups() {
    return {}; //    return SignalGroups.size();
}
void HiSpeedRules::Rename_compName(const std::string& oldname, const std::string& newname) {
    //    for(auto a: SignalClusters) {
    //        if(a->SourcePinRef->compName == oldname)
    //            a->SourcePinRef->compName = newname;
    //        for(auto b: a->Signals) {
    //            if(b->ReceiverPinRef->compName == oldname)
    //                b->ReceiverPinRef->compName = newname;
    //            for(auto c: (b->Components == nullptr ? nullptr : b->Components.Where([&](std::variant</*XML::Null,*/ > r) {
    //                    return r->ReferenceName == oldname;
    //                })))
    //                c->ReferenceName = newname;
    //        }
    //        for(auto b: a->PinPairs)
    //            for(auto c: (b->PinRefs == nullptr ? nullptr : b->PinRefs.Where([&](std::variant</*XML::Null,*/ > r) {
    //                    return r->compName == oldname;
    //                })))
    //                c->compName = newname;
    //    }
}
} // namespace TopoR

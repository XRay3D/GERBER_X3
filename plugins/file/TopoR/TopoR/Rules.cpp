#include "Rules.h"
namespace TopoR {

bool Rules::WidthOfWires::ShouldSerialize_LayersRefs() {
    return {}; //    return LayersRefs.size();
}
bool Rules::WidthOfWires::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}

bool Rules::ClearanceNetToNet::ShouldSerialize_LayersRefs() {
    return {}; //    return LayersRefs.size();
}
bool Rules::ClearanceNetToNet::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}

bool Rules::ClearanceCompToComp::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}

bool Rules::ViastacksOfNets::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}
bool Rules::ViastacksOfNets::ShouldSerialize_Viastacks() {
    return {}; //    return Viastacks.size();
}

bool Rules::PlaneLayerNets::ShouldSerialize_LayersRefs() {
    return {}; //    return LayersRefs.size();
}
bool Rules::PlaneLayerNets::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}

bool Rules::SignalLayerNets::ShouldSerialize_LayersRefs() {
    return {}; //    return LayersRefs.size();
}
bool Rules::SignalLayerNets::ShouldSerialize_ObjectsAffected() {
    return {}; //    return ObjectsAffected.size();
}

bool Rules::NetProperty::ShouldSerialize_NetRefs() {
    return {}; //    return NetRefs.size();
}
bool Rules::PadConnectSettings::ShouldSerializePadstackRefs() {
    return {}; //    return PadstackRefs.size();
}
bool Rules::PadConnectSettings::ShouldSerialize_PinRefs() {
    return {}; //    return PinRefs.size();
}
bool Rules::PadConnectSettings::ShouldSerialize_PadRefs() {
    return {}; //    return PadRefs.size();
}
bool Rules::ShouldSerialize_RulesWidthOfWires() {
    return {}; //    return RulesWidthOfWires.size();
}
bool Rules::ShouldSerialize_RulesClearancesNetToNet() {
    return {}; //    return RulesClearancesNetToNet.size();
}
bool Rules::ShouldSerialize_RulesClearancesCompToComp() {
    return {}; //    return RulesClearancesCompToComp.size();
}
bool Rules::ShouldSerialize_RulesViastacksOfNets() {
    return {}; //    return RulesViastacksOfNets.size();
}
bool Rules::ShouldSerialize_RulesPlaneLayersNets() {
    return {}; //    return RulesPlaneLayersNets.size();
}
bool Rules::ShouldSerialize_RulesSignalLayersNets() {
    return {}; //    return RulesSignalLayersNets.size();
}
bool Rules::ShouldSerialize_NetProperties() {
    return {}; //    return NetProperties.size();
}
void Rules::Rename_compName(const std::string& oldname, const std::string& newname) {
    //    for(auto a: (PadConnectSettings == nullptr ? nullptr : ((PadConnectSettings->PadRefs.empty() ? nullptr : PadConnectSettings->PadRefs.Where([&](std::variant</*XML::Null,*/ > aa) {
    //            return aa->compName == oldname;
    //        })))))
    //        a->compName = newname;
    //    for(auto a: (PadConnectSettings == nullptr ? nullptr : ((PadConnectSettings->PinRefs.empty() ? nullptr : PadConnectSettings->PinRefs.Where([&](std::variant</*XML::Null,*/ > aa) {
    //            return aa->compName == oldname;
    //        })))))
    //        a->compName = newname;
    //    for(auto a: (RulesClearancesCompToComp.empty() ? nullptr : RulesClearancesCompToComp.Where([&](std::variant</*XML::Null,*/ > aa) {
    //            return aa::ObjectsAffected != nullptr;
    //        })))
    //        for(auto b: a::ObjectsAffected::OfType<ComponentRef>().Where([&](std::variant</*XML::Null,*/ > bb) {
    //                return bb->ReferenceName == oldname;
    //            }))
    //            b->ReferenceName = newname;
    //    /****************************************************************/
}
} // namespace TopoR

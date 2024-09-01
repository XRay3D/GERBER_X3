// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
﻿#include "Rules.h"

    // namespace TopoR_PCB_Classes {

    bool
    Rules::WidthOfWires::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool Rules::WidthOfWires::ShouldSerialize_LayersRefs() {
    return _LayersRefs.empty() ? false : _LayersRefs.size() > 0;
}

bool Rules::WidthOfWires::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool Rules::ClearanceNetToNet::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool Rules::ClearanceNetToNet::ShouldSerialize_LayersRefs() {
    return _LayersRefs.empty() ? false : _LayersRefs.size() > 0;
}

bool Rules::ClearanceNetToNet::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool Rules::ClearanceCompToComp::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool Rules::ClearanceCompToComp::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool Rules::ViastacksOfNets::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool Rules::ViastacksOfNets::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool Rules::ViastacksOfNets::ShouldSerialize_Viastacks() {
    return _Viastacks.empty() ? false : _Viastacks.size() > 0;
}

bool Rules::PlaneLayerNets::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool Rules::PlaneLayerNets::ShouldSerialize_LayersRefs() {
    return _LayersRefs.empty() ? false : _LayersRefs.size() > 0;
}

bool Rules::PlaneLayerNets::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool Rules::SignalLayerNets::getEnabledSpecified() const {
    return _enabled != Bool::off;
}

bool Rules::SignalLayerNets::ShouldSerialize_LayersRefs() {
    return _LayersRefs.empty() ? false : _LayersRefs.size() > 0;
}

bool Rules::SignalLayerNets::ShouldSerialize_ObjectsAffected() {
    return _ObjectsAffected.empty() ? false : _ObjectsAffected.size() > 0;
}

bool Rules::NetProperty::getFlexfixSpecified() const {
    return _flexfix != Bool::off;
}

bool Rules::NetProperty::getRouteSpecified() const {
    return _route != Bool::off;
}

bool Rules::NetProperty::ShouldSerialize_NetRefs() {
    return _NetRefs.empty() ? false : _NetRefs.size() > 0;
}

bool Rules::PadConnectSettings::ShouldSerialize_PadstackRefs() {
    return _PadstackRefs.empty() ? false : _PadstackRefs.size() > 0;
}

bool Rules::PadConnectSettings::ShouldSerialize_PinRefs() {
    return _PinRefs.empty() ? false : _PinRefs.size() > 0;
}

bool Rules::PadConnectSettings::ShouldSerialize_PadRefs() {
    return _PadRefs.empty() ? false : _PadRefs.size() > 0;
}

bool Rules::ShouldSerialize_RulesWidthOfWires() {
    return _RulesWidthOfWires.empty() ? false : _RulesWidthOfWires.size() > 0;
}

bool Rules::ShouldSerialize_RulesClearancesNetToNet() {
    return _RulesClearancesNetToNet.empty() ? false : _RulesClearancesNetToNet.size() > 0;
}

bool Rules::ShouldSerialize_RulesClearancesCompToComp() {
    return _RulesClearancesCompToComp.empty() ? false : _RulesClearancesCompToComp.size() > 0;
}

bool Rules::ShouldSerialize_RulesViastacksOfNets() {
    return _RulesViastacksOfNets.empty() ? false : _RulesViastacksOfNets.size() > 0;
}

bool Rules::ShouldSerialize_RulesPlaneLayersNets() {
    return _RulesPlaneLayersNets.empty() ? false : _RulesPlaneLayersNets.size() > 0;
}

bool Rules::ShouldSerialize_RulesSignalLayersNets() {
    return _RulesSignalLayersNets.empty() ? false : _RulesSignalLayersNets.size() > 0;
}

bool Rules::ShouldSerialize_NetProperties() {
    return _NetProperties.empty() ? false : _NetProperties.size() > 0;
}

void Rules::Rename_compName(const QString& oldname, const QString& newname) {
    //    for (auto a : *_PadConnectSettings == nullptr ? false : (_PadConnectSettings->_PadRefs.empty() ? false : _PadConnectSettings->_PadRefs.Where([&](std::any aa) {
    //             return aa->_compName == oldname;
    //         }))) {
    //        a->_compName = newname;
    //    }
    //    for (auto a : *_PadConnectSettings == nullptr ? false : (_PadConnectSettings->_PinRefs.empty() ? false : _PadConnectSettings->_PinRefs.Where([&](std::any aa) {
    //             return aa->_compName == oldname;
    //         }))) {
    //        a->_compName = newname;
    //    }

    //    for (auto a : _RulesClearancesCompToComp.empty() ? false : _RulesClearancesCompToComp.Where([&](std::any aa) {
    //             return aa::_ObjectsAffected != nullptr;
    //         })) {
    //        for (auto b : a::_ObjectsAffected::OfType<ComponentRef*>().Where([&](std::any bb) {
    //                 return bb->_ReferenceName == oldname;
    //             })) {
    //            b->_ReferenceName = newname;
    //        }
    //    }

    /****************************************************************/
}
// } // namespace TopoR_PCB_Classes

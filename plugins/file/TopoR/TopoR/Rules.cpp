#include "Rules.h"

#if RULES

namespace TopoR {

void Rules::Rename_compName(const QString& /*oldname*/, const QString& /*newname*/) {
    // for(auto a: (_PadConnectSettings == nullptr ? nullptr : ((_PadConnectSettings->_PadRefs.empty() ? nullptr : PadConnectSettings_->_PadRefs.Where([&](std::any aa) { return aa.value()._compName == oldname; })))))
    //     a->_compName = newname;
    // for(auto a: (_PadConnectSettings == nullptr ? nullptr : ((_PadConnectSettings->_PinRefs.empty() ? nullptr : PadConnectSettings_->_PinRefs.Where([&](std::any aa) { return aa.value()._compName == oldname; })))))
    //     a->_compName = newname;
    // for(auto a: (_RulesClearancesCompToComp.empty() ? nullptr : RulesClearancesCompToComp.Where([&](std::any aa) { return aa::_ObjectsAffected != nullptr; })))
    //     for(auto b: a::_ObjectsAffected::OfType<ComponentRef>().Where([&](std::any bb) { return bb.value()._ReferenceName == oldname; }))
    //         b->_ReferenceName = newname;
    /****************************************************************/
}


} // namespace TopoR

#endif

// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Rules.h"
namespace TopoR {

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

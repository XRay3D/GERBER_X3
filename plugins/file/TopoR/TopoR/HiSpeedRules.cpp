// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "HiSpeedRules.h"
namespace TopoR {

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

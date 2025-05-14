#include "HiSpeedRules.h"

#if HISPEEDRULES

namespace TopoR {

void HiSpeedRules::Rename_compName(const QString& /*oldname*/, const QString& /*newname*/) {
    /* for(auto a: SignalClusters_) {
         if(a->_SourcePinRef.value()._compName == oldname)
             a->_SourcePinRef.value()._compName = newname;
         for(auto b: a.value()._Signals) {
             if(b->_ReceiverPinRef.value()._compName == oldname)
                 b->_ReceiverPinRef.value()._compName = newname;
             for(auto c: (b->_Components == nullptr ? nullptr : b.value()._Components.Where([&](std::any r) {
                     return r.value()._ReferenceName == oldname;
                 })))
                 c->_ReferenceName = newname;
         }
         for(auto b: a.value()._PinPairs)
             for(auto c: (b->_PinRefs == nullptr ? nullptr : b.value()._PinRefs.Where([&](std::any r) {
                     return r.value()._compName == oldname;
                 })))
                 c->_compName = newname;
     }*/
}


} // namespace TopoR

#endif

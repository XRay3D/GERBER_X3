#include "NetList.h"
#include "Commons.h"
#include <ranges>

// namespace TopoR_PCB_Classes {

bool NetList::Net::ShouldSerialize_refs() {
    return _refs.empty() ? false : _refs.size() > 0;
}

bool NetList::ShouldSerialize_Nets() {
    return _Nets.empty() ? false : _Nets.size() > 0;
}

void NetList::Rename_compName(const QString& oldname, const QString& newname) {
    for (auto&& a : _Nets | std::views::filter([&](Net* net) { return net->_refs.size(); /*!= nullptr;*/ })) //
    {
//        for (auto&& b : a->_refs | std::views::filter([&](std::variant<PinRef, PadRef>&& ref) { return ref.index() == 0; })) { //::OfType<PinRef*>().Where([&](std::any bb) { return bb._compName == oldname; })) {
//            b->_compName = newname;
//        }
//        for (auto&& b : a._refs::OfType<PadRef*>().Where([&](std::any bb) { return bb._compName == oldname; })) {
//            b->_compName = newname;
//        }
    }
}
// } // namespace TopoR_PCB_Classes

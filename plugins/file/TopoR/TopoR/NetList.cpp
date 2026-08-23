#include "NetList.h"
#include "Commons.h"
namespace TopoR {
bool NetList::Net::ShouldSerialize_refs() {
    return {}; //    return refs.size();
}
bool NetList::ShouldSerialize_Nets() {
    return {}; //    return Nets.size();
}
void NetList::Rename_compName(const std::string& oldname, const std::string& newname) {
    // for(auto a: Nets.Where([&](std::variant</*XML::Null,*/ > aa) {
    //         return aa::refs != nullptr;
    //     })) {
    //     for(auto b: a::refs::OfType<PinRef>().Where([&](std::variant</*XML::Null,*/ > bb) {
    //             return bb->compName == oldname;
    //         }))
    //         b->compName = newname;
    //     for(auto b: a::refs::OfType<PadRef>().Where([&](std::variant</*XML::Null,*/ > bb) {
    //             return bb->compName == oldname;
    //         }))
    //         b->compName = newname;
    // }
}
} // namespace TopoR

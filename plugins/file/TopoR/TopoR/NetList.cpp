// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "NetList.h"
#include "Commons.h"
namespace TopoR {

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

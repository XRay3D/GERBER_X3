// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Groups.h"
#include "Commons.h"
namespace TopoR {

std::string Groups::LayerGroup::ToString() {
    return {}; //    return name;
}

void Groups::Rename_compName(const std::string& oldname, const std::string& newname) {
    //    for(auto a: (CompGroups.empty() ? nullptr : CompGroups.Where([&](std::variant</*XML::Null,*/ > aa) {
    //            return aa::CompRefs != nullptr;
    //        })))
    //        for(auto b: a::CompRefs::OfType<CompInstanceRef>().Where([&](std::variant</*XML::Null,*/ > bb) {
    //                return bb->ReferenceName == oldname;
    //            }))
    //            b->ReferenceName = newname;
}
} // namespace TopoR

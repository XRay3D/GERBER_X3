#include "Groups.h"
#include "Commons.h"
namespace TopoR {
bool Groups::LayerGroup::ShouldSerializeLayerRefs() {
    return {}; //    return LayerRefs.size();
}
std::string Groups::LayerGroup::ToString() {
    return {}; //    return name;
}
bool Groups::NetGroup::ShouldSerialize_NetRefs() {
    return {}; //    return NetRefs.size();
}
bool Groups::CompGroup::ShouldSerialize_CompRefs() {
    return {}; //    return CompRefs.size();
}
bool Groups::ShouldSerialize_LayerGroups() {
    return {}; //    return LayerGroups.size();
}
bool Groups::ShouldSerialize_NetGroups() {
    return {}; //    return NetGroups.size();
}
bool Groups::ShouldSerialize_CompGroups() {
    return {}; //    return CompGroups.size();
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

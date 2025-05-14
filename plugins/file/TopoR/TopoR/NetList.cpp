#include "NetList.h"
#include "Commons.h"

#if NetList

namespace TopoR {

void NetList::Rename_compName(const QString& oldname, const QString& newname) {
    auto filter = [&oldname](auto&& b) {
        return std::visit([&oldname](auto& ref) { return ref.compName == oldname; }, b);
    };
    for (Net& a: Nets)
        for (auto&& b: a.refs | std::ranges::views::filter(filter))
            std::visit([&](auto& ref) { ref.compName = newname; }, b);
}


} // namespace TopoR

#endif

#include "Groups.h"
#include "Commons.h"

#if GROUPS

namespace TopoR {
QString Groups::LayerGroup::ToString() {
    return name;
}
void Groups::Rename_compName(const QString& /*oldname*/, const QString& /*newname*/) {
    /*  for(auto a: (_CompGroups.empty() ? nullptr : CompGroups.Where([&](std::any aa) {
              return aa::_CompRefs != nullptr;
          })))
          for(auto b: a::_CompRefs::OfType<CompInstanceRef>().Where([&](std::any bb) {
                  return bb.value()._ReferenceName == oldname;
              }))
              b->_ReferenceName = newname;*/
}

} // namespace TopoR

#endif

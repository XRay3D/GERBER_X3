// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "Settings.h"
namespace TopoR {

bool Settings::Placement::PlacementArea::ShouldSerialize_Dots() {
    return {}; //    return Dots.size();
}

} // namespace TopoR

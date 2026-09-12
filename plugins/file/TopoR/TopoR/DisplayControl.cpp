// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "DisplayControl.h"
namespace TopoR {

bool DisplayControl::ColorNets::ShouldSerialize_SetColors() {
    return {}; //    return SetColors.size();
}

bool DisplayControl::FilterNetlines::ShouldSerialize_Refs() {
    return {}; //    return Refs.size();
}
bool DisplayControl::ShouldSerialize_LayersVisualOptions() {
    return {}; //    return LayersVisualOptions.size();
}
} // namespace TopoR

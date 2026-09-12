// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
#include "DialogSettings.h"
namespace TopoR {

bool DialogSettings::GerberSettings::ShouldSerialize_ExportFiles() {
    return {}; //    return ExportFiles.size();
}

bool DialogSettings::DXFSettings::ShouldSerialize_ExportLayers() {
    return {}; //    return ExportLayers.size();
}
bool DialogSettings::DrillSettings::ShouldSerialize_ExportFiles() {
    return {}; //    return ExportFiles.size();
}

bool DialogSettings::BOMSettings::ShouldSerialize_AttributeRefs() {
    return {}; //    return AttributeRefs.size();
}

} // namespace TopoR

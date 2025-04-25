#include "Connectivity.h"

    // namespace TopoR_PCB_Classes {

    bool
    Connectivity::Via::getFixedSpecified() const {
    return _fixed != Bool::off;
}

bool Connectivity::ZippedWire::getFixedSpecified() const {
    return _fixed != Bool::off;
}

bool Connectivity::ZippedWire::ShouldSerialize_Tracks() {
    return _Tracks.empty() ? false : _Tracks.size() > 0;
}

bool Connectivity::Wire::Subwire::Teardrop::ShouldSerialize_Dots() {
    return _Dots.empty() ? false : _Dots.size() > 0;
}

bool Connectivity::Wire::Subwire::getFixedSpecified() const {
    return _fixed != Bool::off;
}

bool Connectivity::Wire::Subwire::ShouldSerialize_Teardrops() {
    return _Teardrops.empty() ? false : _Teardrops.size() > 0;
}

bool Connectivity::Wire::Subwire::ShouldSerialize_Tracks() {
    return _Tracks.empty() ? false : _Tracks.size() > 0;
}

bool Connectivity::Wire::ShouldSerialize_Subwires() {
    return _Subwires.empty() ? false : _Subwires.size() > 0;
}

bool Connectivity::Copper_Connectivity::Island::ThermalSpoke::ShouldSerialize_Dots() {
    return _Dots.empty() ? false : _Dots.size() > 0;
}

bool Connectivity::Copper_Connectivity::Island::ShouldSerialize_ThermalSpokes() {
    return _ThermalSpokes.empty() ? false : _ThermalSpokes.size() > 0;
}

bool Connectivity::Copper_Connectivity::getUseBackoffSpecified() const {
    return _useBackoff != Bool::off;
}

bool Connectivity::Copper_Connectivity::getDeleteUnconnectedSpecified() const {
    return _deleteUnconnected != Bool::off;
}

bool Connectivity::Copper_Connectivity::ShouldSerialize_Fill_lines() {
    return _Fill_lines.empty() ? false : _Fill_lines.size() > 0;
}

bool Connectivity::ShouldSerialize_Vias() {
    return _Vias.empty() ? false : _Vias.size() > 0;
}

bool Connectivity::ShouldSerialize_Serpents() {
    return _Serpents.empty() ? false : _Serpents.size() > 0;
}

bool Connectivity::ShouldSerialize_ZippedWires() {
    return _ZippedWires.empty() ? false : _ZippedWires.size() > 0;
}

bool Connectivity::ShouldSerialize_Wires() {
    return _Wires.empty() ? false : _Wires.size() > 0;
}

bool Connectivity::ShouldSerialize_Coppers() {
    return _Coppers.empty() ? false : _Coppers.size() > 0;
}

bool Connectivity::ShouldSerialize_NonfilledCoppers() {
    return _NonfilledCoppers.empty() ? false : _NonfilledCoppers.size() > 0;
}
// } // namespace TopoR_PCB_Classes

#include "Settings.h"

// namespace TopoR_PCB_Classes {

bool Settings::Autoroute::getTeardropsSpecified() const {
    return _teardrops != Bool::off;
}

bool Settings::Autoroute::getWeakCheckSpecified() const {
    return _weakCheck != Bool::off;
}

bool Settings::Autoroute::getTakeCurLayoutSpecified() const {
    return _takeCurLayout != Bool::off;
}

bool Settings::Autoroute::getDirectConnectSMDSpecified() const {
    return _directConnectSMD != Bool::off;
}

bool Settings::Autoroute::getDontStretchWireToPolypinSpecified() const {
    return _dontStretchWireToPolypin != Bool::off;
}

bool Settings::Placement::PlacementArea::ShouldSerialize_Dots() {
    return _Dots.empty() ? false : _Dots.size() > 0;
}

bool Settings::Labels_Settings::getRotateWithCompSpecified() const {
    return _rotateWithComp != Bool::off;
}

bool Settings::Labels_Settings::getUseOrientRulesSpecified() const {
    return _useOrientRules != Bool::off;
}

bool Settings::Labels_Settings::getTopHorzRotateSpecified() const {
    return _topHorzRotate != Bool::off;
}

bool Settings::Labels_Settings::getTopVertRotateSpecified() const {
    return _topVertRotate != Bool::off;
}

bool Settings::Labels_Settings::getBottomHorzRotateSpecified() const {
    return _bottomHorzRotate != Bool::off;
}

bool Settings::Labels_Settings::getBottomVertRotateSpecified() const {
    return _bottomVertRotate != Bool::off;
}
// } // namespace TopoR_PCB_Classes

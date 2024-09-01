// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
﻿#include "Settings.h"

    // namespace TopoR_PCB_Classes {

    bool
    App::gcSettings()
        .Autoroute::getTeardropsSpecified() const {
    return _teardrops != Bool::off;
}

bool App::gcSettings().Autoroute::getWeakCheckSpecified() const {
    return _weakCheck != Bool::off;
}

bool App::gcSettings().Autoroute::getTakeCurLayoutSpecified() const {
    return _takeCurLayout != Bool::off;
}

bool App::gcSettings().Autoroute::getDirectConnectSMDSpecified() const {
    return _directConnectSMD != Bool::off;
}

bool App::gcSettings().Autoroute::getDontStretchWireToPolypinSpecified() const {
    return _dontStretchWireToPolypin != Bool::off;
}

bool App::gcSettings().Placement::PlacementArea::ShouldSerialize_Dots() {
    return _Dots.empty() ? false : _Dots.size() > 0;
}

bool App::gcSettings().Labels_Settings::getRotateWithCompSpecified() const {
    return _rotateWithComp != Bool::off;
}

bool App::gcSettings().Labels_Settings::getUseOrientRulesSpecified() const {
    return _useOrientRules != Bool::off;
}

bool App::gcSettings().Labels_Settings::getTopHorzRotateSpecified() const {
    return _topHorzRotate != Bool::off;
}

bool App::gcSettings().Labels_Settings::getTopVertRotateSpecified() const {
    return _topVertRotate != Bool::off;
}

bool App::gcSettings().Labels_Settings::getBottomHorzRotateSpecified() const {
    return _bottomHorzRotate != Bool::off;
}

bool App::gcSettings().Labels_Settings::getBottomVertRotateSpecified() const {
    return _bottomVertRotate != Bool::off;
}
// } // namespace TopoR_PCB_Classes

// This is a personal academic project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: https://pvs-studio.com
﻿#include "LocalLibrary.h"

    // namespace TopoR_PCB_Classes {

    bool
    LocalLibrary::PadRect::getHandlingSpecified() const {
    return _handling != type_handling::None;
}

bool LocalLibrary::PadRect::getHandlingValueSpecified() const {
    return _handlingValue != 0;
}

bool LocalLibrary::PadRect::getCustomSpecified() const {
    return _custom != Bool::off;
}

bool LocalLibrary::PadRect::getCornerLBSpecified() const {
    return _cornerLB != Bool::off;
}

bool LocalLibrary::PadRect::getCornerRBSpecified() const {
    return _cornerRB != Bool::off;
}

bool LocalLibrary::PadRect::getCornerRTSpecified() const {
    return _cornerRT != Bool::off;
}

bool LocalLibrary::PadRect::getCornerLTSpecified() const {
    return _cornerLT != Bool::off;
}

bool LocalLibrary::PadPoly::ShouldSerialize_Dots() {
    return _Dots.empty() ? false : _Dots.size() > 0;
}

bool LocalLibrary::Padstack::getMetallizedSpecified() const {
    return _metallized != Bool::off;
}

bool LocalLibrary::Padstack::ShouldSerialize_Pads() {
    return _Pads.empty() ? false : _Pads.size() > 0;
}

bool LocalLibrary::Viastack::LayerRange::ShouldSerialize_LayerRefs() {
    return _LayerRefs.empty() ? false : _LayerRefs.size() > 0;
}

bool LocalLibrary::Viastack::getViaOnPinSpecified() const {
    return _viaOnPin != Bool::off;
}

bool LocalLibrary::Viastack::ShouldSerialize_ViaPads() {
    return _ViaPads.empty() ? false : _ViaPads.size() > 0;
}

bool LocalLibrary::Footprint::Label_Footprint::getMirrorSpecified() const {
    return _mirror != Bool::off;
}

bool LocalLibrary::Footprint::Pad::getFlippedSpecified() const {
    return _flipped != Bool::off;
}

bool LocalLibrary::Footprint::ShouldSerialize_Pads() {
    return _Pads.empty() ? false : _Pads.size() > 0;
}

bool LocalLibrary::Footprint::ShouldSerialize_Texts() {
    return _Texts.empty() ? false : _Texts.size() > 0;
}

bool LocalLibrary::Footprint::ShouldSerialize_Details() {
    return _Details.empty() ? false : _Details.size() > 0;
}

bool LocalLibrary::Footprint::ShouldSerialize_Coppers() {
    return _Coppers.empty() ? false : _Coppers.size() > 0;
}

bool LocalLibrary::Footprint::ShouldSerialize_KeepoutsPlace() {
    return _KeepoutsPlace.empty() ? false : _KeepoutsPlace.size() > 0;
}

bool LocalLibrary::Footprint::ShouldSerialize_KeepoutsTrace() {
    return _KeepoutsTrace.empty() ? false : _KeepoutsTrace.size() > 0;
}

bool LocalLibrary::Footprint::ShouldSerialize_Mntholes() {
    return _Mntholes.empty() ? false : _Mntholes.size() > 0;
}

bool LocalLibrary::Footprint::ShouldSerialize_Labels() {
    return _Labels.empty() ? false : _Labels.size() > 0;
}

QString LocalLibrary::Footprint::ToString() {
    return _name;
}

bool LocalLibrary::Component::ShouldSerialize_Pins() {
    return _Pins.empty() ? false : _Pins.size() > 0;
}

bool LocalLibrary::Component::ShouldSerialize_Attributes() {
    return _Attributes.empty() ? false : _Attributes.size() > 0;
}

QString LocalLibrary::Component::ToString() {
    return _name;
}

bool LocalLibrary::Package::ShouldSerialize_Pinpacks() {
    return _Pinpacks.empty() ? false : _Pinpacks.size() > 0;
}

bool LocalLibrary::ShouldSerialize_Padstacks() {
    return _Padstacks.empty() ? false : _Padstacks.size() > 0;
}

bool LocalLibrary::ShouldSerialize_Viastacks() {
    return _Viastacks.empty() ? false : _Viastacks.size() > 0;
}

bool LocalLibrary::ShouldSerialize_Footprints() {
    return _Footprints.empty() ? false : _Footprints.size() > 0;
}

bool LocalLibrary::ShouldSerialize_Components() {
    return _Components.empty() ? false : _Components.size() > 0;
}

bool LocalLibrary::ShouldSerialize_Packages() {
    return _Packages.empty() ? false : _Packages.size() > 0;
}
// } // namespace TopoR_PCB_Classes

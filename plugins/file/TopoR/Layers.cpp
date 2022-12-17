#include "Layers.h"

// namespace TopoR_PCB_Classes {

bool Layer::getCompsOutlineSpecified() const { return type == layer_type::Assy; }

bool Layer::getThicknessSpecified() const { return type != layer_type::Assy; }

Layer::Layer() { }

Layer::Layer(const QString& name, layer_type type, Bool compsOutline, float thickness)
    : name {name}
    , type {type}
    , compsOutline {compsOutline}
    , thickness {thickness} {
}

QString Layer::ToString() { return name; }

bool Layers::ShouldSerialize_StackUpLayers() {
    return StackUpLayers.empty() ? false : StackUpLayers.size() > 0;
}

bool Layers::ShouldSerialize_UnStackLayers() {
    return UnStackLayers.empty() ? false : UnStackLayers.size() > 0;
}

bool Layers::LayerStackUpContains(LayerRef* lref) {
    return {}; //_StackUpLayers.empty() ? false : std::ranges::count_if(_StackUpLayers, [&](Layer* r) { return r->_name == lref->_ReferenceName; }) > 0;
}

bool Layers::LayerUnStackContain(LayerRef* lref) {
    return {}; //_UnStackLayers.empty() ? false : std::ranges::count_if(_UnStackLayers, [&](Layer* r) { return r->_name == lref->_ReferenceName; }) > 0;
}
// } // namespace TopoR_PCB_Classes

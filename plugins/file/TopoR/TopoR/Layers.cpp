#include "Layers.h"
namespace TopoR {
bool Layers::Layer::getCompsOutlineSpecified() const {
    return {}; //    return type == layertype::Assy;
}
bool Layers::Layer::getThicknessSpecified() const {
    return {}; //    return type != layertype::Assy;
}
Layers::Layer::Layer() { }
Layers::Layer::Layer(const std::string& name, layertype type, Bool compsOutline, double thickness) {
    //    name = name;
    //    type = type;
    //    compsOutline = compsOutline;
    //    thickness = thickness;
}
std::string Layers::Layer::ToString() {
    return {}; //    return name;
}
bool Layers::ShouldSerialize_StackUpLayers() {
    return {}; //    return StackUpLayers.size();
}
bool Layers::ShouldSerialize_UnStackLayers() {
    return {}; //    return UnStackLayers.size();
}
bool Layers::LayerStackUpContains(LayerRef lref) {
    return {}; //    return (StackUpLayers.empty() ? nullptr : StackUpLayers.Where([&](std::variant</*XML::Null,*/ > r) {
    //                                                                 return r->name == lref->ReferenceName;
    //                                                             })
    //                                                   ->Count())
    //        > 0;
}
bool Layers::LayerUnStackContain(LayerRef lref) {
    return {}; //    return (UnStackLayers.empty() ? nullptr : UnStackLayers.Where([&](std::variant</*XML::Null,*/ > r) {
    //                                                                 return r->name == lref->ReferenceName;
    //                                                             })
    //                                                   ->Count())
    //        > 0;
}
} // namespace TopoR

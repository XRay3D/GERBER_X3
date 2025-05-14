#include "Layers.h"

#if LAYERS
/*****************************************************************
 * Здесь находятся функции для работы с элементами класса Layer. *
 * Они не являются частью формата TopoR PCB.                     *
 * ***************************************************************/
namespace TopoR {

bool Layers::LayerStackUpContains(LayerRef lref) const {
    return std::ranges::find(StackUpLayers, lref.name, &Layer::name) != StackUpLayers.cend();
}

bool Layers::LayerUnStackContain(LayerRef lref) const {
    return std::ranges::find(UnStackLayers, lref.name, &Layer::name) != UnStackLayers.cend();
}


} // namespace TopoR

#endif

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2026                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "topor_layer.h"
#include "gi_group.h"
#include "topor_file.h"

namespace TopoR {

Layer::Layer(File* file, LayerKind kind, QString name)
    : file_(file)
    , name_(std::move(name))
    , kind_(kind) {
}

QColor Layer::color() const {
    return itemsType_ == ItemsType::Normal ? colorNorm_ : colorPath_;
}

void Layer::setColor(const QColor& color) {
    colorNorm_ = color;
    colorNorm_.setAlpha(150);
    colorPath_ = color;
    for(Gi::Group* group: {itemGroupNorm, itemGroupPath})
        if(group)
            for(auto&& gi: *group) gi->changeColor();
}

void Layer::setVisible(bool visible) {
    visible_ = visible;
    applyItemsType();
}

void Layer::applyItemsType() {
    if(!(itemGroupNorm && itemGroupPath))
        return;
    if(itemGroupNorm->empty())
        itemsType_ = ItemsType::Paths;
    else if(itemGroupPath->empty())
        itemsType_ = ItemsType::Normal;
    switch(itemsType_) {
    case ItemsType::Null:
    case ItemsType::Normal:
        itemGroupNorm->setVisible(visible_);
        itemGroupPath->setVisible(false);
        break;
    case ItemsType::Paths:
        itemGroupNorm->setVisible(false);
        itemGroupPath->setVisible(visible_);
        break;
    }
}

Gi::Group* Layer::itemGroup() const {
    return itemsType_ == ItemsType::Paths ? itemGroupPath : itemGroupNorm;
}

bool Layer::isEmpty() const { return !(itemGroupNorm && itemGroupPath); }

void Layer::setItemsType(ItemsType itemsType) {
    if(itemsType_ == itemsType)
        return;
    itemsType_ = itemsType;
    applyItemsType();
}

} // namespace TopoR

void Serial::Adapter<TopoR::Layer*>::write(Writer& sb, TopoR::Layer* const& layer) {
    sb.start_object();
    bool first = true;
    Serial::detail::writeMembers(sb, *layer, first);
    sb.end_object();
}

simdjson::error_code Serial::Adapter<TopoR::Layer*>::read(
    simdjson::ondemand::value& val, TopoR::Layer*& layer) {
    simdjson::ondemand::object obj;
    if(auto err = val.get_object().get(obj); err) return err;
    layer = new TopoR::Layer{TopoR::File::crutch};
    Serial::readFields(obj, *layer);
    return simdjson::SUCCESS;
}

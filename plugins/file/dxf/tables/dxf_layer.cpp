/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  XXXXX XX, 2025                                                  *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2026                                          *
 * License   :                                                                  *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_layer.h"
#include "dxf_file.h"
#include "dxf_types.h"
#include "gi_group.h"

namespace Dxf {

Layer::Layer(File* sp)
    : AbstractTable{nullptr} {
    file_ = sp;
}

Layer::Layer(SectionParser* sp)
    : AbstractTable{sp} {
}

Layer::Layer(SectionParser* sp, const QString& name)
    : AbstractTable{sp}
    , name_(name) {
}

void Layer::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch(code.code()) {
        case SubclassMarker: break;
        case LayerName:
            qDebug() << code.string();
            name_ = code.string();
            break;
        case Flags          : flags = code; break;
        case ColorNumber    : colorNumber_ = code; break;
        case LineTypeName   : lineTypeName = code.string(); break;
        case PlottingFlag   : plottingFlag = code.string().toInt(); break;
        case LineWeightEnum : lineWeightEnum = code; break;
        case PlotStyleNameID: break;
        case MaterialID     : break;
        }
        code = sp->nextCode();
    } while(code.code() != 0);
    setColor(dxfColors[colorNumber_]);
}

QString Layer::name() const { return name_; }

int Layer::colorNumber() const { return colorNumber_; }

const GraphicObjects& Layer::graphicObjects() const { return graphicObjects_; }

void Layer::addGraphicObject(DxfGo&& go) { graphicObjects_.emplace_back(go); }

QColor Layer::color() const {
    return itemsType_ == ItemsType::Normal ? colorNorm_ : colorPath_;
}

void Layer::setColor(const QColor& color) {
    colorNorm_ = color;
    colorNorm_.setAlpha(150);
    colorPath_ = color;
    // Цвет хранится здесь, а элементы смотрят на него по указателю, так что
    // разослать правку -- забота слоя, а не каждого вызывающего.
    //
    // Обе группы, а не itemGroup(): та отдаёт лишь ту, что отвечает текущему
    // ItemsType, а при ItemsType::Both видны обе -- вторая осталась бы на
    // экране со старым цветом.
    for(Gi::Group* group: {itemGroupNorm, itemGroupPath})
        if(group)
            for(auto&& gi: *group) gi->changeColor();
}

bool Layer::isVisible() const { return visible_; }

void Layer::setVisible(bool visible) {
    visible_ = visible;
    applyItemsType();
}

void Layer::applyItemsType() {
    if(!(itemGroupNorm && itemGroupPath))
        return;
    // Тип, для которого нужной группы нет, сводим к той, что есть: слой из
    // одних контуров в режиме Normal не показал бы ничего.
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
    case ItemsType::Both:
        itemGroupNorm->setVisible(visible_);
        itemGroupPath->setVisible(visible_);
        break;
    }
}

Gi::Group* Layer::itemGroup() const {
    return itemsType_ == ItemsType::Paths ? itemGroupPath : itemGroupNorm;
}

bool Layer::isEmpty() const { return !(itemGroupNorm && itemGroupPath); }

ItemsType Layer::itemsType() const { return itemsType_; }

void Layer::setItemsType(ItemsType itemsType) {
    if(itemsType_ == itemsType)
        return;
    itemsType_ = itemsType;
    applyItemsType();
}

} // namespace Dxf

void Serial::Adapter<Dxf::Layer*>::write(Writer& sb, Dxf::Layer* const& layer) {
    sb.start_object();
    bool first = true;
    Serial::detail::writeMembers(sb, *layer, first);
    sb.end_object();
}

simdjson::error_code Serial::Adapter<Dxf::Layer*>::read(
    simdjson::ondemand::value& val, Dxf::Layer*& layer) {
    simdjson::ondemand::object obj;
    if(auto err = val.get_object().get(obj); err) return err;
    layer = new Dxf::Layer{Dxf::File::crutch};
    Serial::readFields(obj, *layer);
    return simdjson::SUCCESS;
}

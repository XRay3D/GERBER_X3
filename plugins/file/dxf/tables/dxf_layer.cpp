// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com

/********************************************************************************
 * Author    :  Damir Bakiev                                                    *
 * Version   :  na                                                              *
 * Date      :  01 February 2020                                                *
 * Website   :  na                                                              *
 * Copyright :  Damir Bakiev 2016-2022                                          *
 * License:                                                                     *
 * Use, modification & distribution is subject to Boost Software License Ver 1. *
 * http://www.boost.org/LICENSE_1_0.txt                                         *
 *******************************************************************************/
#include "dxf_layer.h"
#include "dxf_file.h"
#include "dxf_types.h"

namespace Dxf {

Layer::Layer(File* sp)
    : AbstractTable(nullptr) {
    file_ = sp;
}

Layer::Layer(SectionParser* sp)
    : AbstractTable(sp) {
}

Layer::Layer(SectionParser* sp, const QString& name)
    : AbstractTable(sp)
    , name_(name) {
}

void Layer::parse(CodeData& code) {
    do {
        data.push_back(code);
        switch (code.code()) {
        case SubclassMarker:
            break;
        case LayerName:
            qDebug() << code.string();
            name_ = code.string();
            break;
        case Flags:
            flags = code;
            break;
        case ColorNumber:
            colorNumber_ = code;
            break;
        case LineTypeName:
            lineTypeName = code.string();
            break;
        case PlottingFlag:
            plottingFlag = code.string().toInt();
            break;
        case LineWeightEnum:
            lineWeightEnum = code;
            break;
        case PlotStyleNameID:
            break;
        case MaterialID:
            break;
        }
        code = sp->nextCode();
    } while (code.code() != 0);
    setColor(dxfColors[colorNumber_]);
}

QString Layer::name() const { return name_; }

int Layer::colorNumber() const { return colorNumber_; }

const GraphicObjects& Layer::graphicObjects() const { return graphicObjects_; }

void Layer::addGraphicObject(GraphicObject&& go) { graphicObjects_.emplace_back(go); }

QColor Layer::color() const {
    return itemsType_ == ItemsType::Normal ? colorNorm_ : colorPath_;
}

void Layer::setColor(const QColor& color) {
    colorNorm_ = color;
    colorNorm_.setAlpha(150);
    colorPath_ = color;
}

bool Layer::isVisible() const { return visible_; }

void Layer::setVisible(bool visible) {
    visible_ = visible;
    if (itemGroupNorm && itemGroupPath) {
        switch (itemsType_) {
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
}

GiGroup* Layer::itemGroup() const {
    return itemsType_ == ItemsType::Paths ? itemGroupPath : itemGroupNorm;
}

bool Layer::isEmpty() const { return !(itemGroupNorm && itemGroupPath); }

ItemsType Layer::itemsType() const { return itemsType_; }

void Layer::setItemsType(ItemsType itemsType) {
    if (itemsType_ == itemsType)
        return;
    itemsType_ = itemsType;
    if (itemGroupNorm && itemGroupPath) {
        if (itemGroupNorm->empty())
            itemsType_ = ItemsType::Paths;
        else if (itemGroupPath->empty())
            itemsType_ = ItemsType::Normal;
        switch (itemsType_) {
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
}

} // namespace Dxf

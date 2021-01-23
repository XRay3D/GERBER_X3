// This is an open source non-commercial project. Dear PVS-Studio, please check it.
// PVS-Studio Static Code Analyzer for C, C++, C#, and Java: http://www.viva64.com
/*******************************************************************************
*                                                                              *
* Author    :  Damir Bakiev                                                    *
* Version   :  na                                                              *
* Date      :  01 February 2020                                                *
* Website   :  na                                                              *
* Copyright :  Damir Bakiev 2016-2021                                          *
*                                                                              *
* License:                                                                     *
* Use, modification & distribution is subject to Boost Software License Ver 1. *
* http://www.boost.org/LICENSE_1_0.txt                                         *
*                                                                              *
*******************************************************************************/
#include "dxf_layer.h"
#include "dxf_file.h"
#include "dxf_types.h"

namespace Dxf {

Layer::Layer(File* sp)
    : AbstractTable(nullptr)
{
    m_file = sp;
}

Layer::Layer(SectionParser* sp)
    : AbstractTable(sp)
{
}

Layer::Layer(SectionParser* sp, const QString& name)
    : AbstractTable(sp)
    , m_name(name)
{
}

void Layer::parse(CodeData& code)
{
    do {
        data.push_back(code);
        switch (code.code()) {
        case SubclassMarker:
            break;
        case LayerName:
            m_name = code.string();
            break;
        case Flags:
            flags = code;
            break;
        case ColorNumber:
            m_colorNumber = code;
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
    setColor(dxfColors[m_colorNumber]);
}

QString Layer::name() const { return m_name; }

int Layer::colorNumber() const { return m_colorNumber; }

const GraphicObjects& Layer::graphicObjects() const { return m_graphicObjects; }

void Layer::addGraphicObject(GraphicObject&& go) { m_graphicObjects.emplace_back(go); }

QColor Layer::color() const
{
    return m_itemsType == ItemsType::Normal
        ? m_colorNorm
        : m_colorPath;
}

void Layer::setColor(const QColor& color)
{
    m_colorNorm = color;
    m_colorNorm.setAlpha(150);
    m_colorPath = color;
}

bool Layer::isVisible() const { return m_visible; }

void Layer::setVisible(bool visible)
{
    m_visible = visible;
    if (itemGroupNorm && itemGroupPath) {
        switch (m_itemsType) {
        case ItemsType::Null:
        case ItemsType::Normal:
            itemGroupNorm->setVisible(m_visible);
            itemGroupPath->setVisible(false);
            break;
        case ItemsType::Paths:
            itemGroupNorm->setVisible(false);
            itemGroupPath->setVisible(m_visible);
            break;
        case ItemsType::Both:
            itemGroupNorm->setVisible(m_visible);
            itemGroupPath->setVisible(m_visible);
            break;
        }
    }
}

ItemGroup* Layer::itemGroup() const
{
    return m_itemsType == ItemsType::Paths
        ? itemGroupPath
        : itemGroupNorm;
}

bool Layer::isEmpty() const { return !(itemGroupNorm && itemGroupPath); }

ItemsType Layer::itemsType() const { return m_itemsType; }

void Layer::setItemsType(ItemsType itemsType)
{
    qDebug() << int(itemsType);
    if (m_itemsType == itemsType)
        return;
    m_itemsType = itemsType;
    if (itemGroupNorm && itemGroupPath) {
        if (itemGroupNorm->empty())
            m_itemsType = ItemsType::Paths;
        else if (itemGroupPath->empty())
            m_itemsType = ItemsType::Normal;
        qDebug() << int(itemsType);
        switch (m_itemsType) {
        case ItemsType::Null:
        case ItemsType::Normal:
            itemGroupNorm->setVisible(m_visible);
            itemGroupPath->setVisible(false);
            break;
        case ItemsType::Paths:
            itemGroupNorm->setVisible(false);
            itemGroupPath->setVisible(m_visible);
            break;
        case ItemsType::Both:
            qDebug() << "Both";
            itemGroupNorm->setVisible(m_visible);
            itemGroupPath->setVisible(m_visible);
            break;
        }
    }
}

}

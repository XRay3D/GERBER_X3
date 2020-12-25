#include "dxf_layer.h"
#include "dxf_file.h"
#include "dxf_values.h"

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
            plottingFlag = code;
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

ItemGroup* Layer::itemGroup()
{
    return m_itemsType == ItemsType::Normal
        ? itemGroupNorm
        : itemGroupPath;
}

}
